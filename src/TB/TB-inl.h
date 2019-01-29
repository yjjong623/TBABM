#include <iostream>

#include <Bernoulli.h>

#include "../../include/TBABM/IndividualTypes.h"
#include "../../include/TBABM/TB.h"
#include "../../include/TBABM/utils/termcolor.h"

using SchedulerT = EQ::SchedulerT;

template <typename T>
TB<T>::~TB(void)
{
	return;
}

template <typename T>
void
TB<T>::Log(Time t, string msg)
{
	std::cout << termcolor::on_green << "[" << std::left \
			  << std::setw(12) << name << std::setw(5) << std::right \
			  << (int)t << "] " \
			  << msg << termcolor::reset << std::endl;
}


template <typename T>
TBStatus
TB<T>::GetTBStatus(Time t)
{
	std::cout << termcolor::on_red << "[" << (int)t << "] GetTBStatus" << termcolor::reset;

	switch(tb_status) {
		case (TBStatus::Susceptible): std::cout << " Susceptible\n"; break;
		case (TBStatus::Latent):      std::cout << " Latent\n"; break;
		case (TBStatus::Infectious):  std::cout << " Infectious\n"; break;
		default:				      std::cout << " UNSUPPORTED TB type!\n";
	}

	return tb_status;
}

template <typename T>
void
TB<T>::RiskReeval(Time t)
{
	auto old_risk_window = risk_window;

	printf("RiskReeval\n");

	// CONDITIONS THAT MAY CHANGE RISK WINDOW
	if (HIVStatus() == HIVStatus::Positive)
		risk_window = 365; // unit: [days]

	// /END CONDITIONS THAT MAY CHANGE RISK WINDOW

	risk_window_id += 1;

	InfectionRiskEvaluate(t, risk_window_id);

	return;
}

template <typename T>
void
TB<T>::Investigate(void)
{
	printf("TB<T>::Investigate\n");
	return;
}

// Evaluates the risk of infection according to age,
// sex, year, CD4 count, and household TB presence.
// May schedule TB infection.
// 
// If infection is scheduled, 'InfectionRiskEvaluate'
// will not schedule itself again. If infection is
// not scheduled, then the next 'risk_window' is computed
// from the same properties used to model infection
// risk, and 'InfectionRiskEvaluate' is rescheduled.
// 
// When scheduling an infection, the only StrainType
// supported right now is 'Unspecified'.
template <typename T>
bool
TB<T>::InfectionRiskEvaluate(Time t, int risk_window_local)
{
	if (risk_window_local != risk_window_id) {
		std::cout << termcolor::on_grey << "[" << std::left << std::setw(12) << name << std::setw(5) << std::right << (int)t << "] InfectionRiskEvaluate aborted" << termcolor::reset << std::endl;
		return true;
	}

	if (!AliveStatus())
		return true;

	if (tb_status == TBStatus::Infectious)
		return true;

	std::cout << termcolor::on_blue << "[" << std::left \
			  << std::setw(12) << name << std::setw(5) << std::right \
			  << (int)t << "] InfectionRiskEvaluate" << termcolor::reset << " " \
			  << std::setw(2) << AgeStatus(t) << " years old, " \
			  << std::setw(4) << (int)CD4Count(t) << " cells/ml, " \
			  << (int)HouseholdStatus() << " household status, " \
			  << (int)(HIVStatus() == HIVStatus::Positive) << " HIV status" << termcolor::reset << std::endl;

	if (Bernoulli(0.1)(rng.mt_)) // Will this individual be infected now?
		if (Bernoulli(0.9)(rng.mt_)) // Will they become latently infected, or progress rapidly?
			InfectLatent(t, StrainType::Unspecified);
		else
		 	InfectInfectious(t, StrainType::Unspecified);
	else
		event_queue.QuickSchedule(t + risk_window, 
			[this, risk_window_local] 
			(auto t_new, auto sched) -> bool 
			{ return InfectionRiskEvaluate(t_new, risk_window_local); });

	return true;
}

