#pragma once

#include <unordered_set>
#include <cassert>

#include "Individual.h"
#include "IndividualTypes.h"
#include "TBTypes.h"

template <typename T>
using Pointer = std::shared_ptr<T>;

class Household {
public:
	Pointer<Individual> head;
	Pointer<Individual> spouse;
	std::unordered_set<Pointer<Individual>> offspring;
	std::unordered_set<Pointer<Individual>> other;

	void AddIndividual(Pointer<Individual> idv, HouseholdPosition hp, int hid) {

		// Avoid adding a dead individual to the household
		assert(idv && !idv->dead);

		// Update role and HID for new household member
		idv->householdPosition = hp;
		idv->householdID 	   = hid;

		// Update 'livedWithBefore' for new member, and all current residents
		idv->LivedWith(head);
		idv->LivedWith(spouse);

		if (head)   head->LivedWith(idv);
		if (spouse) spouse->LivedWith(idv);

		for (auto it = offspring.begin(); it != offspring.end(); it++) {
			if (!*it || (*it)->dead) continue;
			(*it)->LivedWith(idv);
			idv->LivedWith(*it);
		}

		for (auto it = other.begin(); it != other.end(); it++) {
			if (!*it || (*it)->dead) continue;
			(*it)->LivedWith(idv);
			idv->LivedWith(*it);
		}

		// Insert new member into the population
		if (hp == HouseholdPosition::Head) {
			if (head) {
				head->householdPosition = HouseholdPosition::Other;
				other.insert(head);
			}
			head = idv;
		}
		else if (hp == HouseholdPosition::Spouse) {
			if (spouse) {
				other.insert(spouse);
				spouse->householdPosition = HouseholdPosition::Other;
			}
			spouse = idv;
		}
		else if (hp == HouseholdPosition::Other) {
			other.insert(idv);
		}
		else if (hp == HouseholdPosition::Offspring) {
			offspring.insert(idv);
		}

		return;
	}

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
		return (head   && !head->dead   ? 1:0) + \
			   (spouse && !spouse->dead ? 1:0) + \
			   offspring.size() + \
			   other.size();
	}

	bool hasMember(Pointer<Individual> idv) {
		return head == idv || spouse == idv || offspring.count(idv) == 1 || other.count(idv) == 1;
	}

	double TBPrevalence(int t) {
		double numInfected {0};

		numInfected += head->TB.GetTBStatus(t) == TBStatus::Infectious ? 1 : 0;

		if (spouse)
			numInfected += spouse->TB.GetTBStatus(t) == TBStatus::Infectious ? 1 : 0;

		for (auto idv : offspring)
			numInfected += idv->TB.GetTBStatus(t) == TBStatus::Infectious ? 1 : 0;

		for (auto idv : other)
			numInfected += idv->TB.GetTBStatus(t) == TBStatus::Infectious ? 1 : 0;

		if (size() == 0)
			return 0.
		else if (size() == 1)
			return numInfected;
		else
			return numInfected/((double)size() - 1);
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