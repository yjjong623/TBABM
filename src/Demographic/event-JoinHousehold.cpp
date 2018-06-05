#include "../../include/TBABM/TBABM.h"
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

// Algorithm S7: Joining a household
// DRAFT implemented
EventFunc TBABM::JoinHousehold(Pointer<Individual> idv, long hid)
{
	EventFunc ef = 
		[this, idv, hid](double t, SchedulerT scheduler) {
			printf("[%d] JoinHousehold, populationSize=%lu\n, individual: %ld::%lu, newHID: %ld\n", (int)t, population.size(), idv->householdID, std::hash<Pointer<Individual>>()(idv), hid);
			if (hid >= nHouseholds)
				return true;
			if (idv->dead)
				return true;


			auto household = households.at(hid);
			assert(household);

			household->PrintHousehold(t);

			// Update .livedWithBefore bidirectionally
			assert(household->head);
			household->head->livedWithBefore.push_back(idv);
			idv->livedWithBefore.push_back(household->head);

			if (household->spouse) {
				household->spouse->livedWithBefore.push_back(idv);
				idv->livedWithBefore.push_back(household->spouse);
			}

			for (auto it = household->offspring.begin(); it != household->offspring.end(); it++) {
				if (!*it)
					continue;
				(*it)->livedWithBefore.push_back(idv);
				idv->livedWithBefore.push_back(*it);
			}

			for (auto it = household->other.begin(); it != household->other.end(); it++) {
				if (!*it)
					continue;
				(*it)->livedWithBefore.push_back(idv);
				idv->livedWithBefore.push_back(*it);
			}


			household->other.insert(idv);
			idv->householdPosition = HouseholdPosition::Other;
			idv->householdID = hid;

			return true;
		};

	return ef;
}