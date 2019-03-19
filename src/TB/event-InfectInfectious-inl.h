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
		else
			DeathHandler(ts + 365*params["TB_t_death"].Sample(rng));
		
		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}