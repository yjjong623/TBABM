#include "../../include/TBABM/TBABM.h"
#include "../../include/TBABM/SurveyUtils.h"
#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using Sex = Individual::Sex;
using MarriageStatus = Individual::MarriageStatus;
using HIVStatus = Individual::HIVStatus;
using DeathCause = Individual::DeathCause;

using namespace StatisticalDistributions;


// Algorithm S10: Natural death
EventFunc TBABM::Death(Pointer<Individual> idv, DeathCause deathCause)
{
	EventFunc ef = 
		[this, idv, deathCause](double t, SchedulerT scheduler) {

			// Be sure this individual is alive
			if (!idv || idv->dead)
				return true;

			// Look up household, and assert that this household actually exists
			auto household = households[idv->householdID];
			assert(household);

			// printf("[%d] Death: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));

			// Eliminate from Looking pools
			if (idv->sex == Sex::Male)
				maleSeeking.erase(idv);
			else
				femaleSeeking.erase(idv);

			// Advise spouse that they are now widowed
			if (idv->spouse)
				idv->spouse->Widowed();

			int age = idv->age(t);
			int sex = idv->sex == Sex::Male ? 0 : 1;

			idv->dead = true;

			// This purges 'idv' from the 'livedWithBefore' records
			// of all who have lived with 'idv'
			DeleteIndividual(idv);

			// Removes the individual from the household and elects a 
			// new head, if 'idv' was the head of his or her household
			household->RemoveIndividual(idv);

			// The call to 'RemoveIndivdual' could cause a house to no 
			// longer have any members in it. In this case, remove the
			// household
			if (household->size() == 0)
				household.reset();

			// With 'idv' cut out of others records, and their household,
			// it is now safe to erase them from the population
			population.erase(idv);
			
			// Update various metrics
			populationSize.Record(t, -1);
			deaths.Record(t, +1);
			deathPyramid.UpdateByAge(t, sex, age, +1);

			if (idv->hivStatus == HIVStatus::Positive)
				hivPositive.Record(t, -1);

			if (idv->hivStatus == HIVStatus::Positive &&
				idv->onART)
				hivPositiveART.Record(t, -1);

			SurveyDeath(idv, t, deathCause);

			return true;
		};

	return ef;
}

void TBABM::SurveyDeath(Pointer<Individual> idv, int t, DeathCause deathCause)
{
	string s = ",";

	string line = to_string(seed) + s
				+ to_string(t) + s
				+ Ihash(idv) + s
				+ age(idv, t) + s
				+ sex(idv) + s
				+ causeDeath(deathCause) + s
				+ HIV(idv) + s 
				+ HIV_date(idv) + s 
				+ ART(idv) + s 
				+ ART_date(idv) + s 
				+ CD4(idv, t, params["m_30"].Sample(rng)) + s
				+ ART_baseline_CD4(idv, params["m_30"].Sample(rng))
				+ "\n";

	*deathSurvey << line;

	return;
}

// - trajectory
// - time: in days
// - hash
// - age: in years
// - sex: male/female
// - cause: HIV/natural
// - HIV: true/false
// - HIV_date: in days
// - ART: true/false
// - ART_date: in days
// - CD4
// - baseline CD4 (CD4 at time of ART initiation)