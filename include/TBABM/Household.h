#pragma once

#include <unordered_set>
#include <cassert>

#include "Individual.h"

template <typename T>
using Pointer = std::shared_ptr<T>;

using Sex = Individual::Sex;
using HouseholdPosition = Individual::HouseholdPosition;

class Household {
public:
	Pointer<Individual> head;
	Pointer<Individual> spouse;
	std::unordered_set<Pointer<Individual>> offspring;
	std::unordered_set<Pointer<Individual>> other;

	void RemoveIndividual(Pointer<Individual> idv) {

		assert(idv);

		bool c1 = head == idv;
		bool c2 = spouse == idv;
		bool c3 = offspring.count(idv) == 1;
		bool c4 = other.count(idv) == 1;
		assert(c1 || c2 || c3 || c4);


		if (head == idv) {
			head.reset();
			// Find a new head
			if (spouse) {
				head = spouse;
				head->householdPosition = HouseholdPosition::Head;
				spouse.reset();
			} else if (offspring.size() > 0) {
					for (auto it = offspring.begin(); it != offspring.end(); it++) {
						if (*it && !(*it)->dead) {
							head = *it;
							head->householdPosition = HouseholdPosition::Head;
							offspring.erase(*it);
							break;
						}
					}
			} else if (other.size() > 0) {
				for (auto it = other.begin(); it != other.end(); it++) {
					if (*it && !(*it)->dead) {
						head = *it;
						head->householdPosition = HouseholdPosition::Head;
						other.erase(*it);
						break;
					}
				}
			} else {
				// There is nobody to replace the head.
			}
		}
		if (spouse == idv)
			spouse.reset();
		offspring.erase(idv);
		other.erase(idv);
	}

	void PrintHousehold(int t) {
		printf("[%d] Printing household\n", t);
		if (head)
			printf("\t[H]  %c %d ", head->sex == Sex::Male ? 'M' : 'F', (t-head->birthDate)/365);

		if (spouse)
			printf("\t[S]  %c %d ", spouse->sex == Sex::Male ? 'M' : 'F', (t-spouse->birthDate)/365);

		for (auto it = offspring.begin(); it != offspring.end(); it++) {
			assert(*it);
			printf("\t[Of] %c %d ", (*it)->sex == Sex::Male ? 'M' : 'F', (t-(*it)->birthDate)/365);
		}

		for (auto it = other.begin(); it != other.end(); it++) {
			assert(*it);
			printf("\t[Ot] %c %d ", (*it)->sex == Sex::Male ? 'M' : 'F', (t-(*it)->birthDate)/365);
		}
		printf("\n");
	}

	int size(void) {
		return (head ? 1:0) + (spouse ? 1:0) + offspring.size() + other.size();
	}

	bool hasMember(Pointer<Individual> idv) {
		return head == idv || spouse == idv || offspring.count(idv) == 1 || other.count(idv) == 1;
	}

	Household(Pointer<Individual> head,
			  Pointer<Individual> spouse,
			  std::unordered_set<Pointer<Individual>> offspring,
			  std::unordered_set<Pointer<Individual>> other) : 
		head(head), 
		spouse(spouse), 
		offspring(offspring), 
		other(other) {}

	Household() : 
		head(), 
		spouse(), 
		offspring({}), 
		other({}) {}
};