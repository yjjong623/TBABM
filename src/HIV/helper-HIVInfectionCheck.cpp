#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
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
using DeathCause = Individual::DeathCause;
using Sex = Individual::Sex;

EventFunc TBABM::HIVInfectionCheck(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			// printf("[%d] HIVInfectionCheck\n", t);

			if (!idv || idv->dead || idv->hivStatus == HIVStatus::Positive)
				return true;

			int startYear   = constants["startYear"];
			int agWidth     = constants["ageGroupWidth"];
			int currentYear = startYear + (int)t/365;
			int gender      = idv->sex == Sex::Male ? 0 : 1;
			int age         = idv->age(t); // in years
			auto spouse     = idv->spouse;

			bool getsInfected {false};
			long double timeToProspectiveInfection {Uniform(0., agWidth)(rng.mt_)}; // in years

			// Different risk profiles for HIV+ and HIV- spouse. As of 12/03/18, these
			// profiles are the same, and are drawn from Excel Thembisa 4.1
			if (spouse && !spouse->dead && spouse->hivStatus == HIVStatus::Positive)
				getsInfected = fileData["HIV_risk_spouse"].getValue(currentYear, gender, age, rng);
			else
				getsInfected = fileData["HIV_risk"].getValue(currentYear, gender, age, rng);

			if (getsInfected)
				Schedule(t + 365*timeToProspectiveInfection, HIVInfection(idv));
			else
				Schedule(t + 365, HIVInfectionCheck(idv));

			return true;
		};

	return ef;
}