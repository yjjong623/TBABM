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

// Algorithm S10: Natural death
EventFunc TBABM::Death(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			if (idv->dead)
				return true;

			// printf("[%d] Death: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));
			if (idv->sex == Sex::Male)
				maleSeeking.erase(idv);
			else
				femaleSeeking.erase(idv);

			auto household = households[idv->householdID];

			idv->dead = true;

			assert(household);

			DeleteIndividual(idv);

			household->RemoveIndividual(idv);
			if (household->size() == 0)
				household.reset();

			population.erase(idv);
			populationSize.Record(t, -1);
			deaths.Record(t, +1);

			return true;
		};

	return ef;
}