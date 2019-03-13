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

	void AddIndividual(Pointer<Individual> idv, int t, HouseholdPosition hp);

	void RemoveIndividual(Pointer<Individual> idv, int t);

	void PrintHousehold(int t);

	// Right now, these functions mean the same thing,
	// and 't' is not used.
	int size(int t);
	int size(void);

	bool hasMember(Pointer<Individual> idv);

	// ActiveTBPrevalence is simply the fraction of individuals
	// in the household who have active TB. Right now,
	// ActiveTBPrevalence(1) defers to ActiveTBPrevalence().
	double ActiveTBPrevalence(void);
	double ActiveTBPrevalence(int t);

	// ContactActiveTBPrevalence returne the fraction of household
	// contacts (excluding the individual with the passed-in TBStatus)
	// who have active TB.
	double ContactActiveTBPrevalence(TBStatus);
	double ContactActiveTBPrevalence(TBStatus, int t);

	Household(Pointer<Individual> head,
			  Pointer<Individual> spouse,
			  std::vector<Pointer<Individual>> offspring,
			  std::vector<Pointer<Individual>> other,
			  int t,
			  int hid) : 
		head(head), 
		spouse(spouse), 
		offspring(offspring), 
		other(other),
		hid(hid),
		nInfectiousTBIndivduals(0) {

			nIndividuals = (head?1:0) + 
						   (spouse?1:0) + 
						   offspring.size() + 
						   other.size();

			AddIndividual(head, t, HouseholdPosition::Head);
			AddIndividual(spouse, t, HouseholdPosition::Spouse);

			for (auto idv : offspring)
				AddIndividual(idv, t, HouseholdPosition::Offspring);
			for (auto idv : other)
				AddIndividual(idv, t, HouseholdPosition::Other);
		}

	Household(int t, int hid) : 
		Household({}, {}, {}, {}, t, hid) {}

private:
	int hid;
	int nIndividuals;
	int nInfectiousTBIndivduals;

	void TriggerReeval(int t, Pointer<Individual> idv);
};