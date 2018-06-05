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
			auto individuals = std::vector<Pointer<Individual>>{};
			long hid = nHouseholds++;

			// Head
			assert(head);
			head->householdID = hid;
			head->householdPosition = HouseholdPosition::Head;
			individuals.push_back(head);

			// Spouse
			if (spouse) {
				spouse->householdPosition = HouseholdPosition::Spouse;
				spouse->householdID = hid;
				individuals.push_back(spouse);
			}

			// Offspring
			for (auto it = offspring.begin(); it != offspring.end(); it++) {
				(*it)->householdPosition = HouseholdPosition::Offspring;
				(*it)->householdID = hid;
				individuals.push_back(*it);
			}

			// Other
			for (auto it = other.begin(); it != other.end(); it++) {
				(*it)->householdPosition = HouseholdPosition::Offspring;
				(*it)->householdID = hid;
				individuals.push_back(*it);
			}

			// Update .livedWithBefore for everybody
			for (size_t i = 0; i < individuals.size(); i++)
				for (size_t j = 0; j < individuals.size(); j++) {
					if (individuals[i] != individuals[j])
						individuals[i]->livedWithBefore.push_back(individuals[j]);
				}

			households[hid] = household;
			return true;
		};

	return ef;
}