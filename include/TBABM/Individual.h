#pragma once

template <typename T>
using Pointer = std::shared_ptr<T>;

class Individual {
public:
	enum class Sex {
		Male, Female
	};

	enum class HouseholdPosition {
		Head, Spouse, Offspring, Other
	};

	enum class MarriageStatus {
		Single, Married, Divorced, Looking
	};

	enum class HIVStatus {
		Negative, PositiveNIC, PositiveIC, PositiveART
	};

	enum class TBStatus {
		Susceptible,
		LatentTN, LatentTC, LatentTCIPT, LatentTI,
		InfectiveTC, InfectiveTN, InfectiveTI
	};

	long householdID;

	int age;
	Sex sex;

	Pointer<Individual> spouse;
	Pointer<Individual> mother;
	Pointer<Individual> father;
	std::vector<Pointer<Individual>> offspring; // Can have multiple children	

	HouseholdPosition householdPosition;
	// HIVStatus hivStatus;
	// TBStatus tbStatus;

	Individual(long hid, int age, Sex sex,
			   Pointer<Individual> spouse,
			   Pointer<Individual> mother,
			   Pointer<Individual> father,
			   std::vector<Pointer<Individual>> offspring,
			   HouseholdPosition householdPosition);
	
	Individual(long hid, int age, Sex sex, HouseholdPosition householdPosition) :
	  Individual(hid, age, sex, Pointer<Individual>(), Pointer<Individual>(), Pointer<Individual>(), {}, householdPosition) {};

private:
};