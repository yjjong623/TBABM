#include "../../include/TBABM/TB.h"

template <typename T>
void
TB<T>::TreatmentDropout(Time t)
{
	auto lambda = [this] (auto ts, auto) -> bool {
		if (!AliveStatus())
			return true;

		// Log(ts, "TB treatment dropout");

		data.tbDroppedTreatment.Record((int)ts, +1); 
		data.tbTreatmentDropout.Record((int)ts, +1);

		tb_treatment_status = TBTreatmentStatus::Dropout;
		
		return true;
	};

	eq.QuickSchedule(t, lambda);

	return;
}