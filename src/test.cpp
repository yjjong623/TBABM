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

TimeSeriesExport<int> births;
TimeSeriesExport<int> deaths;
TimeSeriesExport<int> populationSize;
TimeSeriesExport<int> marriages;
TimeSeriesExport<int> divorces;
TimeSeriesExport<int> households;
TimeSeriesExport<int> singleToLooking;

TimeSeriesExport<int> hivNegative;
TimeSeriesExport<int> hivPositive;
TimeSeriesExport<int> hivPositiveART;

TimeSeriesExport<int> hivInfections;
TimeSeriesExport<int> hivDiagnosed;
TimeSeriesExport<int> hivDiagnosedVCT;
TimeSeriesExport<int> hivDiagnosesVCT;

TimeSeriesExport<int> tbInfections;
TimeSeriesExport<int> tbIncidence;
TimeSeriesExport<int> tbRecoveries;

TimeSeriesExport<int> tbInfectionsHousehold;
TimeSeriesExport<int> tbInfectionsCommunity;

TimeSeriesExport<int> tbSusceptible;
TimeSeriesExport<int> tbLatent;
TimeSeriesExport<int> tbInfectious;

TimeSeriesExport<int> tbExperienced;

TimeSeriesExport<int> tbTreatmentBegin;
TimeSeriesExport<int> tbTreatmentBeginHIV;
TimeSeriesExport<int> tbTreatmentEnd;
TimeSeriesExport<int> tbTreatmentDropout;

TimeSeriesExport<int> tbInTreatment;
TimeSeriesExport<int> tbCompletedTreatment;
TimeSeriesExport<int> tbDroppedTreatment;

PyramidTimeSeriesExport pyramid;
PyramidTimeSeriesExport deathPyramid;
PyramidTimeSeriesExport hivInfectionsPyramid;
PyramidTimeSeriesExport hivPositivePyramid;
PyramidTimeSeriesExport tbExperiencedPyramid;

map<TimeStatType, string> columns {
    {TimeStatType::Sum,  "Total"},
    {TimeStatType::Mean, "Average"},
    {TimeStatType::Min,  "Minimum"},
    {TimeStatType::Max,  "Maximum"}
};

TimeStatisticsExport activeHouseholdContacts(columns);

