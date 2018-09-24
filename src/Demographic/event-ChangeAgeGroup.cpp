#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;
using std::map;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using Sex = Individual::Sex;

using namespace StatisticalDistributions;

auto findHousehold = [] (map<long, Pointer<Household>> &households) -> long {
	for (auto household : households)
		if (household.second && 
			household.second->size() > 0 &&
			household.second->size() < 5)
			return household.first;

	return -1;
};

// Algorithm S9: Change of age groups
EventFunc TBABM::ChangeAgeGroup(Pointer<Individual> idv)
{
	int ageGroupWidth = constants["ageGroupWidth"]; // Age groups go 0-5, 5-10, etc.

	EventFunc ef = 
		[this, idv, ageGroupWidth](double t, SchedulerT scheduler) {
			if (!idv || idv->dead)
				return true;

			// printf("[%d] ChangeAgeGroup: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));
			double timeToNextEvent = ageGroupWidth*365;

			////////////////////////////////////////////////////////
			/// Natural death
			////////////////////////////////////////////////////////
			auto startYear = constants["startYear"];
			int gender = idv->sex == Sex::Male ? 0 : 1;
			double age = idv->age(t);
			double timeToDeath = 365 * fileData["naturalDeath"].getValue(startYear+(int)t/365, gender, age, rng);

			bool scheduledDeath = false;
			if (timeToDeath < timeToNextEvent) {
				scheduledDeath = true;
				Schedule(t + timeToDeath, Death(idv));
			}

			//////////////////////////////////////////////////////
			// Leaving the current household to form a new household
			//////////////////////////////////////////////////////
			bool idvIsHead = idv->householdPosition == HouseholdPosition::Head;
			double timeToLeave = 365*params["leavingHousehold"].Sample(rng);
			if (!idv->spouse && 
				age >= 18 && 
				age <= 55 && 
				!idvIsHead && 
				timeToLeave < timeToNextEvent &&
				timeToLeave < timeToDeath) {
				Schedule(t + timeToLeave, LeaveHousehold(idv));
			}

			////////////////////////////////////////////////////
			// Change of marital status from single to looking
			////////////////////////////////////////////////////
			double scale  = params["timeToLookingScale"].Sample(rng);
			double sample = fileData["timeToLooking"].getValue(0,gender,(t-idv->birthDate)/365,rng);
			double timeToLook = 365 * scale * sample;
			if ((idv->marriageStatus == MarriageStatus::Single ||
				 idv->marriageStatus == MarriageStatus::Divorced)
				&& timeToLook < timeToNextEvent
				&& timeToLook < timeToDeath) {
				Schedule(t + timeToLook, SingleToLooking(idv));
			}

			//////////////////////////////////////////////////////
			// Joining a household
			//////////////////////////////////////////////////////
			auto household = households[idv->householdID];
			bool changed {false};
			if (age >= 65 && household->size() == 1) {
				for (size_t i = 0; i < idv->livedWithBefore.size(); i++) {
					if (!idv->livedWithBefore[i] || idv->livedWithBefore[i]->dead)
						continue;
					int hid = idv->livedWithBefore[i]->householdID;
					if (!households[hid])
						continue;
					ChangeHousehold(idv, hid, HouseholdPosition::Other);
					changed = true;
					break;
				}
				if (!changed)
					ChangeHousehold(idv, findHousehold(households), HouseholdPosition::Other);
			}

			//////////////////////////////////////////////////////
			// HIV infection check
			//////////////////////////////////////////////////////
			HIVInfectionCheck(t, idv);

			if (!scheduledDeath)
				Schedule(t + timeToNextEvent, ChangeAgeGroup(idv));


			return true;
		};

	return ef;
}