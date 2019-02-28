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
	// std::cout << termcolor::on_red << "[" << (int)t << "] GetTBStatus" << termcolor::reset;

	// switch(tb_status) {
	// 	case (TBStatus::Susceptible): std::cout << " Susceptible\n"; break;
	// 	case (TBStatus::Latent):      std::cout << " Latent\n"; break;
	// 	case (TBStatus::Infectious):  std::cout << " Infectious\n"; break;
	// 	default:				      std::cout << " UNSUPPORTED TB type!\n";
	// }

	return tb_status;
}


template <typename T>
void
TB<T>::SetHouseholdCallbacks(function<void(Time)>   progression, 
							 function<void(Time)>   recovery,
							 function<double(void)> householdPrevalence)
{
	if (progression && recovery && householdPrevalence) {
		ProgressionHandler    = progression;
		RecoveryHandler       = recovery;
		HouseholdTBPrevalence = householdPrevalence;
	}
}

template <typename T>
void
TB<T>::ResetHouseholdCallbacks(void)
{
	ProgressionHandler    = nullptr;
	RecoveryHandler       = nullptr;
	HouseholdTBPrevalence = nullptr;
}

// Right now, changing the risk window is disabled. However, calling this function will
// still trigger an immediate InfectionRiskEvaluate and disable any other scheduled
// InfectionRiskEvaluates.
template <typename T>
void
TB<T>::RiskReeval(Time t)
{
	auto lambda = [this] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		// Don't do risk re-evals during the 'seeding' period
		if (ts < risk_window)
			return true;

		auto old_risk_window = risk_window;

		// Log(ts, "TB: RiskReeval triggered");

		// CONDITIONS THAT MAY CHANGE RISK WINDOW
		// if (HIVStatus() == HIVStatus::Positive)
			// risk_window = 2*365; // unit: [days]

		// /END CONDITIONS THAT MAY CHANGE RISK WINDOW

		risk_window_id += 1;

		InfectionRiskEvaluate(ts, risk_window_id);

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}

template <typename T>
void
TB<T>::Investigate(void)
{
	printf("TB<T>::Investigate\n");
	return;
}

