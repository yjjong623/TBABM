#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <cmath>
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

// Algorithm S2: Create a population at simulation time 0
void TBABM::CreatePopulation(int t, long size)
{
	// printf("[%d] CreatePopulation\n", (int)t);
	int popChange = 0;

	while (popChange < size) {
		long hid = nHouseholds++;
		Pointer<Household> hh = householdGen.GetHousehold(hid);
		households[hid] = hh;

		assert(hh->head);

		// Unit: years
		double dt = constants["ageGroupWidth"] - fmod(hh->head->age<double>(t), constants["ageGroupWidth"]);

		// Insert all members of the household into the population
		population.insert(hh->head); popChange++;
		assert(hh->head->householdID == hid);
		InitialEvents(hh->head, t, dt);
		Schedule(t + 365*dt, ChangeAgeGroup(hh->head));
		if (hh->spouse) {
			double dt = constants["ageGroupWidth"] - fmod(hh->spouse->age<double>(t), constants["ageGroupWidth"]);
			popChange++;
			population.insert(hh->spouse);
			assert(hh->spouse->householdID == hid);
			InitialEvents(hh->spouse, t, dt);
			Schedule(t + dt, ChangeAgeGroup(hh->spouse));

			// Set marriage age
			double spouseAge = (t - hh->spouse->birthDate)/365.;
			double headAge = (t - hh->head->birthDate)/365.;
			hh->spouse->marriageDate = t - 365*fileData["timeInMarriage"].getValue(0,0,spouseAge,rng);
			hh->head->marriageDate = t - 365*fileData["timeInMarriage"].getValue(0,0,headAge, rng);
		}
		for (auto it = hh->offspring.begin(); it != hh->offspring.end(); it++) {
			double dt = constants["ageGroupWidth"] - fmod((*it)->age<double>(t), constants["ageGroupWidth"]);
			popChange++;
			population.insert(*it);
			assert((*it)->householdID == hid);
			Schedule(t + dt, ChangeAgeGroup(*it));
			InitialEvents(*it, t, dt);
		}
		for (auto it = hh->other.begin(); it != hh->other.end(); it++) {
			double dt = constants["ageGroupWidth"] - fmod((*it)->age<double>(t), constants["ageGroupWidth"]);
			popChange++;
			population.insert(*it);
			assert((*it)->householdID == hid);
			Schedule(t + dt, ChangeAgeGroup(*it));

			if ((*it)->marriageStatus != MarriageStatus::Married &&
				params["otherMarried"].Sample(rng) == 1) {
				auto it2 = it;
				it2++;
				for (; it2 != hh->other.end(); it2++)
					if (*it2 &&
						!(*it2)->dead &&
						(*it)->sex != (*it2)->sex &&
						(*it2)->marriageStatus != MarriageStatus::Married) {
						
						auto male = (*it)->sex == Sex::Male ? *it : *it2;
						auto female = (*it)->sex == Sex::Male ? *it2 : *it;
						
						male->spouse = female;
						male->marriageStatus = MarriageStatus::Married;
						male->marriageDate = t - 365*fileData["timeInMarriage"].getValue(0,0,male->age(t),rng);

						female->spouse = male;
						female->marriageStatus = MarriageStatus::Married;
						female->marriageDate = t - 365*fileData["timeInMarriage"].getValue(0,0,female->age(t),rng);

						printf("Successfully found a mate for an 'other'\n");
						break;
					}
			}

			InitialEvents(*it, t, dt);
		}
	}

	// Decide whether each female is pregnant
	for (auto it = population.begin(); it != population.end(); it++) {
		auto person = *it;

		if (person->sex == Sex::Male || \
			!person->spouse || \
			person->age(t) < 15)
			continue;

		bool hasKids {person->offspring.size() > 0};

		auto birthDistribution = hasKids ? fileData["timeToSubsequentBirths"] : \
										   fileData["timeToFirstBirth"];

		double yearsToBirth = birthDistribution.getValue(0, 0, person->age(t), rng);
		int daysToFirstBirth = 365 * yearsToBirth;

		Schedule(t + daysToFirstBirth - 9*30, Pregnancy(person, person->spouse));
	}

	printf("number of items in 'households': %ld\n", households.size());
	populationSize.Record(t, popChange);
	printf("Population size: %d\n", populationSize(t));
}