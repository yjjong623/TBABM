#pragma once

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
	std::vector<Pointer<Individual>> offspring;
	std::vector<Pointer<Individual>> other;

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
				other.push_back(head);
			}
			head = idv;
		}
		else if (hp == HouseholdPosition::Spouse) {
			if (spouse) {
				other.push_back(spouse);
				spouse->householdPosition = HouseholdPosition::Other;
			}
			spouse = idv;
		}
		else if (hp == HouseholdPosition::Other) {
			other.push_back(idv);
		}
		else if (hp == HouseholdPosition::Offspring) {
			offspring.push_back(idv);
		}

		return;
	}

	void RemoveIndividual(Pointer<Individual> idv) {
		assert(idv);
		if (!hasMember(idv))
			return;
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
							offspring.erase(it);
							break;
						}
					}
			} else if (other.size() > 0) {
				for (auto it = other.begin(); it != other.end(); it++) {
					if (*it && !(*it)->dead) {
						head = *it;
						head->householdPosition = HouseholdPosition::Head;
						other.erase(it);
						break;
					}
				}
			} else {
				// There is nobody to replace the head.
			}
		}


		if (spouse == idv)
			spouse.reset();


		for (auto it = offspring.begin(); it != offspring.end(); it++) {
			if (*it == idv) {
				offspring.erase(it);
				break;
			}
		}

		for (auto it = other.begin(); it != other.end(); it++)
			if (*it == idv) {
				other.erase(it);
				break;
			}

		return;
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
		if ((head == idv) || (spouse == idv))
			return true;

		for (auto candidate : offspring)
			if (candidate == idv)
				return true;

		for (auto candidate : other)
			if (candidate == idv)
				return true;

		return false;
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
			return 0.;
		else if (size() == 1)
			return numInfected;
		else
			return numInfected/((double)size() - 1);
	}

	Household(Pointer<Individual> head,
			  Pointer<Individual> spouse,
			  std::vector<Pointer<Individual>> offspring,
			  std::vector<Pointer<Individual>> other) : 
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