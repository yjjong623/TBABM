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

EventFunc TBABM::HIVInfection(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			using HIVStatus = Individual::HIVStatus;


			if (!idv || idv->dead || idv->hivStatus == HIVStatus::Positive)
				return true;

			// printf("infection\t\tage: %d\n", idv->age(t));

			idv->hivStatus = HIVStatus::Positive;

			// Decide CD4 count and value of 'k'
			idv->initialCD4 = params["CD4"].Sample(rng);
			idv->kgamma = params["kGamma"].Sample(rng);
			idv->t_HIV_infection = t;
			idv->hivDiagnosed = false;


			printf("[%d] HIVInfection: %lu:%f:%Lf\n", (int)t, std::hash<Pointer<Individual>>()(idv), idv->initialCD4, params["HIV_m_30"].Sample(rng));

			Schedule(t, VCTDiagnosis(idv));
			Schedule(t, MortalityCheck(idv));

			hivPositive.Record(t, +1);
			hivNegative.Record(t, -1);
			hivInfections.Record(t, +1);

			return true;
		};

	return ef;
}