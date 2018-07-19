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
	printf("[%d] HIVInfectionCheck\n", t);

	if (!idv || idv->dead || idv->hivStatus == HIVStatus::Positive)
		return;

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
		spouse->hivStatus == HIVStatus::Positive)
		HIVRisk = fileData["HIV_risk_spouse"].getValue(currentYear, gender, age, rng);
	else
		HIVRisk = fileData["HIV_risk"].getValue(currentYear, gender, age, rng);

	if ((timeToInfection = Exponential(HIVRisk)(rng.mt_)) < agWidth)
		Schedule(t + 365*timeToInfection, HIVInfection(idv));

	return;
}