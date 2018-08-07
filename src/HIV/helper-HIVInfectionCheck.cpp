#include "../../include/TBABM/TBABM.h"
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using namespace StatisticalDistributions;
using HIVStatus = Individual::HIVStatus;
using Sex = Individual::Sex;

void TBABM::HIVInfectionCheck(int t, Pointer<Individual> idv)
{
	// printf("[%d] HIVInfectionCheck\n", t);

	if (!idv || idv->dead || idv->hivStatus == HIVStatus::Positive) {
		// printf("\tPassing over, HIV+\n");
		return;
	}

	int startYear   = constants["startYear"];
	int agWidth     = constants["ageGroupWidth"];
	int currentYear = startYear + (int)t/365;
	int gender      = idv->sex == Sex::Male ? 0 : 1;
	int age         = idv->age(t); // in years
	auto spouse     = idv->spouse;

	double HIVRisk;
	double timeToInfection; // in years

	// Different risk profiles for HIV+ and HIV- spouse
	if (spouse && !spouse->dead &&
		spouse->hivStatus == HIVStatus::Positive) {
		// printf("Calculating spousal HIV risk for currentYear=%d, gender=%d, age=%d\n", currentYear, gender, age);		
		HIVRisk = fileData["HIV_risk_spouse"].getValue(currentYear, gender, age, rng);
	}
	else {
		// printf("Calculating non-spousal HIV risk for currentYear=%d, gender=%d, age=%d\n", currentYear, gender, age);
		HIVRisk = fileData["HIV_risk"].getValue(currentYear, gender, age, rng);
	}

	// printf("Rate: %f\n", HIVRisk);
	if ((timeToInfection = Exponential(HIVRisk)(rng.mt_)) < agWidth)
		Schedule(t + 365*timeToInfection, HIVInfection(idv));

	return;
}