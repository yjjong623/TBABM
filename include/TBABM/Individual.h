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
		Negative, Positive
	};

	enum class TBStatus {
		Susceptible, Latent, Infective
	};

	long householdID;

	int birthDate; // In units of 't'
	Sex sex;
	long double marriageDate;
	bool pregnant;

	Pointer<Individual> spouse;
	Pointer<Individual> mother;
	Pointer<Individual> father;
	std::vector<Pointer<Individual>> offspring; // Can have multiple children	

	std::vector<Pointer<Individual>> livedWithBefore; // People lived with before

	HouseholdPosition householdPosition;
	MarriageStatus marriageStatus;

	// HIV stuff
	int t_HIV_infection;
	HIVStatus hivStatus;
	bool hivDiagnosed;
	double initialCD4;
	double kgamma;
	bool onART;
	int ARTInitTime;

	// TB stuff
	TBStatus tbStatus;
	bool hasTB() {
		return tbStatus == TBStatus::Infective; 
	}

	bool dead;

	int age(double t) {
		return (t - birthDate) / 365;
	}

	double CD4count(double t_cur, double m_30) {
		if (hivStatus == HIVStatus::Negative)
			return 0;

		double t = onART ? ARTInitTime : t_cur;

		double gender   = (sex == Sex::Female) ? 0.9 : 1.0;
		double m_a      = m_30 * pow(1+kgamma,(age(t)-30)/10);
		double t_HIV    = (t - t_HIV_infection - 3*14) / 365;
		double CD4NoART = std::max(0., initialCD4 - gender*m_a*t_HIV);

		if (!onART)
			return CD4NoART;

		double nYears = (t_cur - ARTInitTime)/365;
		double increase;
		if (initialCD4 < 50)
			increase = 414 - 369/std::pow(2, nYears/1.135) - 47;
		else if (50 <= initialCD4 && initialCD4 < 200)
			increase = 517 - 366/std::pow(2, nYears/1.153) - 152;
		else if (200 <= initialCD4 && initialCD4 < 350)
			increase = 661 - 365/std::pow(2, nYears/1.14) - 296;
		else if (350 <= initialCD4 && initialCD4 < 500)
			increase = 803 - 368/std::pow(2, nYears/1.16) - 434;
		else if (500 <= initialCD4)
			increase = 956 - 366/std::pow(2, nYears/1.15) - 592;

		return CD4NoART + increase;
	}

	Individual(long householdID, int birthDate, Sex sex,
			   Pointer<Individual> spouse,
			   Pointer<Individual> mother,
			   Pointer<Individual> father,
			   std::vector<Pointer<Individual>> offspring,
			   HouseholdPosition householdPosition,
			   MarriageStatus marriageStatus) :
	  householdID(householdID), 
	  birthDate(birthDate), 
	  sex(sex), 
	  pregnant(false),
	  spouse(spouse),
	  mother(mother), 
	  father(father), 
	  offspring(offspring),
	  householdPosition(householdPosition),
	  marriageStatus(marriageStatus),
	  onART(false),
	  hivStatus(HIVStatus::Negative),
	  dead(false) {};
	
	Individual(long hid, int birthDate, Sex sex, HouseholdPosition householdPosition,
			   MarriageStatus marriageStatus) :
	  Individual(hid, birthDate, sex, Pointer<Individual>(), Pointer<Individual>(), Pointer<Individual>(), {}, householdPosition, marriageStatus) {};
private:
};