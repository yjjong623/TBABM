#include "../../include/TBABM/TB.h"

// Marks an individual as having begun treatment.
// Decides if they will complete treatment, or drop
// out. Schedules either event.
// For the purposes of surveillance, etc.. this is 
// also considered diagnosis right now
void
TB::TreatmentBegin(Time t)
{
	auto lambda = [this, lifetm = GetLifetimePtr()] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		if (tb_status != TBStatus::Infectious) {
			printf("Error: Cannot begin treatment for non-infectious TB\n");
			return true;
		}

		// Log(ts, "TB treatment begin");

		data.tbInTreatment.Record((int)ts, +1);
		data.tbTreatmentBegin.Record((int)ts, +1);
		data.activeHouseholdContacts.Record(ContactHouseholdTBPrevalence(tb_status));


		data.tbInfectious.Record((int)ts, -1);
		// printf("Incidence for time %d: %d\n", (int)ts, data.tbIncidence((int)ts));
		// printf("Prevalence for time %d: %d\n", (int)ts, data.tbInfectious((int)ts));
		
		// If HIV+, record
		if (GetHIVStatus() == HIVStatus::Positive)
			data.tbTreatmentBeginHIV.Record((int)ts, +1);

		tb_treatment_status = TBTreatmentStatus::Incomplete;

		if (RecoveryHandler)
			RecoveryHandler(ts);

		if (params["TB_p_Tx_cmp"].Sample(rng)) // Will they complete treatment? Assume 100% yes
			TreatmentComplete(ts + 365*params["TB_t_Tx_cmp"].Sample(rng));
		else
			TreatmentDropout(ts + 365*params["TB_t_Tx_drop"].Sample(rng)); // Assume 0.33 years

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}