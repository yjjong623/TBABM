#pragma once

#include <map>
#include <string>
#include <ctime>
#include <fstream>
#include <iostream>

#include <StatisticalDistribution.h>
#include <Exponential.h>
#include <RNG.h>
#include <EventQueue.h>
#include <PrevalenceTimeSeries.h>
#include <IncidenceTimeSeries.h>
#include <IncidencePyramidTimeSeries.h>
#include <CSVExport.h>
#include <EventQueue.h>

#include <Param.h>
#include <DataFrame.h>
#include <JSONImport.h>

#include "Individual.h"
#include "Household.h"
#include "HouseholdGen.h"


using std::map;
using std::unordered_set;
using std::vector;
using std::string;

using namespace StatisticalDistributions;
using namespace SimulationLib;
using namespace SimulationLib::JSONImport;

template <typename T>
using Pointer = std::shared_ptr<T>;

class TBABM {
public:
	using Params = map<string, Param>;
	using Constants = map<string, long double>;

	using EQ = EventQueue<double, bool>;
	using EventFunc = EQ::EventFunc;
	using SchedulerT = EQ::SchedulerT;

	enum class TBABMData {
		HIVNegative, 
		// HIVPositiveNIC, 
		// HIVPositiveIC, 
		HIVPositiveART, 
		HIVPositive,

		HIVInfections,
		HIVDiagnosed,
		HIVDiagnosedVCT,
		HIVDiagnosesVCT,
		
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
		PopulationSize,
		Pyramid,
		Households
	};

	TBABM(Params params, 
		  std::map<string, long double> constants,
		  const char *householdsFile, 
		  long seed) : 
		params(params),
		constants(constants),

		births("births", 0, constants["tMax"], constants["periodLength"]),
		deaths("deaths", 0, constants["tMax"], constants["periodLength"]),
		marriages("marriages", 0, constants["tMax"], constants["periodLength"]),
		divorces("divorces", 0, constants["tMax"], constants["periodLength"]),
		populationSize("populationSize", constants["tMax"], constants["periodLength"]),

		hivNegative("hivNegative", constants["tMax"], constants["periodLength"]),
		hivPositive("hivPositive", constants["tMax"], constants["periodLength"]),
		hivPositiveART("hivPositiveART", constants["tMax"], constants["periodLength"]),

		hivInfections("hivInfections", 0, constants["tMax"], constants["periodLength"]),
		hivDiagnosed("hivDiagnosed", constants["tMax"], constants["periodLength"]),
		hivDiagnosedVCT("hivDiagnosedVCT", constants["tMax"], constants["periodLength"]),
		hivDiagnosesVCT("hivDiagnosesVCT", 0, constants["tMax"], constants["periodLength"]),

		pyramid("Population pyramid", 0, constants["tMax"], 365, 2, {10, 20, 30, 40, 50, 60, 70, 80, 90}),
		householdsCount("households", 0, constants["tMax"], 365),

		population({}),
		households({}),

		maleSeeking({}),
		femaleSeeking({}),

		seekingART({}),

		householdGen(householdsFile),

		nHouseholds(0),
		rng(seed) {
			for (auto it = params.begin(); it != params.end(); it++) {
				if (it->second.getType() == SimulationLib::Type::file_type) {
					auto j = JSONImport::fileToJSON(it->second.getFileName());
					fileData[it->first] = DataFrameFile{j};
				}
			}

			// Open file for mean survival time
			meanSurvivalTime.open("../output/meanSurvivalTimeNoART.csv", ios_base::app);
			meanSurvivalTime << "years lived,age at infection\n";
		};

	bool Run(void);

	template <typename T>
	T* GetData(TBABMData field);

private:
	IncidenceTimeSeries<int> births;
	IncidenceTimeSeries<int> deaths;
	IncidenceTimeSeries<int> marriages;
	IncidenceTimeSeries<int> divorces;
	PrevalenceTimeSeries<int> populationSize;

	IncidencePyramidTimeSeries pyramid;	
	IncidenceTimeSeries<int> householdsCount;

	PrevalenceTimeSeries<int> hivNegative;
	IncidenceTimeSeries<int>  hivInfections;
	PrevalenceTimeSeries<int> hivPositive;
	PrevalenceTimeSeries<int> hivPositiveART;
	PrevalenceTimeSeries<int> hivDiagnosed;
	PrevalenceTimeSeries<int> hivDiagnosedVCT;
	IncidenceTimeSeries<int>  hivDiagnosesVCT;

	////////////////////////////////////////////////////////
	/// Demographic Events
	////////////////////////////////////////////////////////

	// Algorithm S2: Create a population at simulation time 0
	void CreatePopulation(int t, long size);

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
	EventFunc Birth(Pointer<Individual> mother, Pointer<Individual> father);

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

	EventFunc Pregnancy(Pointer<Individual> f);

	// Update the population pyramid
	EventFunc UpdatePyramid(void);

	// Update households
	EventFunc UpdateHouseholds(void);

	////////////////////////////////////////////////////////
	/// Demographic Utilities
	////////////////////////////////////////////////////////
	void PurgeReferencesToIndividual(Pointer<Individual> host,
										    Pointer<Individual> idv);

	void DeleteIndividual(Pointer<Individual> idv);

	void ChangeHousehold(Pointer<Individual> idv, int newHID, HouseholdPosition newRole);

	////////////////////////////////////////////////////////
	/// HIV Events
	////////////////////////////////////////////////////////
	EventFunc ARTGuidelineChange(void);
	EventFunc ARTInitiate(Pointer<Individual>);
	EventFunc HIVInfection(Pointer<Individual>);
	EventFunc MortalityCheck(Pointer<Individual>);
	EventFunc VCTDiagnosis(Pointer<Individual>);

	////////////////////////////////////////////////////////
	/// HIV Utilities
	////////////////////////////////////////////////////////

	bool ARTEligible(int t, Pointer<Individual> idv);
	void HIVInfectionCheck(int t, Pointer<Individual> idv);

	////////////////////////////////////////////////////////
	/// Scheduling
	////////////////////////////////////////////////////////
	EventQueue<> eq;

	void Schedule(int t, EventQueue<>::EventFunc ef);

	////////////////////////////////////////////////////////
	/// Data
	////////////////////////////////////////////////////////

	map<string, DataFrameFile> fileData;
	Params params;

	unordered_set<Pointer<Individual>> population;
	map<long, Pointer<Household>> households;

	unordered_set<Pointer<Individual>> maleSeeking;
	unordered_set<Pointer<Individual>> femaleSeeking;

	unordered_set<Pointer<Individual>> seekingART; // Individuals seeking ART

	Constants constants;

	HouseholdGen householdGen;

	long nHouseholds;

	RNG rng;

	ofstream meanSurvivalTime;
};