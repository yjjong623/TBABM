#include "../include/TBABM/TBABM.h"	

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

// Algorithm S2: Create a population at simulation time 0
EventFunc TBABM::CreatePopulation(long size)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			printf("Create population\n");
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
			auto itr = maleSeeking.begin();

			if (itr == maleSeeking.end()) 
				return true;

			for (auto f = femaleSeeking.begin(); f != femaleSeeking.end(); f++) {

			}
		};

	return ef;
}

// Algorithm S4: Adding new residents
EventFunc TBABM::NewHouseholds(int num)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			printf("NewHouseholds\n");
			return true;
		};

	return ef;
}

// Algorithm S5: Create a household
// DRAFT implemented
EventFunc TBABM::CreateHousehold(vector<Pointer<Individual>> idvs)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			households[nHouseholds++]->members = idvs;
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
		[this](double t, SchedulerT scheduler) {

			Sex sex = distributions["sex"]->operator(rng) ?
					    Sex::Male : Sex::Female;

			HouseholdPosition householdPosition = 
				sex == Sex::Male ? HouseholdPosition::Son :
								   HouseholdPosition::Daughter;

			auto baby = std::make_shared<Individual>(
				motherHID, 0, sex,
				Pointer<Individual>(), mother, father,
				{}, {}, householdPosition);

			households[motherHID]->members->insert(baby);
			population->insert(baby);

			return true;
		};

	return ef;
}

// Algorithm S7: Joining a household
// DRAFT implemented
EventFunc TBABM::JoinHousehold(Pointer<Individual> idv, long hid)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			if (hid >= nHouseholds)
				return false;

			households[hid]->members->insert(idv);

			return true;
		};

	return ef;
}

// Algorithm S8: Create an individual
// 	Defined in header (because of template)

// Algorithm S9: Change of age groups
EventFunc TBABM::ChangeAgeGroup(Pointer<Individual>)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			printf("ChangeAgeGroup\n");
			return true;
		};

	return ef;
}

// Algorithm S10: Natural death
EventFunc TBABM::Death(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			if (idv->sex == Sex::Male)
				maleSeeking.erase(idv);
			else
				femaleSeeking.erase(idv);

			auto hid = idv->householdID;
			households[hid]->members.erase(idv);

			return true;
		};

	return ef;
}

// Algorithm S11: Leave current household to form new household
EventFunc TBABM::LeaveHousehold(Pointer<Individual>)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			printf("LeaveHousehold\n");
			return true;
		};

	return ef;
}

// Algorithm S12: Change of marital status from single to looking
EventFunc TBABM::SingleToLooking(Pointer<Individual> idv)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			if (idv->Sex == Sex::Male)
				maleSeeking.insert(idv);
			else
				femaleSeeking.insert(idv);
			return true;
		};

	return ef;
}

// Algorithm S13: Marriage
EventFunc TBABM::Marriage(Pointer<Individual> m, Pointer<Individual> f)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			printf("Marriage\n");
			return true;
		};

	return ef;
}

// Algorithm S14: Divorce
EventFunc TBABM::Divorce(Pointer<Individual> m, Pointer<Individual> f)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {
			printf("Divorce\n");
			return true;
		};

	return ef;
}