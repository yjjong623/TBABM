#include "../../include/TBABM/TBTypes.h"

TBData CreateTBData(IndividualInitData data) {
	return {
        data.tbInfections,
        data.tbConversions,
        data.tbRecoveries,
        data.tbSusceptible,
        data.tbInfected,
        data.tbLatent,
        data.tbInfectious,
        data.tbTreatmentBegin,
        data.tbTreatmentEnd,
        data.tbTreatmentDropout,
        data.tbInTreatment,
        data.tbCompletedTreatment,
        data.tbDroppedTreatment
	};
}