#pragma once

#include "IndividualTypes.h"

enum class TBStatus {
	Susceptible, Latent, Infectious
};

enum class TBTreatmentStatus {
	None, Incomplete, Complete, Dropout
};

enum class StrainType {Unspecified};

enum class RecoveryType {Natural, Treatment};

typedef struct TBData {
	IncidenceTimeSeries<int>& tbInfections;  // Individuals transitioning from S to L
	IncidenceTimeSeries<int>& tbConversions; // Individuals transitioning from L to I
	IncidenceTimeSeries<int>& tbRecoveries;  // Individuals transitioning from I to L

	PrevalenceTimeSeries<int>&     tbSusceptible; // # Individuals in S
	PrevalenceTimeSeries<int>&     tbInfected;    // # Individuals in L or I
	PrevalenceTimeSeries<int>&     tbLatent;      // # Individuals in L
	PrevalenceTimeSeries<int>&     tbInfectious;  // # Individuals in I

	IncidenceTimeSeries<int>& tbTreatmentBegin;   // Individuals initiating treatment
	IncidenceTimeSeries<int>& tbTreatmentEnd;     // Individuals completing treatment
	IncidenceTimeSeries<int>& tbTreatmentDropout; // Individuals dropping out

	PrevalenceTimeSeries<int>&     tbInTreatment;        // Individuals in treatment
	PrevalenceTimeSeries<int>&     tbCompletedTreatment; // Individuals who completed
	PrevalenceTimeSeries<int>&     tbDroppedTreatment;   // Individuals who dropped
} TBData;

TBData CreateTBData(IndividualInitData data);
// MOVED to TBTypes.cpp
// 
// TBData CreateTBData(IndividualInitData data) {
// 	return {
//         data.tbInfections,
//         data.tbConversions,
//         data.tbRecoveries,
//         data.tbSusceptible,
//         data.tbInfected,
//         data.tbLatent,
//         data.tbInfectious,
//         data.tbTreatmentBegin,
//         data.tbTreatmentEnd,
//         data.tbTreatmentDropout,
//         data.tbInTreatment,
//         data.tbCompletedTreatment,
//         data.tbDroppedTreatment
// 	};
// }