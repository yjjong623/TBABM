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

// Algorithm S4: Adding new residents
EventFunc TBABM::NewHouseholds(int num)
{
	EventFunc ef = 
		[this, num](double t, SchedulerT scheduler) {
			// printf("[%d] NewHouseholds\n", (int)t);
			int popChange = 0; // Number of people created so far

			while (popChange < num) {
				long hid = nHouseholds++;
				Pointer<Household> hh = householdGen.GetHousehold(hid);
				households[hid] = hh;

				// Insert all members of the household into the population
				population.insert(hh->head); popChange++;
				if (hh->spouse){
					population.insert(hh->spouse);
					popChange++;
				}
				for (auto it = hh->offspring.begin(); it != hh->offspring.end(); it++) {
					popChange++;
					population.insert(*it);
				}
				for (auto it = hh->other.begin(); it != hh->other.end(); it++) {
					popChange++;
					population.insert(*it);
				}
			}

			populationSize.Record(t, popChange);

			return true;
		};

	return ef;
}