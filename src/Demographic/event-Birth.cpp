#include "../../include/TBABM/TBABM.h"
#include <cassert>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

// Algorithm S6: Birth
EventFunc TBABM::Birth(Pointer<Individual> mother, Pointer<Individual> father)
{
	EventFunc ef = 
		[this, mother, father](double t, SchedulerT scheduler) {

			// If mother is dead
			if (!mother || mother->dead)
				return true;

			mother->pregnant = false;

			// Decide properties of baby
			Sex sex = params["sex"].Sample(rng) ?
					    Sex::Male : Sex::Female;

			HouseholdPosition householdPosition = HouseholdPosition::Offspring;
			MarriageStatus marriageStatus = MarriageStatus::Single;

			// Construct baby
			auto baby = std::make_shared<Individual>(
				mother->householdID, t, sex,
				Pointer<Individual>(), mother, father,
				vector<Pointer<Individual>>{}, householdPosition, marriageStatus);
				// std::make_shared<Params>(params), std::make_shared<map<string, DataFrameFile>>(fileData));

			// printf("[%d] Baby born: %ld::%lu\n", (int)t, mother->householdID, std::hash<Pointer<Individual>>()(baby));

			// Add baby to household and population
			mother->offspring.push_back(baby);
			if (father && !father->dead)
				father->offspring.push_back(baby);

			population.insert(baby);
			
			ChangeHousehold(baby, mother->householdID, householdPosition);
			
			Schedule(t, ChangeAgeGroup(baby));

			populationSize.Record(t, +1);
			births.Record(t, +1);


			// Schedule the next birth
			auto yearsToNextBirth = fileData["timeToSubsequentBirths"].getValue(0,0,(t-mother->birthDate)/365,rng);
			auto daysToNextBirth = 365*yearsToNextBirth;

			Schedule(t + daysToNextBirth - 9*30, Pregnancy(mother, mother->spouse));

			return true;
		};

	return ef;
}