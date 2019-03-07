#include "../../include/TBABM/TB.h"

// Marks an individual as having begun treatment.
// Decides if they will complete treatment, or drop
// out. Schedules either event.
// For the purposes of surveillance, etc.. this is 
// also considered diagnosis right now
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
		data.activeHouseholdContacts.Record(HouseholdTBPrevalence());

		// If HIV+, record
		if (GetHIVStatus() == HIVStatus::Positive)
			data.tbTreatmentBeginHIV.Record((int)ts, +1);

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