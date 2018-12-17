#include <cstdio>
#include <iostream>
#include <random>

#include "../include/TBABM/HouseholdGen.h"

Pointer<Household>
HouseholdGen::GetHousehold(const int hid)
{
	// Create a blank Household object
	auto household = std::make_shared<Household>();

	// Grab a random number between 0 and size(dataset)-1
	std::random_device rd;
	std::mt19937 gen(rd());
	size_t size = families.size();
	std::uniform_int_distribution<> dis(0, size-1);
	auto fid = dis(gen);


	// Retrieve the corresponding vector
	MicroFamily family = families[fid];

	// The first element in the vector is the head of the
	//   household; create an Individual using the smaller
	//   constructor
	MicroIndividual _head = family[0];
	printf("alive1\n");
	auto head = std::make_shared<Individual>(hid, 0-365*_head.age, _head.sex, _head.role, MarriageStatus::Single); //, params, fileData);
	printf("alive2\n");
	auto newIndividuals = std::vector<Pointer<Individual>>{head};

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
		auto idv = std::make_shared<Individual>(hid, 0-365*midv.age, midv.sex, midv.role, MarriageStatus::Single); //, params, fileData);
		newIndividuals.push_back(idv);
		switch (idv->householdPosition) {
			case (HouseholdPosition::Head):
				std::cout << "Error: Can't have two household heads" << std::endl;
				break;
			case (HouseholdPosition::Spouse):
				idv->marriageStatus = MarriageStatus::Married;

				household->spouse = idv;
				
				idv->spouse = household->head; // bidirectional
				household->head->spouse = idv;
				household->head->marriageStatus = MarriageStatus::Married;
				break;
			case (HouseholdPosition::Offspring):
				// Add offspring to head and spouse
				household->head->offspring.push_back(idv);
				if (household->spouse)
					household->spouse->offspring.push_back(idv);

				// Add offspring to household
				household->offspring.insert(idv);

				// Add mat/paternity to offspring
				if (household->head->sex == Sex::Male)
					idv->father = household->head;
				else
					idv->mother = household->head;

				if (household->spouse) {
					if (household->spouse->sex == Sex::Female) {
						idv->mother = household->spouse;
					}
					else {
						idv->father = household->spouse;
					}
				}
				
				break;
			case (HouseholdPosition::Other):
				idv->marriageStatus = MarriageStatus::Single;
				household->other.insert(idv);
				break;
			default:
				printf("\t\tError!!\n");
		}
	}

	// Keep track of who the individuals have lived with before
	for (size_t i = 0; i < newIndividuals.size(); i++)
		for (size_t j = 0; j < newIndividuals.size(); j++)
			if (newIndividuals[i] != newIndividuals[j])
				newIndividuals[i]->livedWithBefore.push_back(newIndividuals[j]);

	return household;
}