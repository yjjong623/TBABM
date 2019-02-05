#include <iostream>

#include <Bernoulli.h>
#include <Exponential.h>

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

// Right now, changing the risk window is disabled. However, calling this function will
// still trigger an immediate InfectionRiskEvaluate and disable any other scheduled
// InfectionRiskEvaluates.
template <typename T>
void
TB<T>::RiskReeval(Time t)
{
	auto old_risk_window = risk_window;

	printf("RiskReeval\n");

	// CONDITIONS THAT MAY CHANGE RISK WINDOW
	// if (HIVStatus() == HIVStatus::Positive)
		// risk_window = 2*365; // unit: [days]

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
void
TB<T>::InfectionRiskEvaluate(Time t, int risk_window_local)
{
	auto lambda = [this, risk_window_local] (auto ts, auto) -> bool {
		if (risk_window_local != risk_window_id) {
			std::cout << termcolor::on_grey << "[" << std::left << std::setw(12) << name << std::setw(5) << std::right << (int)ts << "] InfectionRiskEvaluate aborted" << termcolor::reset << std::endl;
			return true;
		}

		if (!AliveStatus())
			return true;

		if (tb_status == TBStatus::Infectious)
			return true;

		std::cout << termcolor::on_blue << "[" << std::left \
				  << std::setw(12) << name << std::setw(5) << std::right \
				  << (int)ts << "] InfectionRiskEvaluate" << termcolor::reset << " " \
				  << std::setw(2) << AgeStatus(ts) << " years old, " \
				  << std::setw(4) << (int)CD4Count(ts) << " cells/ml, " \
				  << (int)HouseholdStatus() << " household status, " \
				  << (int)(HIVStatus() == HIVStatus::Positive) << " HIV status" << termcolor::reset << std::endl;

		double timeToInfection {0.0};

		if ((timeToInfection = params["TB_risk"].Sample(rng)) < risk_window/365) // Will this individual be infected now?
			if (!params["TB_rapidprog_risk"].Sample(rng)) { // Will they become latently infected, or progress rapidly?
				InfectLatent(ts + timeToInfection, StrainType::Unspecified);
			} else {
				InfectInfectious(ts + timeToInfection, StrainType::Unspecified);
			}
		else {
			InfectionRiskEvaluate(ts + risk_window, risk_window_local);
		}

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}

// Marks an individual as infected and may or may not
// schedule the beginning of treatment.
// 
// If no treatment, recovery or death is scheduled.
template <typename T>
void
TB<T>::InfectLatent(Time t, StrainType)
{
	auto lambda = [this] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		if (tb_status == TBStatus::Infectious) {
			printf("Warning: TB::Infect: Individual is already has infectous TB. Multiple infections not supported. Behavior undefined\n");
			return true;
		}

		Log(ts, "TB infection: Latent");

		data.tbSusceptible.Record(ts, -1);
		data.tbInfected.Record(ts, +1);
		data.tbLatent.Record(ts, +1);

		data.tbInfections.Record(ts, +1);

		// Mark as latently infected
		tb_status = TBStatus::Latent;

		// Decide whether individual will become infectious
		if (params["TB_prog_risk"].Sample(rng))
			InfectInfectious(ts + params["TB_prog_time"].Sample(rng), StrainType::Unspecified);

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}

// Marks an individual as infected and may or may not
// schedule the beginning of treatment.
// 
// If no treatment, recovery or death is scheduled.
template <typename T>
void
TB<T>::InfectInfectious(Time t, StrainType)
{
	auto lambda = [this] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		if (tb_status == TBStatus::Infectious) {
			printf("Warning: TB::Infect: Individual is already infected. Multiple infections not supported. Behavior undefined\n");
			return true;
		}

		Log(ts, "TB infection: Infectious");

		data.tbLatent.Record(ts, -1);
		data.tbInfectious.Record(ts, +1);

		data.tbConversions.Record(ts, +1);

		// Mark as infectious
		tb_status = TBStatus::Infectious;

		// Decide whether individual will enter treatment
		if (params["TB_Tx_init"].Sample(rng))
			TreatmentBegin(ts); // They begin treatment immediately
		else if (params["TB_p_recov"].Sample(rng)) // Decide whether individual recovers or dies
			Recovery(ts + 365*params["TB_t_recov"].Sample(rng), RecoveryType::Natural);
		else {
			DeathHandler(ts + 365*params["TB_t_death"].Sample(rng));
		}
		
		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}

// Marks an individual as having begun treatment.
// Decides if they will complete treatment, or drop
// out. Schedules either event.
template <typename T>
void
TB<T>::TreatmentBegin(Time t)
{
	auto lambda = [this] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		if (tb_status != TBStatus::Infectious) {
			printf("Error: Cannot begin treatment for non-infectious TB\n");
			return true;
		}

		Log(ts, "TB treatment begin");

		data.tbInfectious.Record(ts, -1);
		data.tbInTreatment.Record(ts, +1);
		data.tbTreatmentBegin.Record(ts, +1);

		tb_treatment_status = TBTreatmentStatus::Incomplete;

		if (params["TB_p_Tx_cmp"].Sample(rng)) {// Will they complete treatment? Assume 93% yes
			TreatmentComplete(ts + params["TB_t_Tx_cmp"].Sample(rng));
		} else {
			TreatmentDropout(ts + params["TB_t_Tx_drop"].Sample(rng)); // Assume 0.42 years
		}

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}

template <typename T>
void
TB<T>::TreatmentDropout(Time t)
{
	auto lambda = [this] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		Log(ts, "TB treatment dropout");

		data.tbDroppedTreatment.Record(ts, +1);
		data.tbTreatmentDropout.Record(ts, +1);

		tb_treatment_status = TBTreatmentStatus::Incomplete;
		
		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}

template <typename T>
void
TB<T>::TreatmentComplete(Time t)
{
	auto lambda = [this] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		Log(ts, "TB treatment complete");

		data.tbTreatmentEnd.Record(ts, +1);
		data.tbCompletedTreatment.Record(ts, +1);

		tb_treatment_status = TBTreatmentStatus::Complete;

		Recovery(ts, RecoveryType::Treatment);
		
		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}


template <typename T>
void
TB<T>::Recovery(Time t, RecoveryType)
{
	auto lambda = [this] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		Log(ts, "TB recovery");

		data.tbRecoveries.Record(ts, +1);
		data.tbInfectious.Record(ts, -1);
		data.tbLatent.Record(ts, +1);

		tb_status = TBStatus::Latent;

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}