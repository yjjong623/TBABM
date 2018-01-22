#pragma once

#include <unordered_set>

#include "Individual.h"

template <typename T>
using Pointer = std::shared_ptr<T>;

class Household {
public:
	Pointer<Individual> head;
	Pointer<Individual> spouse;
	std::unordered_set<Pointer<Individual>> offspring;
	std::unordered_set<Pointer<Individual>> other;

	void RemoveIndividual(Pointer<Individual> idv) {
		if (head == idv)
			head.reset();
		if (spouse == idv)
			spouse.reset();

		offspring.erase(idv);
		other.erase(idv);
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