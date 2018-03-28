#include "../include/TBABM/TBABM.h"
#include <Empirical.h>
#include <cassert>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using Sex = Individual::Sex;

using StatisticalDistributions::Exponential;

using namespace StatisticalDistributions;

template <>
const PrevalenceTimeSeries<int> *TBABM::GetData<PrevalenceTimeSeries<int>>(TBABMData field)
{
    switch(field) {
        case TBABMData::Marriages:   return nullptr;
        case TBABMData::Births:      return nullptr;
        case TBABMData::Deaths:		 return nullptr;
        case TBABMData::PopulationSize: return &populationSize;
        case TBABMData::Divorces:    return nullptr;
        default:                   return nullptr;
    }
}

template <>
const IncidenceTimeSeries<int> *TBABM::GetData<IncidenceTimeSeries<int>>(TBABMData field)
{
    switch(field) {
        case TBABMData::Marriages:   return &marriages;
        case TBABMData::Births:      return &births;
        case TBABMData::Deaths:		 return &deaths;
        case TBABMData::PopulationSize: return nullptr;
        case TBABMData::Divorces:    return nullptr;
        default:                   return nullptr;
    }
}

bool TBABM::Run(void)
{
	Schedule(0, CreatePopulation(1000));
	Schedule(365, Matchmaking());

	while (!eq.Empty()) {
		auto e = eq.Top();

		if (e->t > constants["tMax"])
			break;

		printf("\t\t\t\tBEEP\n");
		e->run();
		printf("\t\t\t\tBOOP\n");
		// delete e;
		eq.Pop();
	}
	printf("Simulation finished!\n");

	births.Close();
	deaths.Close();
	marriages.Close();
	populationSize.Close();

	return true;
}

void TBABM::Schedule(int t, EventQueue<>::EventFunc ef)
{
	auto f = eq.MakeScheduledEvent(t, ef);
	eq.Schedule(f);
	return;
}

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

void TBABM::ChangeHousehold(Pointer<Individual> idv, int newHID, HouseholdPosition newRole)
{
	printf("Changing household of %ld::%lu\n\n", idv->householdID, std::hash<Pointer<Individual>>()(idv));
	int oldHID = idv->householdID;

	auto oldHousehold = households[oldHID];
	auto newHousehold = households[newHID];

	assert(idv);
	assert(households[idv->householdID]);
	assert(households[newHID]);

	oldHousehold->RemoveIndividual(idv);

	// Clear household if there's nobody left in it
	if (oldHousehold->size() == 0)
		households[oldHID].reset();

	idv->householdPosition = newRole;
	idv->householdID = newHID;

	// Update livedWithBefore
	if (newHousehold->head) {
		newHousehold->head->livedWithBefore.push_back(idv);
		idv->livedWithBefore.push_back(newHousehold->head);
	}
	if (newHousehold->spouse) {
		newHousehold->spouse->livedWithBefore.push_back(idv);
		idv->livedWithBefore.push_back(newHousehold->spouse);
	}
	for (auto it = newHousehold->offspring.begin(); it != newHousehold->offspring.end(); it++) {
		if (!*it)
			continue;
		(*it)->livedWithBefore.push_back(idv);
		idv->livedWithBefore.push_back(*it);
	}
	for (auto it = newHousehold->other.begin(); it != newHousehold->other.end(); it++) {
		if (!*it)
			continue;
		(*it)->livedWithBefore.push_back(idv);
		idv->livedWithBefore.push_back(*it);
	}

	// Insert individual
	if (newRole == HouseholdPosition::Head) {
		if (newHousehold->head) {
			newHousehold->head->householdPosition = HouseholdPosition::Other;
			newHousehold->other.insert(newHousehold->head);
		}
		newHousehold->head = idv;
	}
	else if (newRole == HouseholdPosition::Spouse) {
		if (newHousehold->spouse) {
			newHousehold->other.insert(newHousehold->spouse);
			newHousehold->spouse->householdPosition = HouseholdPosition::Other;
		}
		newHousehold->spouse = idv;
	}
	else if (newRole == HouseholdPosition::Other) {
		newHousehold->other.insert(idv);
	}
	else if (newRole == HouseholdPosition::Offspring) {
		newHousehold->offspring.insert(idv);
	}

	return;
}

