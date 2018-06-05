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

using Sex = Individual::Sex;

using namespace StatisticalDistributions;

// Algorithm S14: Divorce
EventFunc TBABM::Divorce(Pointer<Individual> m, Pointer<Individual> f)
{
	EventFunc ef = 
		[this, m, f](double t, SchedulerT scheduler) {
			// printf("[%d] Divorce, populationSize=%lu, people: m=%ld::%lu f=%ld::%lu\n", (int)t, population.size(), m->householdID, std::hash<Pointer<Individual>>()(m), f->householdID, std::hash<Pointer<Individual>>()(f));

			// If someone is dead they can't divorce
			if (m->dead || f->dead)
				return true;

			// Change marriage status to divorced
			m->marriageStatus = MarriageStatus::Divorced;
			f->marriageStatus = MarriageStatus::Divorced;

			// Select who is to leave the household
			auto booted = (m->householdPosition == HouseholdPosition::Head) ? f : m;

			// Identify a new household for whoever left
			int newHouseholdID = -1;
			for (size_t i = 0; i < booted->offspring.size(); i++)
				if (booted->offspring[i]->householdID != m->householdID && \
					households[booted->offspring[i]->householdID])
					newHouseholdID = booted->offspring[i]->householdID;

			if (booted->mother && !booted->mother->dead && booted->mother->householdID != m->householdID && \
				households[booted->mother->householdID != m->householdID])
				newHouseholdID = booted->mother->householdID;

			if (booted->father && !booted->father->dead && booted->father->householdID != m->householdID && \
				households[booted->father->householdID])
				newHouseholdID = booted->father->householdID;

			if (newHouseholdID > -1) {
				assert(households[newHouseholdID]);

				// If another household was found for the booted individual
				ChangeHousehold(booted, newHouseholdID, HouseholdPosition::Other);
			} else {
				Schedule(t, CreateHousehold(booted, Pointer<Individual>(), {}, {}));
			}

			divorces.Record(t, +1);
			return true;
		};

	return ef;
}
