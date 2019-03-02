#include "../../include/TBABM/TB.h"

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