template <typename T>
void
TB<T>::HandleDeath(Time t)
{
	switch(tb_status) {
		case (TBStatus::Susceptible):
			data.tbSusceptible.Record(t, -1); break;
		case (TBStatus::Latent):
			data.tbLatent.Record(t, -1);
			data.tbInfected.Record(t, -1); break;
		case (TBStatus::Infectious):
			data.tbInfectious.Record(t, -1); break;
			data.tbInfected.Record(t, -1); break;
		default: std::cout << "Error: UNSUPPORTED TBStatus!" << std::endl;
	}

	switch(tb_treatment_status) {
		case (TBTreatmentStatus::None): break;
		case (TBTreatmentStatus::Incomplete):
			data.tbInTreatment.Record(t, -1); break;
		case (TBTreatmentStatus::Complete):
			data.tbCompletedTreatment.Record(t, -1); break;
		case (TBTreatmentStatus::Dropout):
			data.tbDroppedTreatment.Record(t, -1); break;
		default: std::cout << "Error: UNSUPPORTED TBTreatmentStatus!" << std::endl;
	}

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
#define SMALLFLOAT 0.0000000001

template <typename T>
void
TB<T>::InfectionRiskEvaluate(Time t, int risk_window_local)
{
	auto lambda = [this, risk_window_local] (auto ts, auto) -> bool {
		if (risk_window_local != risk_window_id)
			return true;

		if (!AliveStatus())
			return true;

		// Can't get infected if you're not susceptible
		if (tb_status != TBStatus::Susceptible)
			return true;

		// std::cout << termcolor::on_blue << "[" << std::left \
		// 		  << std::setw(12) << name << std::setw(5) << std::right \
		// 		  << (int)ts << "] InfectionRiskEvaluate" << termcolor::reset << " " \
		// 		  << std::setw(2) << AgeStatus(ts) << " years old, " \
		// 		  << std::setw(4) << (int)CD4Count(ts) << " cells/ml, " \
		// 		  << (int)(HIVStatus() == HIVStatus::Positive) << " HIV status, " \
		// 		  << GlobalTBPrevalence(ts) << " g_prev, " \
		// 		  << HouseholdTBPrevalence(ts) << " h_prev" << termcolor::reset << std::endl;

		bool init_infection {false};
		double p_init_infection {0.12};

		double risk_global     {GlobalTBPrevalence(ts) * (double)params["TB_risk_global"].Sample(rng)};
		double risk_household  {HouseholdTBPrevalence() * (double)params["TB_risk_household"].Sample(rng)};

		// Time to infection for global and local. If any of these risks are zero,
		// change to a really small number since you can't sample 0-rate exponentials
		long double tti_global     {Exponential(risk_global    > 0 ? risk_global     : SMALLFLOAT)(rng.mt_)};
		long double tti_household  {Exponential(risk_household > 0 ? risk_household  : SMALLFLOAT)(rng.mt_)};

		// First risk window of the simulation, we infect a bunch of people
		if (ts < risk_window && Bernoulli(p_init_infection)(rng.mt_))
			init_infection = true;

		// If they do get infected, this is what the infection source is
		Source infection_source {tti_global < tti_household ? Source::Global : Source::Household};
		long double master_infection_time {init_infection ? 0.0 : std::min(tti_global, tti_household)};

		if (master_infection_time < risk_window/365) // Will this individual be infected now?
			if (!params["TB_rapidprog_risk"].Sample(rng)) { // Will they become latently infected, or progress rapidly?
				InfectLatent(ts + master_infection_time, infection_source, StrainType::Unspecified);
			} else {
				InfectInfectious(ts + master_infection_time, infection_source, StrainType::Unspecified);
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
TB<T>::InfectLatent(Time t, Source s, StrainType)
{
	auto lambda = [this, s] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		if (tb_status == TBStatus::Infectious)
			return true;

		// Log(ts, "TB infection: Latent");

		data.tbSusceptible.Record((int)ts, -1);
		data.tbInfected.Record((int)ts, +1);
		data.tbLatent.Record((int)ts, +1);

		data.tbInfections.Record((int)ts, +1);

		tb_history.emplace_back((int)ts, s);

		(s == Source::Household ? data.tbInfectionsHousehold : data.tbInfectionsCommunity).Record((int)ts, +1);

		// Mark as latently infected
		tb_status = TBStatus::Latent;

		// Decide whether individual will become infectious
		if (params["TB_prog_risk"].Sample(rng))
			InfectInfectious(ts + 365*params["TB_prog_time"].Sample(rng), s, StrainType::Unspecified);

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
TB<T>::InfectInfectious(Time t, Source s, StrainType)
{
	auto lambda = [this] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		if (tb_status == TBStatus::Infectious)
			return true;

		// Log(ts, "TB infection: Infectious");

		if (tb_status == TBStatus::Latent)
			data.tbLatent.Record((int)ts, -1);
		if (tb_status == TBStatus::Susceptible)
			data.tbSusceptible.Record((int)ts, -1);

		data.tbInfectious.Record((int)ts, +1);

		data.tbConversions.Record((int)ts, +1);

		// Mark as infectious
		tb_status = TBStatus::Infectious;

		if (ProgressionHandler)
			ProgressionHandler(ts);

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

		// Log(ts, "TB treatment begin");

		data.tbInTreatment.Record((int)ts, +1);
		data.tbTreatmentBegin.Record((int)ts, +1);

		tb_treatment_status = TBTreatmentStatus::Incomplete;

		Recovery(ts, RecoveryType::Treatment);

		if (params["TB_p_Tx_cmp"].Sample(rng)) {// Will they complete treatment? Assume 93% yes
			TreatmentComplete(ts + 365*params["TB_t_Tx_cmp"].Sample(rng));
		} else {
			TreatmentDropout(ts + 365*params["TB_t_Tx_drop"].Sample(rng)); // Assume 0.42 years
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

		// Log(ts, "TB treatment dropout");

		data.tbDroppedTreatment.Record((int)ts, +1); 
		data.tbTreatmentDropout.Record((int)ts, +1);

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

		// Log(ts, "TB treatment complete");

		data.tbTreatmentEnd.Record((int)ts, +1);
		data.tbCompletedTreatment.Record((int)ts, +1);

		tb_treatment_status = TBTreatmentStatus::Complete;

		// Recovery(ts, RecoveryType::Treatment);
		
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

		// Log(ts, "TB recovery");

		data.tbRecoveries.Record((int)ts, +1);
		data.tbInfectious.Record((int)ts, -1);
		data.tbLatent.Record((int)ts, +1);

		tb_status = TBStatus::Latent;

		if (RecoveryHandler)
			RecoveryHandler(ts);

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}