#include "../../include/TBABM/TB.h"

void
TB::Recovery(Time t, RecoveryType r)
{
	auto lambda = [this, r, lifetm = GetLifetimePtr()] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		// Log(ts, string("TB recovery: ") + (r == RecoveryType::Natural ? "natural" : "treatment"));

		data.tbRecoveries.Record((int)ts, +1);

		tb_status = TBStatus::Latent;

		// If they recovered and it's not because they achieved treatment
		// completion, call the RecoveryHandler
		if (RecoveryHandler && r == RecoveryType::Natural)
			RecoveryHandler(ts);

		// Set up periodic evaluation for reinfection
		InfectionRiskEvaluate(ts, risk_window);

		// Set up one-time sample for reactivation
		InfectLatent(ts, 
					 Source::Global, 
					 StrainType::Unspecified);

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}