// Marks an individual as infected and may or may not
// schedule the beginning of treatment.
// 
// If no treatment, recovery or death is scheduled.
template <typename T>
void
TB<T>::InfectLatent(Time t, StrainType)
{
	if (!AliveStatus())
		return;

	if (tb_status == TBStatus::Infectious) {
		printf("Warning: TB::Infect: Individual is already has infectous TB. Multiple infections not supported. Behavior undefined\n");
		return;
	}

	Log(t, "TB infection: Latent");

	data.tbSusceptible.Record(t, -1);
	data.tbInfected.Record(t, +1);
	data.tbLatent.Record(t, +1);

	data.tbInfections.Record(t, +1);

	// Mark as latently infected
	tb_status = TBStatus::Latent;

	// Decide whether individual will become infectious
	if (Bernoulli(0.5)(rng.mt_))
		event_queue.QuickSchedule(t + 200, 
			[this] (auto ts, auto) -> bool { return InfectInfectious(ts, StrainType::Unspecified); });
	
	return;
}

// Marks an individual as infected and may or may not
// schedule the beginning of treatment.
// 
// If no treatment, recovery or death is scheduled.
template <typename T>
bool
TB<T>::InfectInfectious(Time t, StrainType)
{
	if (!AliveStatus())
		return true;

	if (tb_status == TBStatus::Infectious) {
		printf("Warning: TB::Infect: Individual is already infected. Multiple infections not supported. Behavior undefined\n");
		return true;
	}

	Log(t, "TB infection: Infectious");

	data.tbLatent.Record(t, -1);
	data.tbInfectious.Record(t, +1);

	data.tbConversions.Record(t, +1);

	// Mark as infectious
	tb_status = TBStatus::Infectious;

	// Decide whether individual will enter treatment
	if (Bernoulli(0.5)(rng.mt_))
		TreatmentBegin(t);
	else if (Bernoulli(0.5)(rng.mt_)) // Decide whether individual recovers or dies
		Recovery(t, RecoveryType::Natural);
	else
		DeathHandler(t);
	
	return true;
}

// Marks an individual as having begun treatment.
// Decides if they will complete treatment, or drop
// out. Schedules either event.
template <typename T>
void
TB<T>::TreatmentBegin(Time t)
{
	if (!AliveStatus())
		return;

	if (tb_status != TBStatus::Infectious) {
		printf("Error: Cannot begin treatment for non-infectious TB\n");
		return;
	}

	Log(t, "TB treatment begin");

	data.tbInfectious.Record(t, -1);
	data.tbInTreatment.Record(t, +1);
	data.tbTreatmentBegin.Record(t, +1);

	tb_treatment_status = TBTreatmentStatus::Incomplete;

	if (Bernoulli(0.5)(rng.mt_)) // Will they complete treatment?
		TreatmentComplete(t);
	else
		TreatmentDropout(t);

	return;
}

template <typename T>
void
TB<T>::TreatmentDropout(Time t)
{
	if (!AliveStatus())
		return;

	Log(t, "TB treatment dropout");

	data.tbDroppedTreatment.Record(t, +1);
	data.tbTreatmentDropout.Record(t, +1);

	tb_treatment_status = TBTreatmentStatus::Incomplete;
	
	return;
}

template <typename T>
void
TB<T>::TreatmentComplete(Time t)
{
	if (!AliveStatus())
		return;

	Log(t, "TB treatment complete");

	data.tbTreatmentEnd.Record(t, +1);
	data.tbCompletedTreatment.Record(t, +1);

	tb_treatment_status = TBTreatmentStatus::Complete;

	Recovery(t, RecoveryType::Treatment);
	
	return;
}


template <typename T>
void
TB<T>::Recovery(Time t, RecoveryType)
{
	if (!AliveStatus())
		return;

	Log(t, "TB recovery");

	data.tbRecoveries.Record(t, +1);
	data.tbInfectious.Record(t, -1);
	data.tbLatent.Record(t, +1);

	tb_status = TBStatus::Latent;

	return;
}