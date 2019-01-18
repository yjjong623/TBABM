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

	std::cout << termcolor::reset;

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
		risk_window = 365;

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
		std::cout << termcolor::on_magenta << "[" << std::left << std::setw(8) << name << std::setw(5) << std::right << (int)t << "] InfectionRiskEvaluate aborted\n" << termcolor::reset;
		return true;
	}

	if (!AliveStatus())
		return true;

	// printf("[%s %d] InfectionRiskEvaluate:\n", name.c_str(), (int)t);
	// 	printf("\t%d years old\n", AgeStatus(t));
	// 	printf("\t%d CD4 count\n", (int)CD4Count(t));
		// printf("\t%d household status\n", (int)HouseholdStatus());
		// printf("\t%d HIV status\n", HIVStatus() == HIVStatus::Positive);

	std::cout << termcolor::on_blue << "[" << std::left << std::setw(8) \
			  << name << std::setw(5) << std::right \
			  << (int)t << "] InfectionRiskEvaluate" << termcolor::reset << std::endl \
			  << "\t" << AgeStatus(t) << " years old" << std::endl \
			  << "\t" << (int)CD4Count(t) << " cells/ml" << std::endl \
			  << "\t" << (int)HouseholdStatus() << " household status\n" \
			  << "\t" << (int)(HIVStatus() == HIVStatus::Positive) << " HIV status" << std::endl;

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
TB<T>::Infect(Time t, StrainType)
{
	if (tb_status == TBStatus::Infectious)
		printf("Warning: TB::Infect: Individual is already infected. Multiple infections not supported. Behavior undefined\n");

	// Mark as infectious
	tb_status = TBStatus::Infectious;

	// Decide whether individual will enter treatment
	if (Bernoulli(0.5)(rng.mt_))
		TreatmentBegin(t);
	else if (Bernoulli(0.5)(rng.mt_)) // Decide whether recovers or dies
		Recovery(t, RecoveryType::Natural);
	else
		DeathHandler(t);
	
	return;
}

// Marks an individual as having begun treatment.
// Decides if they will complete treatment, or drop
// out. Schedules either event.
template <typename T>
void
TB<T>::TreatmentBegin(Time t)
{
	if (tb_status != TBStatus::Infectious) {
		printf("Error: Cannot begin treatment for non-infectious TB\n");
		return;
	}

	tb_treatment_status = TBTreatmentStatus::Incomplete;

	if (Bernoulli(0.5)(rng.mt_)) // Will they complete treatment?
		TreatmentComplete(t);
	else
		TreatmentDropout(t);

	return;
}

template <typename T>
void
TB<T>::TreatmentDropout(Time)
{
	tb_treatment_status = TBTreatmentStatus::Incomplete;
	
	return;
}

template <typename T>
void
TB<T>::TreatmentComplete(Time t)
{
	tb_treatment_status = TBTreatmentStatus::Complete;

	Recovery(t, RecoveryType::Treatment);
	
	return;
}


template <typename T>
void
TB<T>::Recovery(Time, RecoveryType)
{
	tb_status = TBStatus::Latent;

	return;
}