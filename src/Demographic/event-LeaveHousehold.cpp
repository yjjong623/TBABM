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

using namespace StatisticalDistributions;

// Algorithm S11: Leave current household to form new household
EventFunc TBABM::LeaveHousehold(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			// printf("[%d] LeaveHousehold: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));
			if (!idv || idv->dead)
				return true;
			
			
			Schedule(t, CreateHousehold(idv, \
										Pointer<Individual>{}, \
										{},\
										{}));

			return true;
		};

	return ef;
}