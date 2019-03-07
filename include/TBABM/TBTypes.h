#pragma once

#include <IncidenceTimeSeries.h>
#include <PrevalenceTimeSeries.h>
#include <DiscreteTimeStatistic.h>
#include "IndividualTypes.h"

using Time = int;

enum class TBStatus {
	Susceptible, Latent, Infectious
};

enum class TBTreatmentStatus {
	None, Incomplete, Complete, Dropout
};

enum class Source {Global, Household};

enum class StrainType {Unspecified};

enum class RecoveryType {Natural, Treatment};

typedef struct TBHistoryItem {
	int t_infection;
	Source source;

	TBHistoryItem(int t, Source s) : t_infection(t), source(s) {};
} TBHistoryItem;

typedef struct TBData {
	IncidenceTimeSeries<int>& tbInfections;  // Individuals transitioning from S to L
	IncidenceTimeSeries<int>& tbConversions; // Individuals transitioning from L to I
	IncidenceTimeSeries<int>& tbRecoveries;  // Individuals transitioning from I to L

	IncidenceTimeSeries<int>& tbInfectionsHousehold; // Individuals infected by household member
	IncidenceTimeSeries<int>& tbInfectionsCommunity; // Individuals infected by community

	PrevalenceTimeSeries<int>&     tbSusceptible; // # Individuals in S
	PrevalenceTimeSeries<int>&     tbInfected;    // # Individuals in L or I
	PrevalenceTimeSeries<int>&     tbLatent;      // # Individuals in L
	PrevalenceTimeSeries<int>&     tbInfectious;  // # Individuals in I

	IncidenceTimeSeries<int>& tbTreatmentBegin;   // Individuals initiating treatment
	IncidenceTimeSeries<int>& tbTreatmentBeginHIV;// Initiating treatment and HIV+
	IncidenceTimeSeries<int>& tbTreatmentEnd;     // Individuals completing treatment
	IncidenceTimeSeries<int>& tbTreatmentDropout; // Individuals dropping out

	PrevalenceTimeSeries<int>&     tbInTreatment;        // Individuals in treatment
	PrevalenceTimeSeries<int>&     tbCompletedTreatment; // Individuals who completed
	PrevalenceTimeSeries<int>&     tbDroppedTreatment;   // Individuals who dropped

	DiscreteTimeStatistic& activeHouseholdContacts; // For each individual diagnosed with active TB,
													// the percentage of household contacts who have
													// active TB.
} TBData;

TBData CreateTBData(IndividualInitData data);

typedef struct TBHandlers {
	function<void(int)> death;
} TBHandlers;

TBHandlers CreateTBHandlers(function<void(int)> death);

typedef struct TBQueryHandlers {
	function<int(Time)> Age;
	function<bool(void)> Alive;
	function<double(Time)> CD4Count;
	function<HIVStatus(void)> GetHIVStatus;
	function<double(Time)> GlobalTBPrevalence;
} TBQueryHandlers ;

TBQueryHandlers CreateTBQueryHandlers(function<int(Time)> Age,
									  function<bool(void)> Alive,
									  function<double(Time)> CD4Count,
									  function<HIVStatus(void)> HIVStatus,
									  function<double(Time)> GlobalTBPrevalence);

typedef IndividualSimContext TBSimContext; // For right now these are the same
