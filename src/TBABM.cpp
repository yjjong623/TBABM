#include "../include/TBABM/TBABM.h"
#include <Empirical.h>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using Sex = Individual::Sex;

using StatisticalDistributions::Exponential;

using namespace StatisticalDistributions;

void TBABM::PurgeReferencesToIndividual(Pointer<Individual> host,
									    Pointer<Individual> idv)
{
	// Return if host or individual do not exist
	if (!host || !idv)
		return;

	// Reset pointers for spouse, mother, father (rough equivalent of setting
	// pointers to NULL)
	if (host->spouse == idv)
		host->spouse.reset();
	if (host->mother == idv)
		host->mother.reset();
	if (host->father == idv)
		host->father.reset();

	// Erase 'idv' from offspring
	for (auto it = host->offspring.begin(); it != host->offspring.end(); it++)
		if ((*it) == idv) {
			host->offspring.erase(it);
			break;
		}

	// Erase 'idv' from list of people 'idv' has lived with before
	for (auto it = host->livedWithBefore.begin(); it != host->livedWithBefore.end(); it++)
		if ((*it) == idv) {
			it = host->livedWithBefore.erase(it);
			break;
		}
}

void TBABM::DeleteIndividual(Pointer<Individual> idv)
{
	for (size_t i = 0; i < idv->livedWithBefore.size(); i++)
		PurgeReferencesToIndividual(idv->livedWithBefore[i], idv);
}

// Algorithm S2: Create a population at simulation time 0
EventFunc TBABM::CreatePopulation(long size)
{
	EventFunc ef = 
		[this, size](double t, SchedulerT scheduler) {
			int popChange = 0;

			while (popChange < size) {
				long hid = nHouseholds++;
				Pointer<Household> hh = householdGen.GetHousehold(hid);
				households[hid] = hh;

				// Insert all members of the household into the population
				population.insert(hh->head); popChange++;
				if (hh->spouse) {
					popChange++;
					population.insert(hh->spouse);
				}
				for (auto it = hh->offspring.begin(); it != hh->offspring.end(); it++) {
					popChange++;
					population.insert(*it);
				}
				for (auto it = hh->offspring.begin(); it != hh->offspring.end(); it++) {
					popChange++;
					population.insert(*it);
				}
			}

			populationSize.Record(t, popChange);

			return true;
		};

	return ef;

	// vector<Pointer<Individual>> household = HouseholdGen.
}

// Algorithm S3: Match making
EventFunc TBABM::Matchmaking(void)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			for (auto it = maleSeeking.begin(); it != maleSeeking.end(); it++) {
				auto weights = std::vector<long double>{};

				int i = 0;
				double denominator = 0;

				for (auto it2 = femaleSeeking.begin(); i < 100 && it2 != femaleSeeking.end(); it++) {
					int maleAge = (t - (*it)->birthDate) / 365;
					int femaleAge = (t - (*it2)->birthDate) / 365;
					int ageDifference = std::abs(maleAge-femaleAge);

					long double weight = distributions["marriageAgeDifference"]->pdf(ageDifference);
					denominator += weight;
					weights.push_back(weight);

					i++;
				}

				for (size_t i = 0; i < weights.size(); i++)
					weights[i] /= denominator;

				auto wifeDist = Empirical(weights);
				int wifeIdx = wifeDist(rng.mt_);
				auto wife = femaleSeeking.begin();
				std::advance(wife, wifeIdx);
				femaleSeeking.erase(wife);

				Schedule(t, Marriage(*it, *wife));

			}

			return true;
		};

	return ef;
}

// Algorithm S4: Adding new residents
EventFunc TBABM::NewHouseholds(int num)
{
	EventFunc ef = 
		[this, num](double t, SchedulerT scheduler) {
			int popChange = 0; // Number of people created so far

			while (popChange < num) {
				long hid = nHouseholds++;
				Pointer<Household> hh = householdGen.GetHousehold(hid);
				households[hid] = hh;

				// Insert all members of the household into the population
				population.insert(hh->head); popChange++;
				if (hh->spouse){
					population.insert(hh->spouse);
					popChange++;
				}
				for (auto it = hh->offspring.begin(); it != hh->offspring.end(); it++) {
					popChange++;
					population.insert(*it);
				}
				for (auto it = hh->other.begin(); it != hh->other.end(); it++) {
					popChange++;
					population.insert(*it);
				}
			}

			populationSize.Record(t, popChange);

			return true;
		};

	return ef;
}

