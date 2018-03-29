#pragma once

#include <map>
#include <string>
#include <ctime>

#include <StatisticalDistribution.h>
#include <Exponential.h>
#include <RNG.h>
#include <EventQueue.h>
#include <PrevalenceTimeSeries.h>
#include <IncidenceTimeSeries.h>
#include <CSVExport.h>
#include <EventQueue.h>

#include "Individual.h"
#include "Household.h"
#include "HouseholdGen.h"


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
	using Constants = map<string, double>;

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
		Deaths,
		PopulationSize
	};

	TBABM(TBABM::Distributions distributions, 
		  TBABM::Constants constants,
		  const char *householdsFile, 
		  long seed) : 
		distributions(distributions),
		constants(constants),

		births("births", 0, constants["tMax"], constants["periodLength"]),
		deaths("deaths", 0, constants["tMax"], constants["periodLength"]),
		marriages("marriages", 0, constants["tMax"], constants["periodLength"]),
		divorces("divorces", 0, constants["tMax"], constants["periodLength"]),
		populationSize("populationSize", constants["tMax"], constants["periodLength"]),

		population({}),
		households({}),

		maleSeeking({}),
		femaleSeeking({}),

		householdGen(distributions, constants, householdsFile),

		nHouseholds(0),
		rng(seed) { printf("Finished initing\n"); };

	bool Run(void);

	template <typename T>
	T* GetData(TBABMData field);

private:
	IncidenceTimeSeries<int> births;
	IncidenceTimeSeries<int> deaths;
	IncidenceTimeSeries<int> marriages;
	IncidenceTimeSeries<int> divorces;
	PrevalenceTimeSeries<int> populationSize;	

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
	EventFunc CreateHousehold(Pointer<Individual> head,
						      Pointer<Individual> spouse,
							  std::unordered_set<Pointer<Individual>> offspring,
							  std::unordered_set<Pointer<Individual>> other);

	// Algorithm S6: Birth
	EventFunc Birth(int motherHID, Pointer<Individual> mother, 
								   Pointer<Individual> father);

	// Algorithm S7: Joining a household
	EventFunc JoinHousehold(Pointer<Individual>, long hid);

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
	/// Scheduling
	////////////////////////////////////////////////////////
	EventQueue<> eq;

	void Schedule(int t, EventQueue<>::EventFunc ef);

	////////////////////////////////////////////////////////
	/// Utility
	////////////////////////////////////////////////////////
	void PurgeReferencesToIndividual(Pointer<Individual> host,
										    Pointer<Individual> idv);

	void DeleteIndividual(Pointer<Individual> idv);

	void ChangeHousehold(Pointer<Individual> idv, int newHID, HouseholdPosition newRole);


	////////////////////////////////////////////////////////
	/// Data
	////////////////////////////////////////////////////////



	unordered_set<Pointer<Individual>> population;
	map<long, Pointer<Household>> households;

	unordered_set<Pointer<Individual>> maleSeeking;
	unordered_set<Pointer<Individual>> femaleSeeking;

	Distributions distributions;
	Constants constants;

	HouseholdGen householdGen;

	long nHouseholds;

	RNG rng;
};