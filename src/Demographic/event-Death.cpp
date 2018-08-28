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
using MarriageStatus = Individual::MarriageStatus;
using HIVStatus = Individual::HIVStatus;

using namespace StatisticalDistributions;

// Algorithm S10: Natural death
EventFunc TBABM::Death(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			if (!idv || idv->dead)
				return true;

			auto household = households[idv->householdID];
			assert(household);

			// printf("[%d] Death: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));
			if (idv->sex == Sex::Male)
				maleSeeking.erase(idv);
			else
				femaleSeeking.erase(idv);

			if (idv->spouse)
				idv->spouse->Widowed();

			int age = idv->age(t);
			int sex = idv->sex == Sex::Male ? 0 : 1;

			idv->dead = true;

			DeleteIndividual(idv);

			household->RemoveIndividual(idv);
			if (household->size() == 0)
				household.reset();

			population.erase(idv);
			
			populationSize.Record(t, -1);
			deaths.Record(t, +1);
			deathPyramid.UpdateByAge(t, sex, age, +1);

			return true;
		};

	return ef;
}