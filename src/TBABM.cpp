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
        case TBABMData::Marriages:   return nullptr;
        case TBABMData::Births:      return nullptr;
        case TBABMData::Deaths:		 return nullptr;
        case TBABMData::PopulationSize: return &populationSize;
        case TBABMData::Divorces:    return nullptr;
        default:                     return nullptr;
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
        default:                     return nullptr;
    }
}

template <>
IncidencePyramidTimeSeries*
TBABM::GetData<IncidencePyramidTimeSeries>(TBABMData field)
{
    switch(field) {
        case TBABMData::Pyramid:     return &pyramid;
        default:                     return nullptr;
    }
}

bool TBABM::Run(void)
{
	Schedule(0, CreatePopulation(100000));
	Schedule(1, Matchmaking());
	Schedule(1, UpdatePyramid());
	Schedule(1, UpdateHouseholds());

	while (!eq.Empty()) {
		auto e = eq.Top();

		if (e.t > constants["tMax"])
			break;

		e.run();
		// delete e;
		eq.Pop();
	}
	// printf("Simulation finished!\n");

	births.Close();
	deaths.Close();
	marriages.Close();
	populationSize.Close();
	divorces.Close();
	pyramid.Close();
	householdsCount.Close();

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
	assert(households[idv->householdID]);
	assert(households[newHID]);

	oldHousehold->RemoveIndividual(idv);

	idv->householdPosition = newRole;
	idv->householdID = newHID;

	// Update livedWithBefore
	if (newHousehold->head) {
		newHousehold->head->livedWithBefore.push_back(idv);
		idv->livedWithBefore.push_back(newHousehold->head);
	}
	if (newHousehold->spouse) {
		newHousehold->spouse->livedWithBefore.push_back(idv);
		idv->livedWithBefore.push_back(newHousehold->spouse);
	}
	for (auto it = newHousehold->offspring.begin(); it != newHousehold->offspring.end(); it++) {
		if (!*it)
			continue;
		(*it)->livedWithBefore.push_back(idv);
		idv->livedWithBefore.push_back(*it);
	}
	for (auto it = newHousehold->other.begin(); it != newHousehold->other.end(); it++) {
		if (!*it)
			continue;
		(*it)->livedWithBefore.push_back(idv);
		idv->livedWithBefore.push_back(*it);
	}

	// Insert individual
	if (newRole == HouseholdPosition::Head) {
		if (newHousehold->head) {
			newHousehold->head->householdPosition = HouseholdPosition::Other;
			newHousehold->other.insert(newHousehold->head);
		}
		newHousehold->head = idv;
	}
	else if (newRole == HouseholdPosition::Spouse) {
		if (newHousehold->spouse) {
			newHousehold->other.insert(newHousehold->spouse);
			newHousehold->spouse->householdPosition = HouseholdPosition::Other;
		}
		newHousehold->spouse = idv;
	}
	else if (newRole == HouseholdPosition::Other) {
		newHousehold->other.insert(idv);
	}
	else if (newRole == HouseholdPosition::Offspring) {
		newHousehold->offspring.insert(idv);
	}

	// Clear household if there's nobody left in it
	if (oldHousehold->size() == 0)
		households[oldHID].reset();

	return;
}


#include "Demographic/event-Birth.cpp"
#include "Demographic/event-ChangeAgeGroup.cpp"
#include "Demographic/event-CreateHousehold.cpp"
#include "Demographic/event-CreatePopulation.cpp"
#include "Demographic/event-Death.cpp"
#include "Demographic/event-Divorce.cpp"
#include "Demographic/event-JoinHousehold.cpp"
#include "Demographic/event-LeaveHousehold.cpp"
#include "Demographic/event-Marriage.cpp"
#include "Demographic/event-Matchmaking.cpp"
#include "Demographic/event-NewHouseholds.cpp"
#include "Demographic/event-SingleToLooking.cpp"
#include "Demographic/event-UpdateHouseholds.cpp"
#include "Demographic/event-UpdatePyramid.cpp"