#include <cstdio>
#include <iostream>
#include <random>

#include "../include/TBABM/HouseholdGen.h"

Pointer<Household>
HouseholdGen::GetHousehold(const int hid, const RNG &rng)
{
	// Create a blank Household object
	auto household = std::make_shared<Household>();

	// Grab a random number between 0 and size(dataset)-1
	size_t size = families.size();
	std::uniform_int_distribution<> dis(0, size-1);
	auto fid = dis(rng);

	// Retrieve the corresponding vector
	MicroFamily family = families[fid];

	// The first element in the vector is the head of the
	//   household; create an Individual using the smaller
	//   constructor
	MicroIndividual _head = family[0];
	auto head = std::make_shared<Individual>(hid, _head.age, _head.sex, _head.role);

	// Add this object to the household as the head
	household->head = head;

	// Go through the remaining elements:
	//   Establish relationships between the head
	//   and the current person under consideration
	//   Add them to the household
	// Return the household
	for (int i = 1; i < family.size(); ++i)
	{
		MicroIndividual midv = family[i];
		auto idv = std::make_shared<Individual>(hid, midv.age, midv.sex, midv.role);
		switch (idv.householdPosition) {
			case (HouseholdPosition::Head):
				cout << "Error: Can't have two household heads" << endl;
				break;
			case (HouseholdPosition::Spouse):
				household.spouse = idv;
				idv->spouse = household->head;
				household.head->spouse = idv;
				break;
			case (HouseholdPosition::Offspring):
				// Add offspring to head and spouse
				household.head->offspring.push_back(idv);
				if (household.spouse)
					household.spouse->offspring.push_back(idv);

				// Add offspring to household
				household.offspring.push_back(idv);

				// Add mat/paternity to offspring
				if (household->head.sex == Sex::Male)
					idv->father = household.head;
				else
					idv->mother = household.head;

				if (household.spouse)
					if (household.spouse->sex == Sex::Female)
						idv->mother = household.spouse;
					else
						idv->father = household.spouse;

				break;
			case (HouseholdPosition::Other):
				household.other.push_back(idv);
				break;
		}
	}
}