// Algorithm S2: Create a population at simulation time 0
EventFunc TBABM::CreatePopulation(long size)
{
	EventFunc ef = 
		[this, size](double t, SchedulerT scheduler) {
			printf("[%d] CreatePopulation\n", (int)t);
			int popChange = 0;

			while (popChange < size) {
				long hid = nHouseholds++;
				Pointer<Household> hh = householdGen.GetHousehold(hid);
				households[hid] = hh;

				assert(hh->head);

				// Insert all members of the household into the population
				population.insert(hh->head); popChange++;
				assert(hh->head->householdID == hid);
				Schedule(t, ChangeAgeGroup(hh->head));
				if (hh->spouse) {
					popChange++;
					population.insert(hh->spouse);
					assert(hh->spouse->householdID == hid);
					Schedule(t, ChangeAgeGroup(hh->spouse));
				}
				for (auto it = hh->offspring.begin(); it != hh->offspring.end(); it++) {
					popChange++;
					population.insert(*it);
					assert((*it)->householdID == hid);
					Schedule(t, ChangeAgeGroup(*it));
				}
				for (auto it = hh->other.begin(); it != hh->other.end(); it++) {
					popChange++;
					population.insert(*it);
					assert((*it)->householdID == hid);
					Schedule(t, ChangeAgeGroup(*it));
				}
			}

			populationSize.Record(t, popChange);
			printf("\tpopSize=%lu\n", population.size());
			printf("\tpopChange=%d\n", popChange);

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
			printf("[%d]\tMatchmaking, ms=%lu, fs=%lu\n", (int)t, maleSeeking.size(), femaleSeeking.size());
			auto it = maleSeeking.begin();
			for (; it != maleSeeking.end(); it++) {
				printf("\t...New Man\n");
				if (femaleSeeking.size() == 0) {
					break;
				}

				auto weights = std::vector<long double>{};

				int j = 0;
				double denominator = 0;

				for (auto it2 = femaleSeeking.begin(); j < 100 && it2 != femaleSeeking.end(); it2++) {
					assert(*it2);
					assert(*it);
					int maleAge = (t - (*it)->birthDate) / 365;
					int femaleAge = (t - (*it2)->birthDate) / 365;
					int ageDifference = std::abs(maleAge-femaleAge);

					long double weight = distributions["marriageAgeDifference"]->pdf(ageDifference);
					denominator += weight;
					weights.push_back(weight);

					j++;
				}

				printf("\t...evaluated each female\n");

				for (size_t i = 0; i < weights.size(); i++)
					weights[i] = weights[i] / denominator;

				auto wifeDist = Empirical(weights);
				int wifeIdx = wifeDist(rng.mt_);
				auto wife = femaleSeeking.begin();


				printf("\t...selected wife\n");

				assert(wifeIdx < femaleSeeking.size());

				std::advance(wife, wifeIdx);
				assert(*wife);

				printf("\t...advanced pointer\n");

				printf("\tScheduling a Marriage\n");
				printf("\t\tM %2d F %2d\n", (int)(t-(*it)->birthDate)/365, (int)(t-(*wife)->birthDate)/365);
				Schedule(t, Marriage(*it, *wife));
				printf("\t...scheduled marriage\n");

				femaleSeeking.erase(wife);
				printf("\t...erased female from seeking\n");
			}

			// Erase males who have been matched to a female
			maleSeeking.erase(maleSeeking.begin(), it);

			printf("\t...exited main loop\n");
			Schedule(t + 30, Matchmaking());

			printf("\t...Matchmaking successful\n");
			return true;
		};


	return ef;
}