// Algorithm S5: Create a household
// DRAFT implemented
// Assumption: relationships between individuals in the household have already
//   been created
EventFunc TBABM::CreateHousehold(Pointer<Individual> head,
								 Pointer<Individual> spouse,
								 std::unordered_set<Pointer<Individual>> offspring,
								 std::unordered_set<Pointer<Individual>> other)
{
	EventFunc ef = 
		[this, head, spouse, offspring, other](double t, SchedulerT scheduler) {
			auto household = std::make_shared<Household>(head, spouse, offspring, other);
			auto individuals = std::vector<Pointer<Individual>>{};
			long hid = nHouseholds++;

			// Head
			head->householdID = hid;
			individuals.push_back(head);

			// Spouse
			if (spouse) {
				spouse->householdID = hid;
				individuals.push_back(spouse);
			}

			// Offspring
			for (auto it = offspring.begin(); it != offspring.end(); it++) {
				(*it)->householdID = hid;
				individuals.push_back(*it);
			}

			// Other
			for (auto it = other.begin(); it != other.end(); it++) {
				(*it)->householdID = hid;
				individuals.push_back(*it);
			}

			// Update .livedWithBefore for everybody
			for (size_t i = 0; i < individuals.size(); i++)
				for (size_t j = 0; j < individuals.size(); j++) {
					if (individuals[i] != individuals[j])
						individuals[i]->livedWithBefore.push_back(individuals[j]);
				}

			households[hid] = household;
			return true;
		};

	return ef;
}

// Algorithm S6: Birth
// DRAFT implemented
EventFunc TBABM::Birth(int motherHID, Pointer<Individual> mother, 
									  Pointer<Individual> father)
{
	EventFunc ef = 
		[this, motherHID, mother, father](double t, SchedulerT scheduler) {

			// Decide properties of baby
			Sex sex = (*distributions["sex"])(rng.mt_) ?
					    Sex::Male : Sex::Female; // CHECK to ensure parity!

			HouseholdPosition householdPosition = HouseholdPosition::Offspring;
			MarriageStatus marriageStatus = MarriageStatus::Single;

			// Construct baby
			auto baby = std::make_shared<Individual>(
				motherHID, t, sex,
				Pointer<Individual>(), mother, father,
				std::vector<Pointer<Individual>>{}, householdPosition, marriageStatus);

			auto household = households[motherHID];

			// Add baby to household and population
			household->offspring.insert(baby);
			population.insert(baby);

			// Update .livedWithBefore
			baby->livedWithBefore.push_back(mother);
			baby->livedWithBefore.push_back(father);
			mother->livedWithBefore.push_back(baby); // bidirectional
			father->livedWithBefore.push_back(baby);

			// Update .livedWithBefore w.r.t. 'offspring' and 'other'
			for (auto it = household->offspring.begin(); it != household->offspring.end(); it++)
				if (*it != baby && *it) {
					baby->livedWithBefore.push_back(*it);
					(*it)->livedWithBefore.push_back(baby);
				}

			for (auto it = household->other.begin(); it != household->other.end(); it++) {
				if (!*it) // if dead 'other's
					continue;
				baby->livedWithBefore.push_back(*it);
				(*it)->livedWithBefore.push_back(baby);
			}

			return true;
		};

	return ef;
}

// Algorithm S7: Joining a household
// DRAFT implemented
EventFunc TBABM::JoinHousehold(Pointer<Individual> idv, long hid)
{
	EventFunc ef = 
		[this, idv, hid](double t, SchedulerT scheduler) {
			if (hid >= nHouseholds)
				return false;

			auto household = households[hid];

			// Update .livedWithBefore bidirectionally
			household->head->livedWithBefore.push_back(idv);
			idv->livedWithBefore.push_back(household->head);

			if (household->spouse) {
				household->spouse->livedWithBefore.push_back(idv);
				idv->livedWithBefore.push_back(household->spouse);
			}

			for (auto it = household->offspring.begin(); it != household->offspring.end(); it++) {
				if (!*it)
					continue;
				(*it)->livedWithBefore.push_back(idv);
				idv->livedWithBefore.push_back(*it);
			}

			for (auto it = household->other.begin(); it != household->other.end(); it++) {
				if (!*it)
					continue;
				(*it)->livedWithBefore.push_back(idv);
				idv->livedWithBefore.push_back(*it);
			}

			household->other.insert(idv);
			idv->householdID = hid;

			return true;
		};

	return ef;
}

// Algorithm S8: Create an individual
// 	Defined in header (because of template)

