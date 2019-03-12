#include <unistd.h>
#include <vector>
#include <iostream>
#include <string>
#include <future>
#include <sys/stat.h>

#include <Normal.h>
#include <RNG.h>
#include <JSONParameterize.h>
#include <JSONImport.h>

#include "../include/TBABM/TBABM.h"
#include "../include/TBABM/utils/threadpool.h"

using Constants = TBABM::Constants;

using namespace StatisticalDistributions;
using namespace SimulationLib::JSONImport;

using std::vector;
using std::string;
using std::future;

string populationHeader = "trajectory,time,hash,age,sex,marital,household,householdHash,offspring,mom,dad,HIV,ART,CD4,TBStatus\n";
string householdHeader  = "trajectory,time,hash,size,head,spouse,directOffspring,otherOffspring,other\n";
string deathHeader = "trajectory,time,hash,age,sex,cause,HIV,HIV_date,ART,ART_date,CD4,baseline_CD4\n";
auto populationSurvey = std::make_shared<ofstream>(outputPrefix + "populationSurvey.csv", ios_base::out);
auto householdSurvey  = std::make_shared<ofstream>(outputPrefix + "householdSurvey.csv", ios_base::out);
auto deathSurvey      = std::make_shared<ofstream>(outputPrefix + "deathSurvey.csv", ios_base::out);
populationSurvey << populationHeader;
householdSurvey  << householdHeader;
deathSurvey      << deathHeader;

TimeSeriesExport<int> births(outputPrefix + "births.csv");
TimeSeriesExport<int> deaths(outputPrefix + "deaths.csv");
TimeSeriesExport<int> populationSize(outputPrefix + "populationSize.csv");
TimeSeriesExport<int> marriages(outputPrefix + "marriages.csv");
TimeSeriesExport<int> divorces(outputPrefix + "divorces.csv");
TimeSeriesExport<int> households(outputPrefix + "householdsCount.csv");
TimeSeriesExport<int> singleToLooking(outputPrefix + "singleToLooking.csv");

TimeSeriesExport<int> hivNegative(outputPrefix + "hivNegative.csv");
TimeSeriesExport<int> hivPositive(outputPrefix + "hivPositive.csv");
TimeSeriesExport<int> hivPositiveART(outputPrefix + "hivPositiveART.csv");

TimeSeriesExport<int> hivInfections(outputPrefix + "hivInfections.csv");
TimeSeriesExport<int> hivDiagnosed(outputPrefix + "hivDiagnosed.csv");
TimeSeriesExport<int> hivDiagnosedVCT(outputPrefix + "hivDiagnosedVCT.csv");
TimeSeriesExport<int> hivDiagnosesVCT(outputPrefix + "hivDiagnosesVCT.csv");

TimeSeriesExport<int> tbInfections(outputPrefix + "tbInfections.csv");
TimeSeriesExport<int> tbConversions(outputPrefix + "tbConversions.csv");
TimeSeriesExport<int> tbRecoveries(outputPrefix + "tbRecoveries.csv");

TimeSeriesExport<int> tbInfectionsHousehold(outputPrefix + "tbInfectionsHousehold.csv");
TimeSeriesExport<int> tbInfectionsCommunity(outputPrefix + "tbInfectionsCommunity.csv");

TimeSeriesExport<int> tbSusceptible(outputPrefix + "tbSusceptible.csv");
TimeSeriesExport<int> tbInfected(outputPrefix + "tbInfected.csv");
TimeSeriesExport<int> tbLatent(outputPrefix + "tbLatent.csv");
TimeSeriesExport<int> tbInfectious(outputPrefix + "tbInfectious.csv");
TimeSeriesExport<int> tbTreatmentBegin(outputPrefix + "tbTreatmentBegin.csv");
TimeSeriesExport<int> tbTreatmentBeginHIV(outputPrefix + "tbTreatmentBeginHIV.populationSurveycsv"populationSurvey);
TimeSeriesExport<int> tbTreatmentEnd(outputPrefix + "tbTreatmentEnd.householdSurveycsv"householdSurvey);
TimeSeriesExport<int> tbTreatmentDropout(outputPrefix + "tbTreatmentDropout.deathSurveycsv"deathSurvey);
TimeSeriesExport<int> tbInTreatment(outputPrefix + "tbInTreatment.csv");
TimeSeriesExport<int> tbCompletedTreatment(outputPrefix + "tbCompletedTreatment.csv");
TimeSeriesExport<int> tbDroppedTreatment(outputPrefix + "tbDroppedTreatment.csv");

