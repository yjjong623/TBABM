#include "../../include/TBABM/TB.h"

// Marks an individual as latently infected. Records information
// to relevent data sources, and calls MaintainLatent(3)
void
TB::InfectLatent(Time t, Source source, StrainType strain)
{
	auto lambda = [this, 
				   source, 
				   strain, 
				   lifetm = GetLifetimePtr()] (auto ts, auto) {

		if (!AliveStatus())
			return true;

		// Log(ts, "TB infection: Latent");

		// If they have no history of latent TB infection
		// NOTE: May change if TB history items become more robust!
		data.tbLatent.Record((int)ts, +1);
		if (tb_status == TBStatus::Susceptible) {
			data.tbSusceptible.Record((int)ts, -1);
			data.tbInfections.Record((int)ts, +1);
		}

		// If this is a new infection, add it to the individual's
		// history
		if (tb_status == TBStatus::Susceptible)
			tb_history.emplace_back((int)ts, source, strain);

		// Mark as latently infected
		tb_status = TBStatus::Latent;

		auto documentInfectionSource = source == Source::Household ? \
										  		 data.tbInfectionsHousehold : \
												 data.tbInfectionsCommunity;
		documentInfectionSource.Record((int)ts, +1);

		// NOTE: right now, you always have the same source and strain
		// as your first TB infection!
		long double timeToActiveDisease {365*params["TB_prog_time"].Sample(rng)};
		InfectInfectious(ts + timeToActiveDisease, source, strain);

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}