// Algorithm S9: Change of age groups
EventFunc TBABM::ChangeAgeGroup(Pointer<Individual> idv)
{
	int ageGroupWidth = 5; // Age groups go 0-5, 5-10, etc.

	auto concoctConstantName = [idv, ageGroupWidth] (int t, std::string prefix) {
		int age = ((double)t - idv->birthDate)/365.;

		std::string constantName = prefix;
		(constantName += std::string("-")) += std::to_string(age/ageGroupWidth);
		(constantName += std::string("-")) += idv->sex == Sex::Male ? std::string("M") : std::string("F");

		return constantName;
	};

	auto birthDateToAge = [] (int birthDate, int currentT) {
		return (currentT-birthDate)/365;
	};

	EventFunc ef = 
		[this, idv, ageGroupWidth, concoctConstantName, birthDateToAge](double t, SchedulerT scheduler) {
			double timeToNextEvent = ageGroupWidth*365;

			////////////////////////////////////////////////////////
			/// Natural death
			////////////////////////////////////////////////////////
			double deathRate = constants[concoctConstantName(t, "naturalDeath")];
			double timeToDeath = Exponential(deathRate)(rng.mt_);

			if (timeToDeath < timeToNextEvent)
				Schedule(t + timeToDeath, Death(idv));


			////////////////////////////////////////////////////////
			/// Leaving the current household to form a new household
			////////////////////////////////////////////////////////
			int age = birthDateToAge(idv->birthDate, t);
			bool idvIsHead = idv->householdPosition == HouseholdPosition::Head;
			double timeToLeave = (*distributions["leavingHousehold"])(rng.mt_);
			if (!idv->spouse && age >= 18 && age <= 55 && !idvIsHead
				&& timeToLeave < timeToNextEvent)
				Schedule(t + timeToLeave, LeaveHousehold(idv));


			////////////////////////////////////////////////////////
			/// Change of marital status from single to looking
			////////////////////////////////////////////////////////
			double timeToLook = (*distributions["timeToLooking"])(rng.mt_);
			if ((idv->marriageStatus == MarriageStatus::Single ||
				 idv->marriageStatus == MarriageStatus::Divorced)
				&& timeToLook < timeToNextEvent)
				Schedule(t + timeToLook, SingleToLooking(idv));


			////////////////////////////////////////////////////////
			/// Joining a household
			////////////////////////////////////////////////////////
			auto household = households[idv->householdID];
			int householdSize = household->offspring.size() + household->other.size() +
								(household->head ? 1:0) + (household->spouse ? 1:0);
			if (age >= 65 && householdSize == 1) {
				for (size_t i = 0; i < idv->livedWithBefore.size(); i++) {
					if (!idv->livedWithBefore[i])
						continue;
					idv->householdPosition = HouseholdPosition::Other;
					int hid = idv->livedWithBefore[i]->householdID;
					Schedule(t, JoinHousehold(idv, hid));
				}
			}

			Schedule(t + timeToNextEvent, ChangeAgeGroup(idv));

			return true;
		};

	return ef;
}

// Algorithm S10: Natural death
EventFunc TBABM::Death(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			if (idv->sex == Sex::Male)
				maleSeeking.erase(idv);
			else
				femaleSeeking.erase(idv);

			auto household = households[idv->householdID];
			switch (idv->householdPosition) {
				case (HouseholdPosition::Head):
					household->head.reset();
					break;
				case (HouseholdPosition::Spouse):
					household->spouse.reset();
					break;
				case (HouseholdPosition::Offspring):
					for (auto it = household->offspring.begin(); it != household->offspring.end(); it++)
						if (*it == idv) {
							household->offspring.erase(it);
							break;
						}
					break;
				case (HouseholdPosition::Other):
					for (auto it = household->other.begin(); it != household->other.end(); it++)
						if (*it == idv) {
							household->other.erase(it);
							break;
						}
					break;
			}

			DeleteIndividual(idv);

			if (household->spouse) {
				household->head = household->spouse;
				household->head->marriageStatus = MarriageStatus::Single;
				household->spouse.reset();
			} else if (household->other.size() > 0 || household->offspring.size() > 0) {
				// Identify oldest 'other' in household
				int earliestBirthDateOther = std::numeric_limits<int>::max();
				auto oldestOther = Pointer<Individual>();
				for (auto it = household->other.begin(); it != household->other.end(); it++) {
					if ((*it)->birthDate < earliestBirthDateOther) {
						earliestBirthDateOther = (*it)->birthDate;
						oldestOther = *it;
					}
				}

				// Identify oldest 'offspring' in household
				int earliestBirthDateOffspring = std::numeric_limits<int>::max();
				auto oldestOffspring = Pointer<Individual>();
				for (auto it = household->offspring.begin(); it != household->offspring.end(); it++) {
					if ((*it)->birthDate < earliestBirthDateOffspring) {
						earliestBirthDateOffspring = (*it)->birthDate;
						oldestOffspring = *it;
					}
				}

				// Assign new head
				if (earliestBirthDateOther < earliestBirthDateOffspring) { // Oldest person is an 'other'
					household->head = oldestOther;
					household->other.erase(oldestOther);
				} else { // Oldest person is an offspring
					household->head = oldestOffspring;
					household->offspring.erase(oldestOffspring);
				}
			} else { // Household must be empty because there is no spouse, others, offspring
				households[idv->householdID].reset();
			}

			return true;
		};

	return ef;
}