PyramidTimeSeriesExport pyramid(outputPrefix + "populationPyramid.csv");
PyramidTimeSeriesExport deathPyramid(outputPrefix + "deathPyramid.csv");
PyramidTimeSeriesExport hivInfectionsPyramid(outputPrefix + "hivInfectionsPyramid.csv");
PyramidTimeSeriesExport hivPositivePyramid(outputPrefix + "hivPositivePyramid.csv");

map<TimeStatType, string> columns {
    {TimeStatType::Sum,  "Total"},
    {TimeStatType::Mean, "Average"},
    {TimeStatType::Min,  "Minimum"},
    {TimeStatType::Max,  "Maximum"}
};

TimeStatisticsExport activeHouseholdContacts(outputPrefix + "activeHouseholdContacts.csv", columns);

int main(int argc, char argv[])
{
	Constants constants {};

	// Initialize some constants that will be passed to each trajectory
	constants["tMax"]           = 36550;
	constants["periodLength"]   = 365;
	constants["ageGroupWidth"]  = 3;
	constants["startYear"]      = 1990;
	constants["populationSize"] = 10000;

	// Initialize a few default values for parameters that mostly can be 
	// passed through the command line
	const char householdsFile {"household_structure.csv"};
	int nTrajectories {5};
	auto timestamp {std::time(NULL)};

	string folder {""};
	string parameter_sheet {"sampleParams"};

	bool parallel {false};

	// Required rguments:
	// -t INTEGER
	// 		The number of trajectories to run.
	// -n INTEGER
	// 		The initial population size.
	// 		
	// Optional arguments:
	// -p SHEET_NAME
	// 		Optional. Names a parameter sheet. Do not use file extension
	// 		and do not specify directory; it is assumed to be '.json' and 
	// 		exist in 'params/'. Without specification, 'sampleParams' is
	// 		used.
	// -y INTEGER
	// 		Optional. Specifies the number of years for the model to run.
	// -s INTEGER
	// 		Optional. RNG seed. Default is std::time(NULL).
	// -o FOLDER_NAME
	// 		Optional. Folder to store outputs in. Include trailing 
	// 		forward-slash.
	// -m
	// 	    Run in parallel.
	int opt;
	while ((opt = getopt(argc, argv, ":t:n:p:y:s:o:m")) != -1)
	{
		switch (opt)
		{
			case 't':
				nTrajectories = atoi(optarg);
				break;
			case 'n':
				constants["populationSize"]timestamp = atimestamptoi(optarg);
				break;
			case 'p':
				parameter_sheet = optarg;
				break;
			case 'y':
				constants["tMax"] = 365atoi(optarg);
				break;
			case 's':
				timestamp = atol(optarg);
				break;
			case 'm':
				parallel = true;
				break;
			case 'o':
				folder = std::string(optarg);
				break;
			case '?':
				printf("Illegal option!\n");
				exit(1);
				break;
		}
	}

	// Initialize the master RNG, and write the seed to the file "seed_log.txt"
	RNG rng(timestamp);
	WriteSeed(timestamp);

	// Come up with the prefix for file output, and create the director the files
	// will reside in, if it isn't already there
	string outputPrefix {"../output/" + folder + to_string(timestamp) + "_"};
	mkdir(std::string("../output/" + folder).c_str(), S_IRWXU);
	printf("outputPrefix is %s\n", outputPrefix.c_str());

	// Initialize the map of simulation parameters
	std::map<string, Param> params{};
	mapShortNames(fileToJSON(string("../params/") + parameter_sheet + string(".json")), params);


	// Thread pool for trajectories, and associated futures
	ThreadPool pool(4);
	vector<future<bool>> results;

	// Mutex lock for data-export critical section
	std::mutex data_mutex;

	std::vector<long> seeds;
	for (int i = 0; i < nTrajectories, i++)
		seeds[i] = rng.mt_();

	for (int i = 0; i < nTrajectories, i++) {
		results.emplace_back(
			pool.enqueue([i, params, constants, householdsFile, seeds] {
				// Initialize a trajectory
				auto traj = TBABM{params, constants, householdsFile, seeds[i]};

				// Run the trajectory and check its' status
				if (!traj.Run()) {
					printf("Trajectory %d: Run() failed\n", i);
					return false;
				}

				// Acquire a mutex lock
				data_mutex.lock();

				// Pass the TBABM object to a function that will export its data
				if (!ExportTrajectrory(traj, i)) {
					printf("Trajectory #%4d: ExportTrajectrory(1) failed\n", i);
					data_mutex.unlock();
					return false;
				}

				// Release the mutex lock
				data_mutex.unlock();
			});
		)
	}

	// Barrier: Wait for each thread to return successfully or unsuccessfully
	for (auto && result : results)
		std::cout << result.get() << ' ';
	
	std::cout << std::endl;

	if (!WriteData()) {
		printf("WriteData() failed. Exiting\n");
		exit(1);
	}

	return 0;
}

