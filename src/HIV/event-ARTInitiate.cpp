#include "../../include/TBABM/TBABM.h"
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc  = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using namespace StatisticalDistributions;
using HIVStatus = Individual::HIVStatus;

EventFunc TBABM::ARTInitiate(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			printf("[%d] ARTInitiate: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));

			if (!idv || idv->dead || idv->hivStatus != HIVStatus::Positive)
				return true;

			idv->ARTInitTime = t;
			idv->onART = true;

			return true;
		};

	return ef;
}