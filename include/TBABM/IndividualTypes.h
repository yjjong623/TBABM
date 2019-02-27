#pragma once

#include <IncidenceTimeSeries.h>
#include <PrevalenceTimeSeries.h>
#include <EventQueue.h>
#include <RNG.h>
#include <Param.h>
#include <DataFrame.h>

using std::function;
using std::map;
using namespace SimulationLib;
using StatisticalDistributions::RNG;
using EQ = EventQueue<double, bool>;
using Params = map<string, Param>;

template <typename T>
using Pointer = std::shared_ptr<T>;

using Time = int;

class Individual;

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


typedef struct IndividualInitData {
	IncidenceTimeSeries<int>&  tbInfections;  // Individuals transitioning from S to L
	IncidenceTimeSeries<int>&  tbConversions; // Individuals transitioning from L to I
	IncidenceTimeSeries<int>&  tbRecoveries;  // Individuals transitioning from I to L

	IncidenceTimeSeries<int>& tbInfectionsHousehold; // Individuals infected by household member
	IncidenceTimeSeries<int>& tbInfectionsCommunity; // Individuals infected by community

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

typedef struct IndividualHandlers {
	function<void(Pointer<Individual>, int, DeathCause)> Death;
	function<double(int)> GlobalTBPrevalence;
} IndividualHandlers;

IndividualHandlers CreateIndividualHandlers(
	function<void(Pointer<Individual>, int, DeathCause)> Death,
	function<double(int)> GlobalTBPrevalence
);

typedef struct IndividualSimContext {
	int current_time;
	EQ& event_queue;
	RNG &rng;
	map<string, DataFrameFile>& fileData;
	Params& params;
} IndividualSimContext;

IndividualSimContext CreateIndividualSimContext(
	int current_time, 
	EQ& event_queue, 
	RNG &rng,
	map<string, DataFrameFile>& fileData,
	Params& params
);