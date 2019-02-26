#include "../../include/TBABM/TBTypes.h"

TBData CreateTBData(IndividualInitData data) {
	return {
        data.tbInfections,
        data.tbConversions,
        data.tbRecoveries,
        data.tbInfectionsHousehold,
        data.tbInfectionsCommunity,
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

TBHandlers CreateTBHandlers(function<void(int)> death,
                            function<void(int)> TBProgression) {
    return {
        death,
        TBProgression
    };
}


TBQueryHandlers CreateTBQueryHandlers(function<int(Time)> Age,
                                      function<bool(void)> Alive,
                                      function<double(Time)> CD4Count,
                                      function<HIVStatus(void)> HIVStatus,
                                      function<double(Time)> GlobalTBPrevalence,
                                      function<double(Time)> HouseholdTBPrevalence)
{
    return {
        Age,
        Alive,
        CD4Count,
        HIVStatus,
        GlobalTBPrevalence,
        HouseholdTBPrevalence
    };
}