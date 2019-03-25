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
#include <PrevalencePyramidTimeSeries.h>
#include <CSVExport.h>
#include <EventQueue.h>

#include <Param.h>
#include <DataFrame.h>
#include <JSONImport.h>

#include "Individual.h"
#include "IndividualTypes.h"
#include "Names.h"

#include "Household.h"
#include "HouseholdGen.h"

#include "Pointers.h"

#include "utils/termcolor.h"

using std::map;
using std::vector;
using std::string;

using namespace StatisticalDistributions;
using namespace SimulationLib;
using namespace SimulationLib::JSONImport;

class TBABM {
public:
	using Params = map<string, Param>;
	using Constants = map<string, long double>;

	using EQ = EventQueue<double, bool>;
	using EventFunc = EQ::EventFunc;
	using SchedulerT = EQ::SchedulerT;

	enum class TBABMData {
		HIVNegative, 
		HIVPositiveART, 
		HIVPositive,
		HIVPositivePyramid,

		HIVInfections,
		HIVInfectionsPyramid,
		HIVDiagnosed,
		HIVDiagnosedVCT,
		HIVDiagnosesVCT,

		TBSusceptible,
		TBLatent,
		TBInfectious,

		TBExperienced,

		TBInfections,
		TBIncidence,
		TBRecoveries,

		TBInfectionsHousehold,
		TBInfectionsCommunity,

		TBInTreatment,
		TBCompletedTreatment,
		TBDroppedTreatment,

		TBTreatmentBegin,
		TBTreatmentBeginHIV,
		TBTreatmentEnd,
		TBTreatmentDropout,

		ActiveHouseholdContacts,

		Marriages,
		Divorces,
		Births,
		Deaths,
		PopulationSize,
		Pyramid,
		DeathPyramid,
		Households,
		SingleToLooking
	};

	TBABM(Params _params, 
		  std::map<string, long double> constants,
		  const char *householdsFile, 
		  long _seed) : 

		params(_params),
		constants(constants),

		births(         "births",          0, constants["tMax"], constants["periodLength"]),
		deaths(         "deaths",          0, constants["tMax"], constants["periodLength"]),
		marriages(      "marriages",       0, constants["tMax"], constants["periodLength"]),
		divorces(       "divorces",        0, constants["tMax"], constants["periodLength"]),
		populationSize( "populationSize",     constants["tMax"], constants["periodLength"]),
		singleToLooking("singleToLooking", 0, constants["tMax"], constants["periodLength"]),

		hivNegative(   "hivNegative",    constants["tMax"], constants["periodLength"]),
		hivPositive(   "hivPositive",    constants["tMax"], constants["periodLength"]),
		hivPositiveART("hivPositiveART", constants["tMax"], constants["periodLength"]),
		hivPositivePyramid("hivPositive pyramid", 0, constants["tMax"], 365, 2, {15, 25, 35, 45, 55, 66}),

		hivInfectionsPyramid("hivInfections pyramid", 0, constants["tMax"], 365, 2, {10, 20, 30, 40, 50, 60, 70, 80, 90}),
		hivInfections(  "hivInfections", 0,   constants["tMax"], constants["periodLength"]),
		hivDiagnosed(   "hivDiagnosed",       constants["tMax"], constants["periodLength"]),
		hivDiagnosedVCT("hivDiagnosedVCT",    constants["tMax"], constants["periodLength"]),
		hivDiagnosesVCT("hivDiagnosesVCT", 0, constants["tMax"], constants["periodLength"]),

		tbInfections( "tbInfections",  0, constants["tMax"], constants["periodLength"]),
		tbIncidence(  "tbIncidence"  , 0, constants["tMax"], constants["periodLength"]),
		tbRecoveries( "tbRecoveries",  0, constants["tMax"], constants["periodLength"]),

		tbInfectionsHousehold( "tbInfectionsHousehold",  0, constants["tMax"], constants["periodLength"]),
		tbInfectionsCommunity( "tbInfectionsCommunity",  0, constants["tMax"], constants["periodLength"]),

		tbSusceptible("tbSusceptible", constants["tMax"], constants["periodLength"]),
		tbLatent(     "tbLatent",      constants["tMax"], constants["periodLength"]),
		tbInfectious( "tbInfectious",  constants["tMax"], constants["periodLength"]),

		tbExperienced("tbExperienced", constants["tMax"], constants["periodLength"]),
		tbExperiencedPyr("tbExperiencedPyr", 0, constants["tMax"], constants["periodLength"], 2, {0, 15, 30, 45, 60, 75}),

