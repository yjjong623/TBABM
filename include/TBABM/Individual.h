#pragma once
#include <memory>

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

	int birthDate; // In units of 't'
	Sex sex;
	long double marriageDate;

	Pointer<Individual> spouse;
	Pointer<Individual> mother;
	Pointer<Individual> father;
	std::vector<Pointer<Individual>> offspring; // Can have multiple children	

	std::vector<Pointer<Individual>> livedWithBefore; // People lived with before

	HouseholdPosition householdPosition;
	MarriageStatus marriageStatus;
	// HIVStatus hivStatus;
	// TBStatus tbStatus;

	bool dead;

	int age(double t) {
		return (t - birthDate) / 365;
	}

	Individual(long householdID, int birthDate, Sex sex,
			   Pointer<Individual> spouse,
			   Pointer<Individual> mother,
			   Pointer<Individual> father,
			   std::vector<Pointer<Individual>> offspring,
			   HouseholdPosition householdPosition,
			   MarriageStatus marriageStatus) :
	  householdID(householdID), birthDate(birthDate), sex(sex), spouse(spouse),
	  mother(mother), father(father), offspring(offspring),
	  householdPosition(householdPosition),
	  marriageStatus(marriageStatus),
	  dead(false) {};
	
	Individual(long hid, int birthDate, Sex sex, HouseholdPosition householdPosition,
			   MarriageStatus marriageStatus) :
	  Individual(hid, birthDate, sex, Pointer<Individual>(), Pointer<Individual>(), Pointer<Individual>(), {}, householdPosition, marriageStatus) {};
private:
};