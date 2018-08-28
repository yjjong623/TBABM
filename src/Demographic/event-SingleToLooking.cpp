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

// Algorithm S12: Change of marital status from single to looking
EventFunc TBABM::SingleToLooking(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			// printf("[%d] SingleToLooking: %s, %ld::%lu\n", (int)t, idv->sex == Sex::Male ? "Male" : "Female", idv->householdID, std::hash<Pointer<Individual>>()(idv));
			if (idv->dead)
				return true;

			if (idv->sex == Sex::Male)
				maleSeeking.insert(idv);
			else
				femaleSeeking.insert(idv);

			idv->marriageStatus = MarriageStatus::Looking;

			singleToLooking.Record(t, +1);
			return true;
		};

	return ef;
}