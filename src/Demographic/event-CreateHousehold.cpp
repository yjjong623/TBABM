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

// Algorithm S5: Create a household. All but the first argument may be empty pointers
//   or sets
EventFunc TBABM::CreateHousehold(Pointer<Individual> head,
								 Pointer<Individual> spouse,
								 std::vector<Pointer<Individual>> offspring,
								 std::vector<Pointer<Individual>> other)
{
	EventFunc ef = 
		[this, head, spouse, offspring, other](double t, SchedulerT scheduler) {
			// printf("[%d] CreateHousehold\n", (int)t);
			long hid = nHouseholds++;
			auto household = std::make_shared<Household>(head, spouse, offspring, other, t, hid);
			households[hid] = household;

			ChangeHousehold(head, t, hid, HouseholdPosition::Head);

			if (spouse)
				ChangeHousehold(spouse, t, hid, HouseholdPosition::Spouse);

			for (auto idv : offspring)
				ChangeHousehold(idv, t, hid, HouseholdPosition::Offspring);

			for (auto idv : other)
				ChangeHousehold(idv, t, hid, HouseholdPosition::Offspring);

			return true;
		};

	return ef;
}