#include "../../include/TBABM/TB.h"

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