		tbTreatmentBegin(   "tbTreatmentBegin",   0, constants["tMax"], constants["periodLength"]),
		tbTreatmentBeginHIV("tbTreatmentBeginHIV",0, constants["tMax"], constants["periodLength"]),
		tbTreatmentEnd(     "tbTreatmentEnd",     0, constants["tMax"], constants["periodLength"]),
		tbTreatmentDropout( "tbTreatmentDropout", 0, constants["tMax"], constants["periodLength"]),

		tbInTreatment(       "tbInTreatment",        constants["tMax"], constants["periodLength"]),
		tbCompletedTreatment("tbCompletedTreatment", constants["tMax"], constants["periodLength"]),
		tbDroppedTreatment(  "tbDroppedTreatment",   constants["tMax"], constants["periodLength"]),

		activeHouseholdContacts("activeHouseholdContacts"),

		pyramid("Population pyramid", 0, constants["tMax"], 365, 2, {10, 20, 30, 40, 50, 60, 70, 80, 90}),
		deathPyramid("Death pyramid", 0, constants["tMax"], 365, 2, {10, 20, 30, 40, 50, 60, 70, 80, 90}),
		householdsCount("households", 0, constants["tMax"], 365),

		population({}),
		households({}),

		maleSeeking({}),
		femaleSeeking({}),

		seekingART({}),

		nHouseholds(0),
		seed(_seed),
		rng(_seed),
		householdGen(householdsFile, 
					 std::make_shared<Params>(params),
					 std::make_shared<map<string, DataFrameFile>>(fileData),
					 eq,
					 {tbInfections, tbIncidence, tbRecoveries, \
					  tbInfectionsHousehold, tbInfectionsCommunity,
					  tbSusceptible, tbLatent, tbInfectious, tbExperienced, tbExperiencedPyr,\
					  tbTreatmentBegin, tbTreatmentBeginHIV, tbTreatmentEnd, tbTreatmentDropout, \
					  tbInTreatment, tbCompletedTreatment, tbDroppedTreatment, activeHouseholdContacts},
					 CreateIndividualHandlers([this] (weak_ptr<Individual> i, int t, DeathCause dc) -> void \
					 						  { return Schedule(t, Death(i, dc)); },
					 						  [this] (int t) -> double { return (double)tbInfectious(t)/(double)populationSize(t); }
											  )) {
			for (auto it = params.begin(); it != params.end(); it++) {
				if (it->second.getType() == SimulationLib::Type::file_type) {
					auto j = JSONImport::fileToJSON(it->second.getFileName());
					fileData[it->first] = DataFrameFile{j};
				}
			}

			printf("Seed is %ld\n", seed);
		};

	bool Run(void);

	template <typename T>
	T
	GetData(TBABMData field);

	bool WriteSurveys(ofstream& ps, 
					  ofstream& hs, 
					  ofstream& ds);


private:
	IncidenceTimeSeries<int> births;
	IncidenceTimeSeries<int> deaths;
	IncidenceTimeSeries<int> marriages;
	IncidenceTimeSeries<int> divorces;
	PrevalenceTimeSeries<int> populationSize;
	IncidenceTimeSeries<int> singleToLooking;

	IncidencePyramidTimeSeries pyramid;	
	IncidencePyramidTimeSeries deathPyramid;
	IncidenceTimeSeries<int> householdsCount;

	PrevalenceTimeSeries<int>   hivNegative;
	IncidenceTimeSeries<int>    hivInfections;
	PrevalenceTimeSeries<int>   hivPositive;
	PrevalenceTimeSeries<int>   hivPositiveART;
	PrevalenceTimeSeries<int>   hivDiagnosed;
	PrevalenceTimeSeries<int>   hivDiagnosedVCT;
	IncidenceTimeSeries<int>    hivDiagnosesVCT;
	PrevalencePyramidTimeSeries hivPositivePyramid;
	IncidencePyramidTimeSeries  hivInfectionsPyramid;

	IncidenceTimeSeries<int>  tbInfections;  // Individuals transitioning from S to L
	IncidenceTimeSeries<int>  tbIncidence;   // Individuals transitioning from L to I
	IncidenceTimeSeries<int>  tbRecoveries;  // Individuals transitioning from I to L

	IncidenceTimeSeries<int>  tbInfectionsHousehold; // Individuals infected by household member
	IncidenceTimeSeries<int>  tbInfectionsCommunity; // Individuals infected by community