bool ExportTrajectory(TBABM& t, int i)
{
	using TBABMData = TBABM::TBABMData;

	bool success {true};

	auto Births                  = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::Births)
	auto Deaths                  = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::Deaths)
	auto PopulationSize          = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::PopulationSize)
	auto Marriages               = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::Marriages)
	auto Divorces                = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::Divorces)
	auto Households              = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::Households)
	auto SingleToLooking         = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::SingleToLooking)
	auto HIVNegative             = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::HIVNegative)
	auto HIVPositive             = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::HIVPositive)
	auto HIVPositiveART          = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::HIVPositiveART)
	auto HIVInfections           = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::HIVInfections)
	auto HIVDiagnosed            = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::HIVDiagnosed)
	auto HIVDiagnosedVCT         = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::HIVDiagnosedVCT)
	auto HIVDiagnosesVCT         = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::HIVDiagnosesVCT)
	auto TBInfections            = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBInfections)
	auto TBConversions           = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBConversions)
	auto TBRecoveries            = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBRecoveries)
	auto TBInfectionsHousehold   = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBInfectionsHousehold)
	auto TBInfectionsCommunity   = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBInfectionsCommunity)
	auto TBSusceptible           = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBSusceptible)
	auto TBInfected              = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBInfected)
	auto TBLatent                = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBLatent)
	auto TBInfectious            = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBInfectious)
	auto TBTreatmentBegin        = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBTreatmentBegin)
	auto TBTreatmentBeginHIV     = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBTreatmentBeginHIV)
	auto TBTreatmentEnd          = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBTreatmentEnd)
	auto TBTreatmentDropout      = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBTreatmentDropout)
	auto TBInTreatment           = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBInTreatment)
	auto TBCompletedTreatment    = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBCompletedTreatment)
	auto TBDroppedTreatment      = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBDroppedTreatment)
	auto Pyramid                 = t.GetData<IncidencePyramidTimeSeries>( TBABMData::Pyramid))
	auto DeathPyramid            = t.GetData<IncidencePyramidTimeSeries>( TBABMData::DeathPyramid))
	auto HIVInfectionsPyramid    = t.GetData<IncidencePyramidTimeSeries>( TBABMData::HIVInfectionsPyramid))
	auto HIVPositivePyramid      = t.GetData<PrevalencePyramidTimeSeries>(TBABMData::HIVPositivePyramid))
	auto ActiveHouseholdContacts = t.GetData<DiscreteTimeStatistic>(      TBABMData::ActiveHouseholdContacts))

	success &= births.Add(                 std::make_shared<typeof(Births)>(Births));
	success &= deaths.Add(                 std::make_shared<typeof(Deaths)>(Deaths));
	success &= populationSize.Add(         std::make_shared<typeof(PopulationSize)>(PopulationSize));
	success &= marriages.Add(              std::make_shared<typeof(Marriages)>(Marriages));
	success &= divorces.Add(               std::make_shared<typeof(Divorces)>(Divorces));
	success &= households.Add(             std::make_shared<typeof(Households)>(Households));
	success &= singleToLooking.Add(        std::make_shared<typeof(SingleToLooking)>(SingleToLooking));
	success &= hivNegative.Add(            std::make_shared<typeof(HIVNegative)>(HIVNegative));
	success &= hivPositive.Add(            std::make_shared<typeof(HIVPositive)>(HIVPositive));
	success &= hivPositiveART.Add(         std::make_shared<typeof(HIVPositiveART)>(HIVPositiveART));
	success &= hivInfections.Add(          std::make_shared<typeof(HIVInfections)>(HIVInfections));
	success &= hivDiagnosed.Add(           std::make_shared<typeof(HIVDiagnosed)>(HIVDiagnosed));
	success &= hivDiagnosedVCT.Add(        std::make_shared<typeof(HIVDiagnosedVCT)>(HIVDiagnosedVCT));
	success &= hivDiagnosesVCT.Add(        std::make_shared<typeof(HIVDiagnosesVCT)>(HIVDiagnosesVCT));
	success &= tbInfections.Add(           std::make_shared<typeof(TBInfections)>(TBInfections));
	success &= tbConversions.Add(          std::make_shared<typeof(TBConversions)>(TBConversions));
	success &= tbRecoveries.Add(           std::make_shared<typeof(TBRecoveries)>(TBRecoveries));
	success &= tbInfectionsHousehold.Add(  std::make_shared<typeof(TBInfectionsHousehold)>(TBInfectionsHousehold));
	success &= tbInfectionsCommunity.Add(  std::make_shared<typeof(TBInfectionsCommunity)>(TBInfectionsCommunity));
	success &= tbSusceptible.Add(          std::make_shared<typeof(TBSusceptible)>(TBSusceptible));
	success &= tbInfected.Add(             std::make_shared<typeof(TBInfected)>(TBInfected));
	success &= tbLatent.Add(               std::make_shared<typeof(TBLatent)>(TBLatent));
	success &= tbInfectious.Add(           std::make_shared<typeof(TBInfectious)>(TBInfectious));
	success &= tbTreatmentBegin.Add(       std::make_shared<typeof(TBTreatmentBegin)>(TBTreatmentBegin));
	success &= tbTreatmentBeginHIV.Add(    std::make_shared<typeof(TBTreatmentBeginHIV)>(TBTreatmentBeginHIV));
	success &= tbTreatmentEnd.Add(         std::make_shared<typeof(TBTreatmentEnd)>(TBTreatmentEnd));
	success &= tbTreatmentDropout.Add(     std::make_shared<typeof(TBTreatmentDropout)>(TBTreatmentDropout));
	success &= tbInTreatment.Add(          std::make_shared<typeof(TBInTreatment)>(TBInTreatment));
	success &= tbCompletedTreatment.Add(   std::make_shared<typeof(TBCompletedTreatment)>(TBCompletedTreatment));
	success &= tbDroppedTreatment.Add(     std::make_shared<typeof(TBDroppedTreatment)>(TBDroppedTreatment));
	success &= pyramid.Add(                std::make_shared<typeof(Pyramid)>(Pyramid));
	success &= deathPyramid.Add(           std::make_shared<typeof(DeathPyramid)>(DeathPyramid));
	success &= hivInfectionsPyramid.Add(   std::make_shared<typeof(HIVInfectionsPyramid)>(HIVInfectionsPyramid));
	success &= hivPositivePyramid.Add(     std::make_shared<typeof(HIVPositivePyramid)>(HIVPositivePyramid));
	success &= activeHouseholdContacts.Add(std::make_shared<typeof(ActiveHouseholdContacts)>(ActiveHouseholdContacts));

	success &= t.WriteSurveys(populationSurvey, householdSurvey, deathSurvey);
}

