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

void CheckMortality(Pointer<Individual> idv, double t)
{

}

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

		double dt = constants["ageGroupWidth"] - fmod(hh->head->age<double>(t), constants["ageGroupWidth"]);

		// Insert all members of the household into the population
		population.insert(hh->head); popChange++;
		assert(hh->head->householdID == hid);
		InitialMortalityCheck(hh->head, t, dt);
		Schedule(t + dt, ChangeAgeGroup(hh->head));
		if (hh->spouse) {
			double dt = constants["ageGroupWidth"] - fmod(hh->spouse->age<double>(t), constants["ageGroupWidth"]);
			popChange++;
			population.insert(hh->spouse);
			assert(hh->spouse->householdID == hid);
			InitialMortalityCheck(hh->spouse, t, dt);
			Schedule(t + dt, ChangeAgeGroup(hh->spouse));

			// Set marriage age
			double spouseAge = (t - hh->spouse->birthDate)/365.;
			double headAge = (t - hh->head->birthDate)/365.;
			hh->spouse->marriageDate = t - fileData["timeInMarriage"].getValue(0,0,spouseAge,rng);
			hh->head->marriageDate = t - fileData["timeInMarriage"].getValue(0,0,headAge, rng);
		}
		for (auto it = hh->offspring.begin(); it != hh->offspring.end(); it++) {
			double dt = constants["ageGroupWidth"] - fmod((*it)->age<double>(t), constants["ageGroupWidth"]);
			popChange++;
			population.insert(*it);
			assert((*it)->householdID == hid);
			Schedule(t + dt, ChangeAgeGroup(*it));
			InitialMortalityCheck(*it, t, dt);
		}
		for (auto it = hh->other.begin(); it != hh->other.end(); it++) {
			double dt = constants["ageGroupWidth"] - fmod((*it)->age<double>(t), constants["ageGroupWidth"]);
			popChange++;
			population.insert(*it);
			assert((*it)->householdID == hid);
			Schedule(t + dt, ChangeAgeGroup(*it));
			InitialMortalityCheck(*it, t, dt);
		}
	}

	// Decide whether each female is pregnant
	// for (auto it = population.begin(); it != population.end(); it++) {
	// 	auto person = *it;
	// 	if (person->sex == Sex::Male)
	// 		continue;

	// 	if (person->spouse && person->age(t) >= 15) {
	// 		auto birthDistribution = (person->offspring.size() > 0) ? \
	// 				fileData["timeToSubsequentBirths"] : fileData ["timeToFirstBirth"];

	// 		double yearsToBirth = birthDistribution.getValue(0, 0, person->age(t), rng);
	// 		int daysToFirstBirth = 365 * yearsToBirth;

	// 		if (daysToFirstBirth < constants["tMax"]) {
	// 			Schedule(t + daysToFirstBirth - 9*30, Pregnancy(person));
	// 			Schedule(t + daysToFirstBirth, Birth(person, person->spouse));
	// 		}
	// 	}
	// }

	printf("number of items in 'households': %ld\n", households.size());
	populationSize.Record(t, popChange);
	printf("Population size: %d\n", populationSize(t));
}