	PrevalenceTimeSeries<int> tbSusceptible; // # Individuals in S
	PrevalenceTimeSeries<int> tbLatent;      // # Individuals in L
	PrevalenceTimeSeries<int> tbInfectious;  // # Individuals in I

	PrevalenceTimeSeries<int> tbExperienced; // # Individuals who are experienced with TB (L or I)
	PrevalencePyramidTimeSeries tbExperiencedPyr; // Pyramid of the above


	IncidenceTimeSeries<int>  tbTreatmentBegin;   // Individuals initiating treatment
	IncidenceTimeSeries<int>  tbTreatmentBeginHIV;// Initiating but also HIV+
	IncidenceTimeSeries<int>  tbTreatmentEnd;     // Individuals completing treatment
	IncidenceTimeSeries<int>  tbTreatmentDropout; // Individuals dropping out

	PrevalenceTimeSeries<int> tbInTreatment;        // Individuals in treatment
	PrevalenceTimeSeries<int> tbCompletedTreatment; // Individuals who completed
	PrevalenceTimeSeries<int> tbDroppedTreatment;   // Individuals who dropped

	DiscreteTimeStatistic  activeHouseholdContacts; // For each individual diagnosed with active TB,
													// the percentage of household contacts who have
													// active TB.

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
	EventFunc CreateHousehold(weak_p<Individual> head,
						      weak_p<Individual> spouse,
							  std::vector<weak_p<Individual>> offspring,
							  std::vector<weak_p<Individual>> other);

	// Algorithm S6: Birth
	EventFunc Birth(weak_p<Individual> mother, weak_p<Individual> father);

	// Algorithm S7: Joining a household
	EventFunc JoinHousehold(weak_p<Individual>, long hid);

	// Algorithm S9: Change of age groups
	EventFunc ChangeAgeGroup(weak_p<Individual>);

	// Algorithm S10: Natural death
	EventFunc Death(weak_p<Individual>, DeathCause deathCause);

	// Algorithm S11: Leave current household to form new household
	EventFunc LeaveHousehold(weak_p<Individual>);

	// Algorithm S12: Change of marital status from single to looking
	EventFunc SingleToLooking(weak_p<Individual>);

	// Algorithm S13: Marriage
	EventFunc Marriage(weak_p<Individual> m, weak_p<Individual> f);

	// Algorithm S14: Divorce
	EventFunc Divorce(weak_p<Individual> m, weak_p<Individual> f);

	EventFunc Pregnancy(weak_p<Individual> f, weak_p<Individual> m);

	// Update the population pyramid
	EventFunc UpdatePyramid(void);

	// Update households
	EventFunc UpdateHouseholds(void);

	// Population survey
	EventFunc Survey(void);

	// BYPASS for debugging
	EventFunc ExogenousBirth(void);

	////////////////////////////////////////////////////////
	/// Demographic Utilities
	////////////////////////////////////////////////////////
	void InitialEvents(weak_p<Individual> idv, double t, double dt);

	void PurgeReferencesToIndividual(weak_p<Individual> host,
								     weak_p<Individual> idv);

	void DeleteIndividual(weak_p<Individual> idv);

	void ChangeHousehold(weak_p<Individual> idv, int time, int newHID, HouseholdPosition newRole);

	void SurveyDeath(shared_p<Individual> idv, int t, DeathCause deathCause);

	Names name_gen;

	////////////////////////////////////////////////////////
	/// HIV Events
	////////////////////////////////////////////////////////
	EventFunc ARTGuidelineChange(void);
	EventFunc ARTInitiate(weak_p<Individual>);
	EventFunc HIVInfectionCheck(weak_p<Individual>);
	EventFunc HIVInfection(weak_p<Individual>);
	EventFunc MortalityCheck(weak_p<Individual>);
	EventFunc VCTDiagnosis(weak_p<Individual>);

	////////////////////////////////////////////////////////
	/// HIV Utilities
	////////////////////////////////////////////////////////

	bool ARTEligible(int t, weak_p<Individual> idv);
	void HIVInfectionCheck(int t, weak_p<Individual> idv);

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

	vector<shared_p<Individual>> population;
	map<long, shared_p<Household>> households;

	vector<weak_p<Individual>> maleSeeking;
	vector<weak_p<Individual>> femaleSeeking;

	vector<weak_p<Individual>> seekingART; // Individuals seeking ART

	Constants constants;

	HouseholdGen householdGen;

	long nHouseholds;

	RNG rng;
	long seed;

	string populationSurvey;
	string householdSurvey;
	string deathSurvey;
};