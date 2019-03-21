#include "../../include/TBABM/TB.h"

// Marks an individual as infected and may or may not
// schedule the beginning of treatment.
// 
// If no treatment, recovery or death is scheduled.
void
TB::InfectInfectious(Time t, Source s, StrainType)
{
	auto lambda = [this, lifetm = GetLifetimePtr()] (auto ts, auto) {
		if (!AliveStatus())
			return true;

		if (tb_status == TBStatus::Infectious)
			return true;

		// Log(ts, "TB infection: Infectious");

		// Prevent InfectionRiskEvaluate from continuing to run
		// (this could happen in the case that an individual is
		// re-activating right now - it shouldn't be possible for
		// them to develop a re-infection)
		risk_window += 1;

		if (tb_status == TBStatus::Latent)
			data.tbLatent.Record((int)ts, -1);
		if (tb_status == TBStatus::Susceptible)
			data.tbSusceptible.Record((int)ts, -1);

		data.tbInfectious.Record((int)ts, +1);
		data.tbIncidence.Record((int)ts, +1);

		// printf("Incidence for time %d: %d\n", (int)ts, data.tbIncidence((int)ts));
		// printf("Prevalence for time %d: %d\n", (int)ts, data.tbInfectious((int)ts));

		// Mark as infectious
		tb_status = TBStatus::Infectious;

		if (ProgressionHandler)
			ProgressionHandler(ts);

		auto timeToNaturalRecovery	= params["TB_t_recov"].Sample(rng);
		auto timeToDeath			= params["TB_t_death"].Sample(rng);
		auto timeToSeekingTreatment = params["TB_t_seek_tx"].Sample(rng);

		auto winner =
			std::min({timeToNaturalRecovery,
					  timeToDeath,
					  timeToSeekingTreatment});

		// Individual recovers naturally
		if (winner == timeToNaturalRecovery)
			Recovery(ts + 365*timeToNaturalRecovery, RecoveryType::Natural);

		// Individual dies from infection
		else if (winner == timeToDeath)
			DeathHandler(ts + 365*timeToDeath);

		// Individual seeks out treatment
		else if (winner == timeToSeekingTreatment)
			TreatmentBegin(ts + 365*timeToSeekingTreatment);

		// Something bad happened
		else {
			printf("Unsupported progression from Infectious state!\n");
			exit(1);
		}
		
		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}