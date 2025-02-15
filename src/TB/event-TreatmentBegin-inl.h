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
		
		assert(lifetm);
		assert(tb_treatment_status != TBTreatmentStatus::Incomplete);

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

		// Subgroups of treatmentBegin for children, treatment-naive adults,
		// treatment-experienced adults
		if (AgeStatus(ts) < 15)
			data.tbTreatmentBeginChildren.Record(ts, +1);
		else if (tb_treatment_status == TBTreatmentStatus::None)
			data.tbTreatmentBeginAdultsNaive.Record(ts, +1);
		else
			data.tbTreatmentBeginAdultsExperienced.Record(ts, +1);
		
		if (tb_treatment_status == TBTreatmentStatus::None && \
			AgeStatus(ts) >= 15) {
			data.tbTxExperiencedAdults.Record(ts, +1);
			data.tbTxExperiencedInfectiousAdults.Record(ts, +1);
			data.tbTxNaiveAdults.Record(ts, -1);
			data.tbTxNaiveInfectiousAdults.Record(ts, -1);
		}

		// Subgroup for HIV+ people
		if (GetHIVStatus() == HIVStatus::Positive)
			data.tbTreatmentBeginHIV.Record(ts, +1);

		if (tb_treatment_status == TBTreatmentStatus::Dropout)
			data.tbDroppedTreatment.Record(ts, -1);
		else if (tb_treatment_status == TBTreatmentStatus::Complete)
			data.tbCompletedTreatment.Record(ts, -1);

		data.activeHouseholdContacts.Record(prev_household);

		tb_treatment_status = TBTreatmentStatus::Incomplete;

		if (RecoveryHandler)
			RecoveryHandler(ts);

		// Will they complete treatment? Assume 100% yes
		if (params["TB_p_Tx_cmp"].Sample(rng))
			TreatmentComplete(ts + 365*params["TB_t_Tx_cmp"].Sample(rng));
		else
			TreatmentDropout(ts + 365*params["TB_t_Tx_drop"].Sample(rng));

		// Schedule the moment where they will be marked as "treatment-experienced."
		// Right now, this is 1 month after treatment start
		TreatmentMarkExperienced(ts + 1*30);

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}

void
TB::TreatmentMarkExperienced(Time t)
{
	auto lambda = [this, lifetm = GetLifetimePtr()] (auto ts_, auto) -> bool {

		assert(lifetm);

		if (!AliveStatus())
			return true;

		if (tb_treatment_status != TBTreatmentStatus::Incomplete)
			return true;

		if (!treatment_experienced && AgeStatus(ts_) >= 15)
			data.tbTxExperiencedAdults.Record((int)ts_, +1);

		treatment_experienced = true;

		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}
