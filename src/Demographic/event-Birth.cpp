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


// Algorithm S6: Birth
EventFunc TBABM::Birth(Pointer<Individual> mother, Pointer<Individual> father)
{
	EventFunc ef = 
		[this, mother, father](double t, SchedulerT scheduler) {

			// If mother is dead
			if (mother->dead)
				return true;

			// Decide properties of baby
			Sex sex = params["sex"].Sample(rng) ?
					    Sex::Male : Sex::Female;

			HouseholdPosition householdPosition = HouseholdPosition::Offspring;
			MarriageStatus marriageStatus = MarriageStatus::Single;

			// Construct baby
			auto baby = std::make_shared<Individual>(
				mother->householdID, t, sex,
				Pointer<Individual>(), mother, father,
				std::vector<Pointer<Individual>>{}, householdPosition, marriageStatus);

			// printf("[%d] Baby born: %ld::%lu\n", (int)t, mother->householdID, std::hash<Pointer<Individual>>()(baby));

			auto household = households[mother->householdID];

			assert(household);

			// Add baby to household and population
			household->offspring.insert(baby);
			population.insert(baby);

			// Update .livedWithBefore
			baby->livedWithBefore.push_back(mother);
			if (father && !father->dead)
				baby->livedWithBefore.push_back(father);
			mother->livedWithBefore.push_back(baby); // bidirectional
			if (father && !father->dead)
				father->livedWithBefore.push_back(baby);

			// Update .livedWithBefore w.r.t. 'offspring' and 'other'
			for (auto it = household->offspring.begin(); it != household->offspring.end(); it++)
				if (*it != baby && *it) {
					baby->livedWithBefore.push_back(*it);
					(*it)->livedWithBefore.push_back(baby);
				}

			for (auto it = household->other.begin(); it != household->other.end(); it++) {
				if (!*it) // if dead 'other's
					continue;
				baby->livedWithBefore.push_back(*it);
				(*it)->livedWithBefore.push_back(baby);
			}
			
			Schedule(t + 365*5, ChangeAgeGroup(baby));

			populationSize.Record(t, +1);
			births.Record(t, +1);

			// Schedule the next birth
			auto timeToNextBirth = fileData["timeToSubsequentBirths"].getValue(0,0,(t-mother->birthDate)/365,rng);
			Schedule(t + 365*timeToNextBirth, Birth(mother, mother->spouse));

			return true;
		};

	return ef;
}