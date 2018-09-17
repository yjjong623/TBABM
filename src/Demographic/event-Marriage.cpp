#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using Sex 			 = Individual::Sex;
using MarriageStatus = Individual::MarriageStatus;

using namespace StatisticalDistributions;

// Algorithm S13: Marriage
EventFunc TBABM::Marriage(Pointer<Individual> m, Pointer<Individual> f)
{
	EventFunc ef = 
		[this, m, f](double t, SchedulerT scheduler) {

			if (population.count(m) != 1 || population.count(f) != 1) {
				// printf("\tA spouse is dead!\n");
				return true;
			}

			// printf("[%d] Marriage, populationSize=%lu, people: m=%ld::%lu f=%ld::%lu\n", (int)t, population.size(), m->householdID, std::hash<Pointer<Individual>>()(m), f->householdID, std::hash<Pointer<Individual>>()(f));
			assert(m);
			assert(f);
			assert(m != f);

			if (m->dead || f->dead)
				return true;

			bool canDivorce = true;

			if (households[m->householdID]->size() == 1) {
				// The female will join the male's household
				ChangeHousehold(f, m->householdID, HouseholdPosition::Spouse);

				if (f->offspring.size() > 0)
					canDivorce = false;

				for (auto idv : f->offspring)
					ChangeHousehold(idv, m->householdID, HouseholdPosition::Offspring);

			} else if (households[f->householdID]->size() == 1) {
				// Male joins female household
				ChangeHousehold(m, f->householdID, HouseholdPosition::Spouse);

				if (m->offspring.size() > 0)
					canDivorce = false;

				for (auto idv : f->offspring)
					ChangeHousehold(idv, f->householdID, HouseholdPosition::Offspring);

			} else {
				// Couple forms new household?
				if (params["coupleFormsNewHousehold"].Sample(rng) == 1) {
					auto hid = nHouseholds++;
					households[hid] = std::make_shared<Household>();

					ChangeHousehold(m, hid, HouseholdPosition::Head);
					ChangeHousehold(f, hid, HouseholdPosition::Spouse);

					if (f->offspring.size() > 0 || m->offspring.size() > 0)
						canDivorce = false;

					for (auto idv : f->offspring)
						ChangeHousehold(idv, hid, HouseholdPosition::Offspring);
					for (auto idv : m->offspring)
						ChangeHousehold(idv, hid, HouseholdPosition::Offspring);

				} else {
					ChangeHousehold(f, m->householdID, HouseholdPosition::Spouse);

					if (f->offspring.size() > 0)
						canDivorce = false;

					for (auto idv : f->offspring)
						ChangeHousehold(idv, m->householdID, HouseholdPosition::Offspring);
				}
			}

			m->spouse = f;
			f->spouse = m;

			m->marriageStatus = MarriageStatus::Married;
			f->marriageStatus = MarriageStatus::Married;

			m->marriageDate = t;
			f->marriageDate = t;

			// Will they divorce?
			if (params["probabilityOfDivorce"].Sample(rng) == 1 && canDivorce) {
				// Time to divorce
				double coupleAvgAge = (m->age(t) + f->age(t))/(2*365);
				double yearsToDivorce = fileData["timeInMarriage"].getValue(0,0,coupleAvgAge,rng);
				int daysToDivorce = 365 * yearsToDivorce;
				Schedule(t + daysToDivorce, Divorce(m, f));
			}


			// Time to first birth
			double yearsToFirstBirth = fileData["timeToFirstBirth"].getValue(0,0,f->age(t),rng);
			int daysToFirstBirth = 365 * yearsToFirstBirth;

			Schedule(t + daysToFirstBirth - 9*30, Pregnancy(m, f));

			marriages.Record(t, +1);

			return true;
		};

	return ef;
}