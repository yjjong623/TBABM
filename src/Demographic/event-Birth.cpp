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
			if (!mother.use_count())
				return true;
			if (mother->dead)
				return true;

			mother->pregnant = false;

			// Decide properties of baby
			Sex sex = params["sex"].Sample(rng) ?
					    Sex::Male : Sex::Female;

			HouseholdPosition householdPosition = HouseholdPosition::Offspring;
			MarriageStatus marriageStatus = MarriageStatus::Single;

			IndividualInitData initData {tbInfections, tbConversions, tbRecoveries, \
				tbInfectionsHousehold, tbInfectionsCommunity,
				tbSusceptible, tbInfected, tbLatent, tbInfectious, \
				tbTreatmentBegin, tbTreatmentBeginHIV, tbTreatmentEnd, tbTreatmentDropout, \
				tbInTreatment, tbCompletedTreatment, tbDroppedTreatment, activeHouseholdContacts};

			auto deathHandler = [this] (Pointer<Individual> idv, int t, DeathCause cause) -> void { 
				return Schedule(t, Death(idv, cause));
			};

			auto GlobalTBHandler = [this] (int t) -> double {
				return (double)tbInfectious(t)/(double)populationSize(t);
			};

			// Construct baby
			auto baby = makeIndividual(
				CreateIndividualSimContext(t, eq, rng, fileData, params),
				initData,
				CreateIndividualHandlers(deathHandler, GlobalTBHandler),
				name_gen.getName(rng),
				mother->householdID, t, sex,
				Pointer<Individual>(), mother, father,
				vector<Pointer<Individual>>{}, householdPosition, marriageStatus);

			// printf("[%d] Baby born: %ld::%lu\n", (int)t, mother->householdID, std::hash<Pointer<Individual>>()(baby));

			// Add baby to household and population
			mother->offspring.push_back(baby);
			if (father && !father->dead)
				father->offspring.push_back(baby);

			population.push_back(baby);
			
			ChangeHousehold(baby, t, mother->householdID, householdPosition);
			
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