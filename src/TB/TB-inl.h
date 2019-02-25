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

		auto old_risk_window = risk_window;

		Log(ts, "TB: RiskReeval triggered");

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
	printf("Handling death at time %f\n", t);
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
template <typename T>
void
TB<T>::InfectionRiskEvaluate(Time t, int risk_window_local)
{
	auto lambda = [this, risk_window_local] (auto ts, auto) -> bool {
		if (risk_window_local != risk_window_id)
			return true;

		if (!AliveStatus())
			return true;

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

		bool infectAtInit {false};
		double timeToInfection {0.0};
		
		double risk_global {GlobalTBPrevalence(ts) * (double)params["TB_risk_global"].Sample(rng)};
		double risk_local  {HouseholdTBPrevalence(ts) * (double)params["TB_risk_local"].Sample(rng)};
		double risk {0.0000001 + risk_global + risk_local};

		// printf("problem?\n");

		if (ts < 365 && Bernoulli(0.12)(rng.mt_))
			infectAtInit = true;
		else
			timeToInfection = Exponential(risk)(rng.mt_);

		// printf("no problem\n");

		if (timeToInfection < risk_window/365 || infectAtInit) // Will this individual be infected now?
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

		if (tb_status == TBStatus::Infectious)
			return true;

		Log(ts, "TB infection: Latent");

		data.tbSusceptible.Record((int)ts, -1);
		data.tbInfected.Record((int)ts, +1);
		data.tbLatent.Record((int)ts, +1);

		data.tbInfections.Record((int)ts, +1);

		// Mark as latently infected
		tb_status = TBStatus::Latent;

		// Decide whether individual will become infectious
		if (params["TB_prog_risk"].Sample(rng))
			InfectInfectious(ts + 365*params["TB_prog_time"].Sample(rng), StrainType::Unspecified);

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

		if (tb_status == TBStatus::Infectious)
			return true;

		Log(ts, "TB infection: Infectious");

		if (tb_status == TBStatus::Latent)
			data.tbLatent.Record((int)ts, -1);
		if (tb_status == TBStatus::Susceptible)
			data.tbSusceptible.Record((int)ts, -1);

		data.tbInfectious.Record((int)ts, +1);

		data.tbConversions.Record((int)ts, +1);

		// Mark as infectious
		tb_status = TBStatus::Infectious;

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

		Log(ts, "TB treatment begin");

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

		Log(ts, "TB treatment dropout");

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

		Log(ts, "TB treatment complete");

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

		Log(ts, "TB recovery");

		data.tbRecoveries.Record((int)ts, +1);
		data.tbInfectious.Record((int)ts, -1);
		data.tbLatent.Record((int)ts, +1);

		tb_status = TBStatus::Latent;

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}