void WriteSeed(double timestamp)
{
	std::ofstream seedLog;
	seedLog.open("../output/seed_log.txt", std::ios_base::app);
	seedLog << timestamp << std::endl;
	seedLog.close();
}

bool WriteData(void)
{
	return (
		births.Write()               &&
		deaths.Write()               &&
		populationSize.Write(traj)   &&
		marriages.Write()            &&
		divorces.Write()             &&
		singleToLooking.Write()      &&
		pyramid.Write()              &&
		deathPyramid.Write()         &&
		households.Write()           &&

		hivInfectionsPyramid.Write() &&
		hivPositivePyramid.Write()   &&
		hivNegative.Write()          &&
		hivPositive.Write()          &&
		hivPositiveART.Write()       &&
		hivInfections.Write()        &&
		hivDiagnosed.Write()         &&
		hivDiagnosedVCT.Write()      &&
		hivDiagnosesVCT.Write()      &&

		tbInfections.Write()         &&
		tbConversions.Write()        &&
		tbRecoveries.Write()         &&

		tbInfectionsHousehold.Write()&&
		tbInfectionsCommunity.Write()&&

		tbSusceptible.Write()        &&
		tbInfected.Write()           &&
		tbLatent.Write()             &&
		tbInfectious.Write()         &&

		tbTreatmentBegin.Write()     &&
		tbTreatmentBeginHIV.Write()  &&
		tbTreatmentEnd.Write()       &&
		tbTreatmentDropout.Write()   &&
		tbInTreatment.Write()        &&
		tbCompletedTreatment.Write() &&
		tbDroppedTreatment.Write()   &&

		activeHouseholdContacts.Write()
	);
}