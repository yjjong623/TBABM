#include "../../include/TBABM/TB.h"

void
TB::Recovery(Time t, RecoveryType)
{
	auto lambda = [this, lifetm = GetLifetimePtr()] (auto ts, auto) -> bool {
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