bool ExportTrajectory(TBABM& t, 
					  int i, 
					  ofstream& populationSurvey, 
					  ofstream& householdSurvey, 
					  ofstream& deathSurvey)
{
	using TBABMData = TBABM::TBABMData;

	bool success {true};

	auto Births                  = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::Births);
	auto Deaths                  = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::Deaths);
	auto PopulationSize          = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::PopulationSize);
	auto Marriages               = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::Marriages);
	auto Divorces                = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::Divorces);
	auto Households              = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::Households);
	auto SingleToLooking         = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::SingleToLooking);
	auto HIVNegative             = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::HIVNegative);
	auto HIVPositive             = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::HIVPositive);
	auto HIVPositiveART          = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::HIVPositiveART);
	auto HIVInfections           = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::HIVInfections);
	auto HIVDiagnosed            = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::HIVDiagnosed);
	auto HIVDiagnosedVCT         = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::HIVDiagnosedVCT);
	auto HIVDiagnosesVCT         = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::HIVDiagnosesVCT);
	auto TBInfections            = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBInfections);
	auto TBIncidence             = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBIncidence);
	auto TBRecoveries            = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBRecoveries);
	auto TBInfectionsHousehold   = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBInfectionsHousehold);
	auto TBInfectionsCommunity   = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBInfectionsCommunity);
	auto TBSusceptible           = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBSusceptible);
	auto TBLatent                = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBLatent);
	auto TBInfectious            = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBInfectious);
	auto TBExperienced           = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBExperienced);
	auto TBTreatmentBegin        = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBTreatmentBegin);
	auto TBTreatmentBeginHIV     = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBTreatmentBeginHIV);
	auto TBTreatmentEnd          = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBTreatmentEnd);
	auto TBTreatmentDropout      = t.GetData<IncidenceTimeSeries<int>>(   TBABMData::TBTreatmentDropout);
	auto TBInTreatment           = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBInTreatment);
	auto TBCompletedTreatment    = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBCompletedTreatment);
	auto TBDroppedTreatment      = t.GetData<PrevalenceTimeSeries<int>>(  TBABMData::TBDroppedTreatment);
	auto Pyramid                 = t.GetData<IncidencePyramidTimeSeries>( TBABMData::Pyramid);
	auto DeathPyramid            = t.GetData<IncidencePyramidTimeSeries>( TBABMData::DeathPyramid);
	auto HIVInfectionsPyramid    = t.GetData<IncidencePyramidTimeSeries>( TBABMData::HIVInfectionsPyramid);
	auto HIVPositivePyramid      = t.GetData<PrevalencePyramidTimeSeries>(TBABMData::HIVPositivePyramid);
	auto TBExperiencedPyramid    = t.GetData<PrevalencePyramidTimeSeries>(TBABMData::TBExperienced);
	auto ActiveHouseholdContacts = t.GetData<DiscreteTimeStatistic>(      TBABMData::ActiveHouseholdContacts);

	success &= births.Add(                 std::make_shared<decltype(Births)>(Births), i);
	success &= deaths.Add(                 std::make_shared<decltype(Deaths)>(Deaths), i);
	success &= populationSize.Add(         std::make_shared<decltype(PopulationSize)>(PopulationSize), i);
	success &= marriages.Add(              std::make_shared<decltype(Marriages)>(Marriages), i);
	success &= divorces.Add(               std::make_shared<decltype(Divorces)>(Divorces), i);
	success &= households.Add(             std::make_shared<decltype(Households)>(Households), i);
	success &= singleToLooking.Add(        std::make_shared<decltype(SingleToLooking)>(SingleToLooking), i);
	success &= hivNegative.Add(            std::make_shared<decltype(HIVNegative)>(HIVNegative), i);
	success &= hivPositive.Add(            std::make_shared<decltype(HIVPositive)>(HIVPositive), i);
	success &= hivPositiveART.Add(         std::make_shared<decltype(HIVPositiveART)>(HIVPositiveART), i);
	success &= hivInfections.Add(          std::make_shared<decltype(HIVInfections)>(HIVInfections), i);
	success &= hivDiagnosed.Add(           std::make_shared<decltype(HIVDiagnosed)>(HIVDiagnosed), i);
	success &= hivDiagnosedVCT.Add(        std::make_shared<decltype(HIVDiagnosedVCT)>(HIVDiagnosedVCT), i);
	success &= hivDiagnosesVCT.Add(        std::make_shared<decltype(HIVDiagnosesVCT)>(HIVDiagnosesVCT), i);
	success &= tbInfections.Add(           std::make_shared<decltype(TBInfections)>(TBInfections), i);
	success &= tbIncidence.Add(            std::make_shared<decltype(TBIncidence)>(TBIncidence), i);
	success &= tbRecoveries.Add(           std::make_shared<decltype(TBRecoveries)>(TBRecoveries), i);
	success &= tbInfectionsHousehold.Add(  std::make_shared<decltype(TBInfectionsHousehold)>(TBInfectionsHousehold), i);
	success &= tbInfectionsCommunity.Add(  std::make_shared<decltype(TBInfectionsCommunity)>(TBInfectionsCommunity), i);
	success &= tbSusceptible.Add(          std::make_shared<decltype(TBSusceptible)>(TBSusceptible), i);
	success &= tbLatent.Add(               std::make_shared<decltype(TBLatent)>(TBLatent), i);
	success &= tbInfectious.Add(           std::make_shared<decltype(TBInfectious)>(TBInfectious), i);
	success &= tbExperienced.Add(		   std::make_shared<decltype(TBExperienced)>(TBExperienced), i);
	success &= tbTreatmentBegin.Add(       std::make_shared<decltype(TBTreatmentBegin)>(TBTreatmentBegin), i);
	success &= tbTreatmentBeginHIV.Add(    std::make_shared<decltype(TBTreatmentBeginHIV)>(TBTreatmentBeginHIV), i);
	success &= tbTreatmentEnd.Add(         std::make_shared<decltype(TBTreatmentEnd)>(TBTreatmentEnd), i);
	success &= tbTreatmentDropout.Add(     std::make_shared<decltype(TBTreatmentDropout)>(TBTreatmentDropout), i);
	success &= tbInTreatment.Add(          std::make_shared<decltype(TBInTreatment)>(TBInTreatment), i);
	success &= tbCompletedTreatment.Add(   std::make_shared<decltype(TBCompletedTreatment)>(TBCompletedTreatment), i);
	success &= tbDroppedTreatment.Add(     std::make_shared<decltype(TBDroppedTreatment)>(TBDroppedTreatment), i);
	success &= pyramid.Add(                std::make_shared<decltype(Pyramid)>(Pyramid), i);
	success &= deathPyramid.Add(           std::make_shared<decltype(DeathPyramid)>(DeathPyramid), i);
	success &= hivInfectionsPyramid.Add(   std::make_shared<decltype(HIVInfectionsPyramid)>(HIVInfectionsPyramid), i);
	success &= hivPositivePyramid.Add(     std::make_shared<decltype(HIVPositivePyramid)>(HIVPositivePyramid), i);
	success &= tbExperiencedPyramid.Add(   std::make_shared<decltype(TBExperiencedPyramid)>(TBExperiencedPyramid), i);
	success &= activeHouseholdContacts.Add(std::make_shared<decltype(ActiveHouseholdContacts)>(ActiveHouseholdContacts));

	success &= t.WriteSurveys(populationSurvey, householdSurvey, deathSurvey);

	return success;
}

