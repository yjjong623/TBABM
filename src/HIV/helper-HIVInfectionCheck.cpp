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

EventFunc TBABM::HIVInfectionCheck(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			// printf("[%s %d] HIVInfectionCheck\n", idv->Name().c_str(), (int)t);
			// printf("Use count: %ld\n", idv.use_count());
			if (!idv.use_count())
				return true;

			if (idv->dead || idv->hivStatus == HIVStatus::Positive)
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