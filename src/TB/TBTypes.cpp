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
        data.tbTreatmentBeginHIV,
        data.tbTreatmentEnd,
        data.tbTreatmentDropout,
        data.tbInTreatment,
        data.tbCompletedTreatment,
        data.tbDroppedTreatment
    };
}

TBHandlers CreateTBHandlers(function<void(int)> death) {
    return {
        death
    };
}


TBQueryHandlers CreateTBQueryHandlers(function<int(Time)> Age,
                                      function<bool(void)> Alive,
                                      function<double(Time)> CD4Count,
                                      function<HIVStatus(void)> HIVStatus,
                                      function<double(Time)> GlobalTBPrevalence)
{
    if (!Age || !Alive || !CD4Count || !HIVStatus || !GlobalTBPrevalence) {
        printf("Error: >= 1 argument to CreateTBHandlers contained empty std::function\n");
        exit(1);
    }

    return {
        Age,
        Alive,
        CD4Count,
        HIVStatus,
        GlobalTBPrevalence
    };
}