void WriteSeed(double timestamp)
{
	std::ofstream seedLog;
	seedLog.open("../output/seed_log.txt", std::ios_base::app);
	seedLog << timestamp << std::endl;
	seedLog.close();
}

bool WriteData(string outputPrefix)
{
	return (
		births.Write(outputPrefix + "births.csv")               &&                                             
		deaths.Write(outputPrefix + "deaths.csv")               &&                                             
		populationSize.Write(outputPrefix + "populationSize.csv")       &&                     
		marriages.Write(outputPrefix + "marriages.csv")            &&                                    
		divorces.Write(outputPrefix + "divorces.csv")             &&                                       
		singleToLooking.Write(outputPrefix + "singleToLooking.csv")      &&                  
		pyramid.Write(outputPrefix + "pyramid.csv")              &&                                          
		deathPyramid.Write(outputPrefix + "deathPyramid.csv")         &&                           
		households.Write(outputPrefix + "households.csv")           &&                                 
		
		hivInfectionsPyramid.Write(outputPrefix + "hivInfectionsPyramid.csv") &&   
		hivPositivePyramid.Write(outputPrefix + "hivPositivePyramid.csv")   &&         
		hivNegative.Write(outputPrefix + "hivNegative.csv")          &&                              
		hivPositive.Write(outputPrefix + "hivPositive.csv")          &&                              
		hivPositiveART.Write(outputPrefix + "hivPositiveART.csv")       &&                     
		hivInfections.Write(outputPrefix + "hivInfections.csv")        &&                        
		hivDiagnosed.Write(outputPrefix + "hivDiagnosed.csv")         &&                           
		hivDiagnosedVCT.Write(outputPrefix + "hivDiagnosedVCT.csv")      &&                  
		hivDiagnosesVCT.Write(outputPrefix + "hivDiagnosesVCT.csv")      &&                  
		
		tbInfections.Write(outputPrefix + "tbInfections.csv")         &&                           
		tbIncidence.Write(outputPrefix + "tbIncidence.csv")        &&                        
		tbRecoveries.Write(outputPrefix + "tbRecoveries.csv")         &&                           
		
		tbInfectionsHousehold.Write(outputPrefix + "tbInfectionsHousehold.csv")&&
		tbInfectionsCommunity.Write(outputPrefix + "tbInfectionsCommunity.csv")&&
		
		tbSusceptible.Write(outputPrefix + "tbSusceptible.csv")        &&                        
		tbLatent.Write(outputPrefix + "tbLatent.csv")             &&                                       
		tbInfectious.Write(outputPrefix + "tbInfectious.csv")         &&                           

		tbExperienced.Write(outputPrefix + "tbExperienced.csv") &&
		
		tbTreatmentBegin.Write(outputPrefix + "tbTreatmentBegin.csv")     &&               
		tbTreatmentBeginHIV.Write(outputPrefix + "tbTreatmentBeginHIV.csv")  &&      
		tbTreatmentEnd.Write(outputPrefix + "tbTreatmentEnd.csv")       &&                     
		tbTreatmentDropout.Write(outputPrefix + "tbTreatmentDropout.csv")   &&         
		tbInTreatment.Write(outputPrefix + "tbInTreatment.csv")        &&                        
		tbCompletedTreatment.Write(outputPrefix + "tbCompletedTreatment.csv") &&   
		tbDroppedTreatment.Write(outputPrefix + "tbDroppedTreatment.csv")   &&   

		tbExperiencedPyramid.Write(outputPrefix + "tbExperiencedPyramid.csv") &&
		
		activeHouseholdContacts.Write(outputPrefix + "activeHouseholdContacts.csv")
	);
}

