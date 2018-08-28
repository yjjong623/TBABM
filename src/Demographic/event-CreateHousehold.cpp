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

// Algorithm S5: Create a household. All but the first argument may be empty pointers
//   or sets
EventFunc TBABM::CreateHousehold(Pointer<Individual> head,
								 Pointer<Individual> spouse,
								 std::unordered_set<Pointer<Individual>> offspring,
								 std::unordered_set<Pointer<Individual>> other)
{
	EventFunc ef = 
		[this, head, spouse, offspring, other](double t, SchedulerT scheduler) {
			// printf("[%d] CreateHousehold\n", (int)t);
			auto household = std::make_shared<Household>(head, spouse, offspring, other);
			long hid = nHouseholds++;
			households[hid] = household;

			ChangeHousehold(head, hid, HouseholdPosition::Head);

			if (spouse)
				ChangeHousehold(spouse, hid, HouseholdPosition::Spouse);

			for (auto idv : offspring)
				ChangeHousehold(idv, hid, HouseholdPosition::Offspring);

			for (auto idv : other)
				ChangeHousehold(idv, hid, HouseholdPosition::Offspring);

			return true;
		};

	return ef;
}