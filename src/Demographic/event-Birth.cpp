#include "../../include/TBABM/TBABM.h"
#include <cassert>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EQ::SchedulerT;

// Algorithm S6: Birth
EventFunc TBABM::Birth(Pointer<Individual> mother, Pointer<Individual> father)
{
	EventFunc ef = 
		[this, mother, father](double t, SchedulerT scheduler) {

			// If mother is dead
			if (!mother || mother->dead)
				return true;

			mother->pregnant = false;

			// Decide properties of baby
			Sex sex = params["sex"].Sample(rng) ?
					    Sex::Male : Sex::Female;

			HouseholdPosition householdPosition = HouseholdPosition::Offspring;
			MarriageStatus marriageStatus = MarriageStatus::Single;

			IndividualInitData initData {tbInfections, tbConversions, tbRecoveries, \
				tbSusceptible, tbInfected, tbLatent, tbInfectious, \
				tbTreatmentBegin, tbTreatmentEnd, tbTreatmentDropout, \
				tbInTreatment, tbCompletedTreatment, tbDroppedTreatment};

			auto deathHandler = [this] (Pointer<Individual> idv, int t, DeathCause cause) -> void { 
				return Schedule(t, Death(idv, cause));
			};

			auto tbProgressionHandler = std::bind(&TBABM::TBProgressionHandler, 
												  this, 
												  std::placeholders::_1, 
												  std::placeholders::_2);

			auto GlobalTBHandler = [this] (int t) -> double {
				return (double)tbInfectious(t)/(double)populationSize(t);
			};

			auto HouseholdTBHandler = [this] (int t, int householdID) -> double {
				auto household = households.at(householdID);

				// assert(household);
				if (household)
					return household->TBPrevalence(t);
				else
					return 0;
			};


			// Construct baby
			auto baby = std::make_shared<Individual>(
				CreateIndividualSimContext(t, eq, rng, fileData, params),
				initData,
				CreateIndividualHandlers(deathHandler, 
									     tbProgressionHandler, 
										 GlobalTBHandler, 
										 HouseholdTBHandler),
				name_gen.getName(rng),
				mother->householdID, t, sex,
				Pointer<Individual>(), mother, father,
				vector<Pointer<Individual>>{}, householdPosition, marriageStatus);
				// std::make_shared<Params>(params), std::make_shared<map<string, DataFrameFile>>(fileData));

			// printf("[%d] Baby born: %ld::%lu\n", (int)t, mother->householdID, std::hash<Pointer<Individual>>()(baby));

			// Add baby to household and population
			mother->offspring.push_back(baby);
			if (father && !father->dead)
				father->offspring.push_back(baby);

			population.push_back(baby);
			
			ChangeHousehold(baby, mother->householdID, householdPosition);
			
			Schedule(t, ChangeAgeGroup(baby));

			populationSize.Record(t, +1);
			births.Record(t, +1);


			// Schedule the next birth
			auto yearsToNextBirth = fileData["timeToSubsequentBirths"].getValue(0,0,(t-mother->birthDate)/365,rng);
			auto daysToNextBirth = 365*yearsToNextBirth;

			Schedule(t + daysToNextBirth - 9*30, Pregnancy(mother, mother->spouse));

			return true;
		};

	return ef;
}