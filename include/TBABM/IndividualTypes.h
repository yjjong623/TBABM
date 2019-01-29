#pragma once

#include <IncidenceTimeSeries.h>
#include <PrevalenceTimeSeries.h>

using namespace SimulationLib;

typedef struct IndividualInitData {
	IncidenceTimeSeries<int>&  tbInfections;  // Individuals transitioning from S to L
	IncidenceTimeSeries<int>&  tbConversions; // Individuals transitioning from L to I
	IncidenceTimeSeries<int>&  tbRecoveries;  // Individuals transitioning from I to L

	PrevalenceTimeSeries<int>& tbSusceptible; // # Individuals in S
	PrevalenceTimeSeries<int>& tbInfected;    // # Individuals in L or I
	PrevalenceTimeSeries<int>& tbLatent;      // # Individuals in L
	PrevalenceTimeSeries<int>& tbInfectious;  // # Individuals in I

	IncidenceTimeSeries<int>&  tbTreatmentBegin;   // Individuals initiating treatment
	IncidenceTimeSeries<int>&  tbTreatmentEnd;     // Individuals completing treatment
	IncidenceTimeSeries<int>&  tbTreatmentDropout; // Individuals dropping out

	PrevalenceTimeSeries<int>& tbInTreatment;        // Individuals in treatment
	PrevalenceTimeSeries<int>& tbCompletedTreatment; // Individuals who completed
	PrevalenceTimeSeries<int>& tbDroppedTreatment;   // Individuals who dropped
} IndividualInitData;

enum class Sex {
	Male, Female
};

enum class HouseholdPosition {
	Head, Spouse, Offspring, Other
};

enum class MarriageStatus {
	Single, Married, Divorced, Looking
};

enum class HIVStatus {
	Negative, Positive
};

enum class DeathCause {
	Natural, HIV, TB
};