// Algorithm S4: Adding new residents
EventFunc TBABM::NewHouseholds(int num)
{
	EventFunc ef = 
		[this, num](double t, SchedulerT scheduler) {
			printf("[%d] NewHouseholds\n", (int)t);
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
			printf("[%d] CreateHousehold\n", (int)t);
			auto household = std::make_shared<Household>(head, spouse, offspring, other);
			auto individuals = std::vector<Pointer<Individual>>{};
			long hid = nHouseholds++;

			// Head
			assert(head);
			head->householdID = hid;
			head->householdPosition = HouseholdPosition::Head;
			individuals.push_back(head);

			// Spouse
			if (spouse) {
				spouse->householdPosition = HouseholdPosition::Spouse;
				spouse->householdID = hid;
				individuals.push_back(spouse);
			}

			// Offspring
			for (auto it = offspring.begin(); it != offspring.end(); it++) {
				(*it)->householdPosition = HouseholdPosition::Offspring;
				(*it)->householdID = hid;
				individuals.push_back(*it);
			}

			// Other
			for (auto it = other.begin(); it != other.end(); it++) {
				(*it)->householdPosition = HouseholdPosition::Offspring;
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

			// If mother is dead
			if (!mother)
				return true;


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

			printf("[%d] Baby born: %d::%lu\n", (int)t, motherHID, std::hash<Pointer<Individual>>()(baby));

			auto household = households[motherHID];

			assert(household);

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
			
			Schedule(t + 365*5, ChangeAgeGroup(baby));
			populationSize.Record(t, +1);
			births.Record(t, +1);

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
			printf("[%d] JoinHousehold, populationSize=%lu\n, individual: %ld::%lu, newHID: %ld\n", (int)t, population.size(), idv->householdID, std::hash<Pointer<Individual>>()(idv), hid);
			if (hid >= nHouseholds)
				return false;


			auto household = households.at(hid);
			assert(household);

			household->PrintHousehold(t);

			// Update .livedWithBefore bidirectionally
			assert(household->head);
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
			idv->householdPosition = HouseholdPosition::Other;
			idv->householdID = hid;

			printf("\t...Inserted individual into household\n");

			return true;
		};

	return ef;
}

// Algorithm S8: Create an individual
// 	Defined in header (because of template)

// Algorithm S9: Change of age groups
EventFunc TBABM::ChangeAgeGroup(Pointer<Individual> idv)
{
	int ageGroupWidth = constants["ageGroupWidth"]; // Age groups go 0-5, 5-10, etc.

	auto concoctConstantName = [idv, ageGroupWidth] (int t, std::string prefix) {
		int age = ((double)t - idv->birthDate)/365.;

		std::string constantName = prefix;
		(constantName += std::string("-")) += std::to_string((age/ageGroupWidth) * ageGroupWidth);
		(constantName += std::string("-")) += idv->sex == Sex::Male ? std::string("M") : std::string("F");

		return constantName;
	};

	auto birthDateToAge = [] (int birthDate, int currentT) {
		return (currentT-birthDate)/365;
	};

	EventFunc ef = 
		[this, idv, ageGroupWidth, concoctConstantName, birthDateToAge](double t, SchedulerT scheduler) {
			if (!idv)
				return true;

			printf("[%d] ChangeAgeGroup: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));
			double timeToNextEvent = ageGroupWidth*365;

			////////////////////////////////////////////////////////
			/// Natural death
			////////////////////////////////////////////////////////
			double deathRate = constants[concoctConstantName(birthDateToAge(idv->birthDate, t), "naturalDeath")];
			double timeToDeath = 365 * Exponential(deathRate)(rng.mt_);

			bool scheduledDeath = false;
			if (timeToDeath < timeToNextEvent) {
				printf("\tScheduling death\n");
				scheduledDeath = true;
				Schedule(t + timeToDeath, Death(idv));
			}


			////////////////////////////////////////////////////////
			/// Leaving the current household to form a new household
			////////////////////////////////////////////////////////
			int age = birthDateToAge(idv->birthDate, t);
			bool idvIsHead = idv->householdPosition == HouseholdPosition::Head;
			double timeToLeave = (*distributions["leavingHousehold"])(rng.mt_);
			if (!idv->spouse && 
				age >= 18 && 
				age <= 55 && 
				!idvIsHead && 
				timeToLeave < timeToNextEvent &&
				timeToLeave < timeToDeath) {
				printf("\tScheduling leaving the household\n");
				Schedule(t + timeToLeave, LeaveHousehold(idv));
			}


			////////////////////////////////////////////////////////
			/// Change of marital status from single to looking
			////////////////////////////////////////////////////////
			double timeToLook = (*distributions["timeToLooking"])(rng.mt_);
			if ((idv->marriageStatus == MarriageStatus::Single ||
				 idv->marriageStatus == MarriageStatus::Divorced)
				&& timeToLook < timeToNextEvent
				&& timeToLook < timeToDeath) {
				printf("\tScheduling SingleToLooking\n");
				Schedule(t + timeToLook, SingleToLooking(idv));
			}


			////////////////////////////////////////////////////////
			/// Joining a household
			////////////////////////////////////////////////////////
			auto household = households[idv->householdID];
			if (age >= 65 && household->size() == 1) {
				for (size_t i = 0; i < idv->livedWithBefore.size(); i++) {
					if (!idv->livedWithBefore[i])
						continue;
					int hid = idv->livedWithBefore[i]->householdID;
					if (!households[hid])
						continue;
					ChangeHousehold(idv, hid, HouseholdPosition::Other);
					break;
				}
			}

			if (!scheduledDeath)
				Schedule(t + timeToNextEvent, ChangeAgeGroup(idv));

			printf("\t...ChangeAgeGroup successful\n");

			return true;
		};

	return ef;
}

// Algorithm S10: Natural death
EventFunc TBABM::Death(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			printf("[%d] Death: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));
			if (idv->sex == Sex::Male)
				maleSeeking.erase(idv);
			else
				femaleSeeking.erase(idv);

			auto household = households[idv->householdID];

			DeleteIndividual(idv);

			household->RemoveIndividual(idv);
			if (household->size() == 0)
				household.reset();

			population.erase(idv);

			printf("\t...population size=%lu\n", population.size());
			populationSize.Record(t, -1);
			deaths.Record(t, +1);
			printf("\t...death successful\n");

			return true;
		};

	return ef;
}

// Algorithm S11: Leave current household to form new household
EventFunc TBABM::LeaveHousehold(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			printf("[%d] LeaveHousehold: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));
			if (!idv)
				return true;
			
			households[idv->householdID]->RemoveIndividual(idv);

			if (households[idv->householdID]->size() == 0)
				households[idv->householdID].reset();

			idv->householdPosition = HouseholdPosition::Head;
			
			auto household = std::make_shared<Household>(idv, Pointer<Individual>(), std::unordered_set<Pointer<Individual>>{}, std::unordered_set<Pointer<Individual>>{});
			households[nHouseholds++] = household;
			idv->householdID = nHouseholds - 1;

			printf("\t...LeaveHousehold successful\n");
			return true;
		};

	return ef;
}

// Algorithm S12: Change of marital status from single to looking
EventFunc TBABM::SingleToLooking(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this, idv](double t, SchedulerT scheduler) {
			printf("[%d] SingleToLooking: %ld::%lu\n", (int)t, idv->householdID, std::hash<Pointer<Individual>>()(idv));
			if (idv->sex == Sex::Male)
				maleSeeking.insert(idv);
			else
				femaleSeeking.insert(idv);

			idv->marriageStatus = MarriageStatus::Looking;

			printf("\t...SingleToLooking successful\n");

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

			if (population.count(m) != 1 || population.count(f) != 1) {
				printf("\tA spouse is dead!\n");
				return true;
			}

			printf("[%d] Marriage, populationSize=%lu, people: m=%ld::%lu f=%ld::%lu\n", (int)t, population.size(), m->householdID, std::hash<Pointer<Individual>>()(m), f->householdID, std::hash<Pointer<Individual>>()(f));
			assert(m);
			assert(f);
			assert(m != f);

			if (households[m->householdID]->size() == 1) {
				printf("\t...Female joins male\n");
				// The female will join the male's household
				ChangeHousehold(f, m->householdID, HouseholdPosition::Spouse);
			} else if (households[f->householdID]->size() == 1) {
				printf("\t...Male joins female\n");
				// Male joins female household
				ChangeHousehold(m, f->householdID, HouseholdPosition::Spouse);
			} else {
				// Couple forms new household?
				if ((*distributions["coupleFormsNewHousehold"])(rng.mt_) == 1) {
					printf("\t...Couple forms new household\n");
					auto hid = nHouseholds++;
					households[hid] = std::make_shared<Household>();

					ChangeHousehold(m, hid, HouseholdPosition::Head);
					ChangeHousehold(f, hid, HouseholdPosition::Spouse);
				} else {
					printf("\t...Female joins plural male household\n");
					ChangeHousehold(f, m->householdID, HouseholdPosition::Spouse);
				}
			}

			m->spouse = f;
			f->spouse = m;

			// DIVORCE IS DISABLED RN
			// Time to divorce
			double yearsToDivorce = (*distributions["marriageDuration"])(rng.mt_);
			int daysToDivorce = 365 * yearsToDivorce;
			Schedule(t + daysToDivorce, Divorce(m, f));

			// Time to first birth
			double yearsToFirstBirth = (*distributions["timeToBirth"])(rng.mt_);
			int daysToFirstBirth = 365 * yearsToFirstBirth;
			Schedule(t + daysToFirstBirth, Birth(m->householdID, m, f));

			marriages.Record(t, +1);

			printf("\t...Finished the marriage\n");
			return true;
		};

	return ef;
}

// Algorithm S14: Divorce
EventFunc TBABM::Divorce(Pointer<Individual> m, Pointer<Individual> f)
{
	EventFunc ef = 
		[this, m, f](double t, SchedulerT scheduler) {
			printf("[%d] Divorce, populationSize=%lu, people: m=%ld::%lu f=%ld::%lu\n", (int)t, population.size(), m->householdID, std::hash<Pointer<Individual>>()(m), f->householdID, std::hash<Pointer<Individual>>()(f));

			// If someone is dead they can't divorce
			if (!population.count(m) || !population.count(f))
				return true;

			// Change marriage status to divorced
			m->marriageStatus = MarriageStatus::Divorced;
			f->marriageStatus = MarriageStatus::Divorced;

			printf("DIVORCE STILL ALIVE! -1 \n");

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

			printf("DIVORCE STILL ALIVE! -2 \n");

			if (newHouseholdID > -1) {
				assert(households[newHouseholdID]);

				// If another household was found for the booted individual
				ChangeHousehold(booted, newHouseholdID, HouseholdPosition::Other);
			} else {
				printf("DIVORCE STILL ALIVE! -4 \n");
				Schedule(t, CreateHousehold(booted, Pointer<Individual>(), {}, {}));
			}

			printf("\t...divorce successful\n");

			return true;
		};

	return ef;
}
