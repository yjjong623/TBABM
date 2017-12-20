#pragma once

#include <unordered_set>

#include "Individual.h"

template <typename T>
using Pointer = std::shared_ptr<T>;

class Household {
public:
	Pointer<Individual> head;
	Pointer<Inividual> spouse;
	std::unordered_set<Pointer<Individual>> offspring;
	std::unordered_set<Pointer<Individual>> other;

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