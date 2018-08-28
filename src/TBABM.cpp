#include "../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using Sex = Individual::Sex;

using namespace StatisticalDistributions;

template <>
PrevalenceTimeSeries<int> *
TBABM::GetData<PrevalenceTimeSeries<int>>(TBABMData field)
{
    switch(field) {
        case TBABMData::Marriages:       return nullptr;
        case TBABMData::Births:          return nullptr;
        case TBABMData::Deaths:		     return nullptr;
        case TBABMData::PopulationSize:  return &populationSize;
        case TBABMData::Divorces:        return nullptr;
 
        case TBABMData::HIVNegative:     return &hivNegative;
        case TBABMData::HIVPositive:     return &hivPositive;
        case TBABMData::HIVPositiveART:  return &hivPositiveART;
 
        case TBABMData::HIVDiagnosed:    return &hivDiagnosed;
        case TBABMData::HIVDiagnosedVCT: return &hivDiagnosedVCT;
 
        default:                         return nullptr;
    }
}

template <>
IncidenceTimeSeries<int> *
TBABM::GetData<IncidenceTimeSeries<int>>(TBABMData field)
{
    switch(field) {
        case TBABMData::Marriages:   return &marriages;
        case TBABMData::Births:      return &births;
        case TBABMData::Deaths:		 return &deaths;
        case TBABMData::PopulationSize: return nullptr;
        case TBABMData::Divorces:    return &divorces;
        case TBABMData::Households:  return &householdsCount;
        case TBABMData::SingleToLooking: return &singleToLooking;

        case TBABMData::HIVInfections: return &hivInfections;
        case TBABMData::HIVDiagnosesVCT: return &hivDiagnosesVCT;

        default:                     return nullptr;
    }
}

template <>
IncidencePyramidTimeSeries*
TBABM::GetData<IncidencePyramidTimeSeries>(TBABMData field)
{
    switch(field) {
        case TBABMData::Pyramid:      return &pyramid;
        case TBABMData::DeathPyramid: return &deathPyramid;
        default:                      return nullptr;
    }
}

bool TBABM::Run(void)
{
	CreatePopulation(0, 10000);
	Schedule(1, Matchmaking());
	Schedule(1, UpdatePyramid());
	Schedule(1, UpdateHouseholds());
	Schedule(1, ARTGuidelineChange());
	Schedule(1, Survey());

	while (!eq.Empty()) {
		auto e = eq.Top();

		if (e->t > constants["tMax"])
			break;

		e->run();
		eq.Pop();
	}

	printf("Size of households at end was %ld\n", households.size());

	births.Close();
	deaths.Close();
	marriages.Close();
	populationSize.Close();
	divorces.Close();
	singleToLooking.Close();

	pyramid.Close();
	deathPyramid.Close();
	householdsCount.Close();

	hivNegative.Close();
	hivPositive.Close();
	hivPositiveART.Close();

	hivDiagnosed.Close();
	hivDiagnosedVCT.Close();

	hivInfections.Close();
	hivDiagnosesVCT.Close();

	meanSurvivalTime.close();

	return true;
}

void TBABM::Schedule(int t, EventQueue<>::EventFunc ef)
{
	auto f = eq.MakeScheduledEvent(t, ef);
	eq.Schedule(f);
	return;
}

void TBABM::PurgeReferencesToIndividual(Pointer<Individual> host,
									    Pointer<Individual> idv)
{
	// Return if host or individual do not exist
	if (!host || !idv)
		return;

	// Reset pointers for spouse, mother, father (rough equivalent of setting
	// pointers to NULL)
	if (host->spouse == idv)
		host->spouse.reset();
	if (host->mother == idv)
		host->mother.reset();
	if (host->father == idv)
		host->father.reset();

	// Erase 'idv' from offspring
	for (auto it = host->offspring.begin(); it != host->offspring.end(); it++)
		if ((*it) == idv) {
			host->offspring.erase(it);
			break;
		}

	// Erase 'idv' from list of people 'idv' has lived with before
	for (auto it = host->livedWithBefore.begin(); it != host->livedWithBefore.end(); it++)
		if ((*it) == idv) {
			it = host->livedWithBefore.erase(it);
			break;
		}
}

void TBABM::DeleteIndividual(Pointer<Individual> idv)
{
	for (size_t i = 0; i < idv->livedWithBefore.size(); i++)
		PurgeReferencesToIndividual(idv->livedWithBefore[i], idv);
}

void TBABM::ChangeHousehold(Pointer<Individual> idv, int newHID, HouseholdPosition newRole)
{
	// printf("\tChanging household of %ld::%lu\n", idv->householdID, std::hash<Pointer<Individual>>()(idv));
	int oldHID = idv->householdID;

	auto oldHousehold = households[oldHID];
	auto newHousehold = households[newHID];

	assert(idv);
	assert(oldHousehold);
	assert(newHousehold);

	oldHousehold->RemoveIndividual(idv);
	newHousehold->AddIndividual(idv, newRole, newHID);

	// Clear household if there's nobody left in it
	if (oldHousehold->size() == 0)
		households[oldHID].reset();

	return;
}


#include "Demographic/event-Birth.cpp"
#include "Demographic/event-ChangeAgeGroup.cpp"
#include "Demographic/event-CreateHousehold.cpp"
#include "Demographic/event-CreatePopulation.cpp"
#include "Demographic/helper-InitialMortalityCheck.cpp"
#include "Demographic/event-Death.cpp"
#include "Demographic/event-Divorce.cpp"
#include "Demographic/event-LeaveHousehold.cpp"
#include "Demographic/event-Marriage.cpp"
#include "Demographic/event-Matchmaking.cpp"
#include "Demographic/event-NewHouseholds.cpp"
#include "Demographic/event-Pregnancy.cpp"
#include "Demographic/event-SingleToLooking.cpp"
#include "Demographic/event-UpdateHouseholds.cpp"
#include "Demographic/event-UpdatePyramid.cpp"
#include "Demographic/event-Survey.cpp"
#include "Demographic/event-ExogenousBirth.cpp"

#include "HIV/event-ARTGuidelineChange.cpp"
#include "HIV/event-MortalityCheck.cpp"
#include "HIV/event-ARTInitiate.cpp"
#include "HIV/event-VCTDiagnosis.cpp"
#include "HIV/event-HIVInfection.cpp"
#include "HIV/helper-ARTEligible.cpp"
#include "HIV/helper-HIVInfectionCheck.cpp"