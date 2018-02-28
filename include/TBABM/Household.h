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
		printf("RemoveIndividual: BEGINNING\n");

		assert(idv);

		assert(head == idv || spouse == idv || offspring.count(idv) == 1 || other.count(idv) == 1);

		if (idv->householdPosition == HouseholdPosition::Head)
			assert(head == idv);
		else if (idv->householdPosition == HouseholdPosition::Spouse)
			assert(spouse == idv);
		else if (idv->householdPosition == HouseholdPosition::Offspring)
			assert(offspring.count(idv) == 1);
		else if (idv->householdPosition == HouseholdPosition::Other)
			assert(other.count(idv) == 1);

		if (head == idv) {
			head.reset();
			// Find a new head
			if (spouse) {
				head = spouse;
				head->householdPosition = HouseholdPosition::Head;
				spouse.reset();
			} else {
				if (offspring.size() > 1) {
					head = *offspring.begin();
					offspring.erase(offspring.begin());
					head->householdPosition = HouseholdPosition::Head;
				} else if (other.size() > 1) {
					head = *other.begin();
					other.erase(other.begin());
					head->householdPosition = HouseholdPosition::Head;
				}
			}
		}
		if (spouse == idv)
			spouse.reset();

		offspring.erase(idv);
		other.erase(idv);
		printf("RemoveIndividual: SUCCESS\n");
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