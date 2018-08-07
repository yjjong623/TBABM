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
			double m_30   = params["HIV_m_30"].Sample(rng);
			int CD4       = idv->CD4count(t, m_30);
			// printf("[%d] ARTInitiate: %ld::%lu, CD4=%d\n", (int)t, idv->householdID, \
														// std::hash<Pointer<Individual>>()(idv), CD4);

			if (!idv || idv->dead || idv->hivStatus != HIVStatus::Positive)
				return true;

			idv->ARTInitTime = t;
			idv->onART = true;

			hivPositiveART.Record(t, +1);

			return true;
		};

	return ef;
}