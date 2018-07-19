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

// Algorithm S2: Create a population at simulation time 0
EventFunc TBABM::CreatePopulation(long size)
{
	EventFunc ef = 
		[this, size](double t, SchedulerT scheduler) {
			// printf("[%d] CreatePopulation\n", (int)t);
			int popChange = 0;

			while (popChange < size) {
				long hid = nHouseholds++;
				Pointer<Household> hh = householdGen.GetHousehold(hid);
				households[hid] = hh;

				assert(hh->head);

				// Insert all members of the household into the population
				population.insert(hh->head); popChange++;
				assert(hh->head->householdID == hid);
				Schedule(t, ChangeAgeGroup(hh->head));
				if (hh->spouse) {
					popChange++;
					population.insert(hh->spouse);
					assert(hh->spouse->householdID == hid);
					Schedule(t, ChangeAgeGroup(hh->spouse));

					// Set marriage age
					double spouseAge = (t - hh->spouse->birthDate)/365.;
					double headAge = (t - hh->head->birthDate)/365.;
					hh->spouse->marriageDate = t - fileData["timeInMarriage"].getValue(0,0,spouseAge,rng);
					hh->head->marriageDate = t - fileData["timeInMarriage"].getValue(0,0,headAge, rng);
				}
				for (auto it = hh->offspring.begin(); it != hh->offspring.end(); it++) {
					popChange++;
					population.insert(*it);
					assert((*it)->householdID == hid);
					Schedule(t, ChangeAgeGroup(*it));
				}
				for (auto it = hh->other.begin(); it != hh->other.end(); it++) {
					popChange++;
					population.insert(*it);
					assert((*it)->householdID == hid);
					Schedule(t, ChangeAgeGroup(*it));
				}
			}

			// Decide whether each female is pregnant
			for (auto it = population.begin(); it != population.end(); it++) {
				auto person = *it;
				if (person->sex == Sex::Male)
					continue;

				// bool isPregnant = fileData["probabilityOfPregnant"].getValue(0,0,person->age(t), rng);
				// if (isPregnant) {
				// 	auto timeToBirth = Uniform(0, .75*365)(rng.mt_);
				// 	Schedule(t + timeToBirth, Birth(person, person->spouse));
				// }
				if (person->spouse && person->age(t) >= 15) {
					auto birthDistribution = person->offspring.size() > 0 ? \
							fileData["timeToSubsequentBirths"] : fileData ["timeToFirstBirth"];

					double yearsToBirth = birthDistribution.getValue(0,0,person->age(t),rng);
					int daysToFirstBirth = 365 * yearsToBirth;
					Schedule(t + daysToFirstBirth - 9*30, Pregnancy(person));
					Schedule(t + daysToFirstBirth, Birth(person, person->spouse));
				}
			}

			populationSize.Record(t, popChange);
			printf("Population size: %d\n", populationSize(t));

			return true;
		};

	return ef;
}