#include "../../include/TBABM/TB.h"

// Marks an individual as having begun treatment.
// Decides if they will complete treatment, or drop
// out. Schedules either event.
// For the purposes of surveillance, etc.. this is 
// also considered diagnosis right now
void
TB::TreatmentBegin(Time t)
{
	auto lambda = [this, lifetm = GetLifetimePtr()] (auto ts_, auto) -> bool {
		auto ts = static_cast<int>(ts_);

		if (!AliveStatus())
			return true;

		if (tb_status != TBStatus::Infectious) {
			printf("Error: Cannot begin treatment for non-infectious TB\n");
			return true;
		}

		// Log(ts, "TB treatment begin");

		auto prev_household = ContactHouseholdTBPrevalence(tb_status);

		data.tbInTreatment.Record(ts, +1);
		data.tbTreatmentBegin.Record(ts, +1);
		if (tb_treatment_status == TBTreatmentStatus::Dropout)
			data.tbDroppedTreatment.Record(ts, -1);

		data.activeHouseholdContacts.Record(prev_household);

		data.tbInfectious.Record(ts, -1);
		
		// If HIV+, record
		if (GetHIVStatus() == HIVStatus::Positive)
			data.tbTreatmentBeginHIV.Record(ts, +1);

		tb_treatment_status = TBTreatmentStatus::Incomplete;

		if (RecoveryHandler)
			RecoveryHandler(ts);

		// Will they complete treatment? Assume 100% yes
		if (params["TB_p_Tx_cmp"].Sample(rng))
			TreatmentComplete(ts + 365*params["TB_t_Tx_cmp"].Sample(rng));
		else
			TreatmentDropout(ts + 365*params["TB_t_Tx_drop"].Sample(rng));

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}