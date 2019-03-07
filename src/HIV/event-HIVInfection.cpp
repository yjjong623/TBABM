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

void HIVInfectionLogger(Pointer<Individual> idv, double t)
{
	std::cout << termcolor::on_magenta << 
		"[" << 
		std::left << std::setw(12) << 
		idv->Name() << 
		std::setw(5) << std::right << 
		(int)t << "] HIV infection" << 
		termcolor::reset << std::endl;
}

EventFunc TBABM::HIVInfection(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {

			// Have to be alive and seronegative to get infected
			if (!idv || idv->dead || idv->hivStatus == HIVStatus::Positive)
				return true;

			// HIVInfectionLogger(idv, t);

			idv->hivStatus = HIVStatus::Positive;

			// Decide CD4 count and value of 'k', and record as undiagnosed
			idv->initialCD4 = params["CD4"].Sample(rng);
			idv->kgamma = params["kGamma"].Sample(rng);
			idv->t_HIV_infection = t;
			idv->hivDiagnosed = false;

			// Immediately schedule possible VCT diagnosis, and begin checking
			// their HIV-related mortality
			Schedule(t, VCTDiagnosis(idv));
			Schedule(t, MortalityCheck(idv));

			// Reevaluate risk for tuberculosis, and change risk window
			idv->tb.RiskReeval(t);

			int sex {idv->sex == Sex::Male ? 0 : 1};
			int age {idv->age(t)};

			hivPositive.Record(t, +1);
			hivNegative.Record(t, -1);
			hivInfections.Record(t, +1);
			hivPositivePyramid.UpdateByAge(t, sex, age, +1);
			hivInfectionsPyramid.UpdateByAge(t, sex, age, +1);

			return true;
		};

	return ef;
}