int main(int argc, char **argv)
{
	Constants constants {};

	// Initialize some constants that will be passed to each trajectory
	constants["tMax"]           = 365*50;
	constants["periodLength"]   = 365;
	constants["ageGroupWidth"]  = 3;
	constants["startYear"]      = 1990;
	constants["populationSize"] = 10000;

	// Initialize a few default values for parameters that mostly can be 
	// passed through the command line
	const char *householdsFile {"household_structure.csv"};
	int nTrajectories {5};
	long timestamp {std::time(NULL)};

	string folder {""};
	string parameter_sheet {"sampleParams"};

	int pool_size {1};

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
	// -m INTEGER
	// 	    Run in parallel with a pool size of INTEGER.
	int opt;
	while ((opt = getopt(argc, argv, ":t:n:p:y:s:o:m:")) != -1)
	{
		switch (opt)
		{
			case 't':
				nTrajectories = atoi(optarg);
				break;
			case 'n':
				constants["populationSize"] = atoi(optarg);
				break;
			case 'p':
				parameter_sheet = optarg;
				break;
			case 'y':
				constants["tMax"] = 365 * atoi(optarg);
				break;
			case 's':
				timestamp = atol(optarg);
				break;
			case 'm':
				pool_size = atoi(optarg);
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

	// Initialize population, household, survey streams
	string populationHeader = "trajectory,time,hash,age,sex,marital,household,householdHash,offspring,mom,dad,HIV,ART,CD4,TBStatus\n";
	string householdHeader  = "trajectory,time,hash,size,head,spouse,directOffspring,otherOffspring,other\n";
	string deathHeader = "trajectory,time,hash,age,sex,cause,HIV,HIV_date,ART,ART_date,CD4,baseline_CD4\n";
	ofstream populationSurvey(outputPrefix + "populationSurvey.csv", ios_base::out);
	ofstream householdSurvey(outputPrefix + "householdSurvey.csv", ios_base::out);
	ofstream deathSurvey(outputPrefix + "deathSurvey.csv", ios_base::out);
	populationSurvey << populationHeader;
	householdSurvey  << householdHeader;
	deathSurvey      << deathHeader;


	// Initialize the map of simulation parameters
	std::map<string, Param> params{};
	mapShortNames(fileToJSON(string("../params/") + parameter_sheet + string(".json")), params);


	// Thread pool for trajectories, and associated futures
	ThreadPool pool(pool_size);
	vector<future<bool>> results;

	// Mutex lock for data-export critical section
	std::mutex mtx;

	std::vector<long> seeds;
	for (int i = 0; i < nTrajectories; i++)
		seeds.emplace_back(rng.mt_());

	printf("Finished processing arguments and initializing the pool\n");

	for (int i = 0; i < nTrajectories; i++) {
		results.emplace_back(
			pool.enqueue([i, params, constants, householdsFile, seeds, &mtx, &populationSurvey, &householdSurvey, &deathSurvey] {
				printf("#%4d RUNNING\n", i);

				// Initialize a trajectory
				auto traj = TBABM{params, constants, householdsFile, seeds[i]};

				// Run the trajectory and check its' status
				if (!traj.Run()) {
					printf("Trajectory %4d: Run() failed\n", i);
					return false;
				}

				// Acquire a mutex lock for the data-exporting critical section
				mtx.lock();

				// Pass the TBABM object to a function that will export its data
				if (!ExportTrajectory(traj, i, populationSurvey, householdSurvey, deathSurvey)) {
					printf("Trajectory #%4d: ExportTrajectrory(1) failed\n", i);
					mtx.unlock();
					return false;
				}

				// Release the mutex lock and return
				mtx.unlock();

				printf("#%4d FINISHED\n", i);
				return true;
			})
		);
	}

	// Barrier: Wait for each thread to return successfully or unsuccessfully
	// for (auto && result : results)
		// std::cout << result.get() << ' ';
	for (int i = 0; i < nTrajectories; i++) {
		printf("#%4d JOINED: %d\n", i, (int)results[i].get());
	}

	
	std::cout << std::endl;

	if (!WriteData(outputPrefix)) {
		printf("WriteData() failed. Exiting\n");
		exit(1);
	}

	return 0;
}