// Algorithm S11: Leave current household to form new household
EventFunc TBABM::LeaveHousehold(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			households[idv->householdID]->RemoveIndividual(idv);

			idv->householdPosition = HouseholdPosition::Head;

			Schedule(t, CreateHousehold(idv, Pointer<Individual>(), {}, {}));
			return true;
		};

	return ef;
}

// Algorithm S12: Change of marital status from single to looking
EventFunc TBABM::SingleToLooking(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			if (idv->sex == Sex::Male)
				maleSeeking.insert(idv);
			else
				femaleSeeking.insert(idv);

			idv->marriageStatus = MarriageStatus::Looking;

			return true;
		};

	return ef;
}

// Algorithm S13: Marriage
// UNIMPLEMENTED: Updating 'livedWithBefore' property
EventFunc TBABM::Marriage(Pointer<Individual> m, Pointer<Individual> f)
{
	EventFunc ef = 
		[this, m, f](double t, SchedulerT scheduler) {

			if (households[m->householdID]->size() == 1) {
				// The female will join the male's household
				auto household = households[m->householdID];
				household->spouse = f;

				// Male's relationship is head
				m->householdPosition = HouseholdPosition::Head;

				// Female is spouse
				f->householdPosition = HouseholdPosition::Spouse;
			} else if (households[f->householdID]->size() == 1) {
				// Male joins female household
				auto household = households[f->householdID];
				household->spouse = m;

				// Male is spouse
				m->householdPosition = HouseholdPosition::Spouse;
				// Female is head
				f->householdPosition = HouseholdPosition::Head;
			} else {
				// Couple forms new household?
				if ((*distributions["coupleFormsNewHousehold"])(rng.mt_) == 1) {
					Schedule(t, CreateHousehold(m, f, {}, {}));
				} else {
					auto household = households[m->householdID];
					household->spouse = f;
					f->householdPosition = HouseholdPosition::Spouse;
				}
			}

			m->spouse = f;
			f->spouse = m;

			// Time to divorce
			double yearsToDivorce = (*distributions["marriageDuration"])(rng.mt_);
			int daysToDivorce = 365 * yearsToDivorce;
			Schedule(t + daysToDivorce, Divorce(m, f));

			// Time to first birth
			double yearsToFirstBirth = (*distributions["timeToBirth"])(rng.mt_);
			int daysToFirstBirth = 365 * yearsToFirstBirth;
			Schedule(t + daysToFirstBirth, Birth(m->householdID, m, f));

			return true;
		};

	return ef;
}

// Algorithm S14: Divorce
EventFunc TBABM::Divorce(Pointer<Individual> m, Pointer<Individual> f)
{
	EventFunc ef = 
		[this, m, f](double t, SchedulerT scheduler) {
			// Change marriage status to divorced
			m->marriageStatus = MarriageStatus::Divorced;
			f->marriageStatus = MarriageStatus::Divorced;

			// Select who is to leave the household
			auto booted = (m->householdPosition == HouseholdPosition::Head) ? f : m;

			// Identify a new household for whoever left
			int newHouseholdID = -1;
			for (size_t i = 0; i < booted->offspring.size(); i++)
				if (booted->offspring[i]->householdID != m->householdID)
					newHouseholdID = booted->offspring[i]->householdID;

			if (booted->mother && booted->mother->householdID != m->householdID)
				newHouseholdID = booted->mother->householdID;

			if (booted->father && booted->father->householdID != m->householdID)
				newHouseholdID = booted->father->householdID;

			if (newHouseholdID > -1) {
				// If another household was found for the booted individual
				booted->householdPosition = HouseholdPosition::Other;
				Schedule(t, JoinHousehold(booted, newHouseholdID));
			} else {
				booted->householdPosition = HouseholdPosition::Head;
				Schedule(t, CreateHousehold(booted, Pointer<Individual>(), {}, {}));
			}

			return true;
		};

	return ef;
}
