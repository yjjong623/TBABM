#include "../../include/TBABM/TBABM.h"
#include <Empirical.h>
#include <Exponential.h>
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

EventFunc TBABM::MortalityCheck(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			// printf("[%d] MortalityCheck: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));

			double ageGroupWidth = constants["ageGroupWidth"];

			// Make sure the individual is alive and HIV-positive
			if (!idv || idv->dead || idv->hivStatus != HIVStatus::Positive)
				return true;

			// Retrieve constant parameters m, p, and m_30
			double m = params["HIV_m"].Sample(rng);
			double p = params["HIV_p"].Sample(rng);
			double m_30 = params["HIV_m_30"].Sample(rng);
			
			// Calculate current CD4 count
			double CD4 = idv->CD4count(t, m_30);
			
			// Calculate rate of death for this individual, varying
			// on CD4 count
			double M_c = m*exp(-1*p*CD4);

			// Calculate time to HIV-related death, and schedule it if
			// it occurs before next MortalityCheck and CD4 count is low
			// Unit: YEARS
			double timeToMortality = Exponential(M_c)(rng.mt_);

			// printf("timeToMortality=%f, m=%f, p=%f, m_30=%f, CD4=%f\n", timeToMortality, m, p, m_30, CD4);

			if (timeToMortality < ageGroupWidth && CD4 < 350)
				Schedule(t + 365*timeToMortality, Death(idv, DeathCause::HIV));
			else
				Schedule(t + 365*ageGroupWidth, MortalityCheck(idv));
			
			return true;
		};

	return ef;
}