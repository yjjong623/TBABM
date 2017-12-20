#pragma once

#include <map>
#include <string>

#include <StatisticalDistribution.h>
#include <RNG.h>
#include <EventQueue.h>
#include <PrevalenceTimeSeries.h>

#include "Individual.h"
#include "Household.h"

using std::map;
using std::unordered_set;
using std::vector;
using std::string;

using namespace StatisticalDistributions;
using namespace SimulationLib;

template <typename T>
using Pointer = std::shared_ptr<T>;

class TBABM {
public:
	using Distributions = map<string, Pointer<StatisticalDistribution<long double>>>;
	using Constants = map<string, int>;

	using EQ = EventQueue<double, bool>;
	using EventFunc = EQ::EventFunc;
	using SchedulerT = EQ::SchedulerT;

	enum class TBABMData {
		// HIVNegative, 
		// HIVPositiveNIC, 
		// HIVPositiveIC, 
		// HIVPositiveART, 
		// HIVPositive,
		
		// TBSusceptible,
		// TBLatentTN, 
		// TBLatentTC, 
		// TBLatentTCIPT, 
		// TBLatentTI, 
		// TBLatent,
		
		// TBInfectiveTC, 
		// TBInfectiveTN, 
		// TBInfectiveTI, 
		// TBInfective,

		Marriages,
		Divorces,
		Births,
		Deaths
	};

	TBABM(TBABM::Distributions distributions, TBABM::Constants constants) : 
		distributions(distributions),
		constants(constants),

		births("births", constants["tMax"], constants["periodLength"]),
		deaths("deaths", constants["tMax"], constants["periodLength"]),
		populationSize("populationSize", constants["tMax"], constants["periodLength"]),

		population({}),
		households({}),

		maleSeeking({}),
		femaleSeeking({}),

		nHouseholds(0),
		rng(0) {}

	bool Run(void);

	template <typename T>
	const T& GetData(TBABMData field);

private:

	////////////////////////////////////////////////////////
	/// Events
	////////////////////////////////////////////////////////

	// Algorithm S2: Create a population at simulation time 0
	EventFunc CreatePopulation(long size);

	// Algorithm S3: Match making
	EventFunc Matchmaking(void);

	// Algorithm S4: Adding new residents
	EventFunc NewHouseholds(int num);

	// Algorithm S5: Create a household
	EventFunc CreateHousehold(vector<Pointer<Individual>> idvs);

	// Algorithm S6: Birth
	EventFunc Birth(void);

	// Algorithm S7: Joining a household
	EventFunc JoinHousehold(Pointer<Individual>, long hid);

	// Algorithm S8: Create an individual
	// DRAFT implemented
	template <typename... Args>
	EventFunc CreateIndividual(Args... args)
	{
		EventFunc ef = 
			[this](double t, SchedulerT scheduler) {
				auto idv = std::make_shared<Individual>(std::forward<Args>(args)...);

				schedule(t, ChangeAgeGroup(idv));

				return true;
			};
	}

	// Algorithm S9: Change of age groups
	EventFunc ChangeAgeGroup(Pointer<Individual>);

	// Algorithm S10: Natural death
	EventFunc Death(Pointer<Individual>);

	// Algorithm S11: Leave current household to form new household
	EventFunc LeaveHousehold(Pointer<Individual>);

	// Algorithm S12: Change of marital status from single to looking
	EventFunc SingleToLooking(Pointer<Individual>);

	// Algorithm S13: Marriage
	EventFunc Marriage(Pointer<Individual> m, Pointer<Individual> f);

	// Algorithm S14: Divorce
	EventFunc Divorce(Pointer<Individual> m, Pointer<Individual> f);

	////////////////////////////////////////////////////////
	/// Data
	////////////////////////////////////////////////////////

	PrevalenceTimeSeries<int> births;
	PrevalenceTimeSeries<int> deaths;
	PrevalenceTimeSeries<int> populationSize;	

	unordered_set<Pointer<Individual>> population;
	map<long, Pointer<Household>> households;

	unordered_set<Pointer<Individual>> maleSeeking;
	unordered_set<Pointer<Individual>> femaleSeeking;

	Distributions distributions;
	Constants constants;

	long nHouseholds;

	RNG rng;
};