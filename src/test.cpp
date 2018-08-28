#include <vector>
#include <iostream>
#include <string>

#include <Normal.h>
#include <Weibull.h>
#include <Uniform.h>
#include <CSVExport.h>
#include <RNG.h>
#include <JSONParameterize.h>
#include <JSONImport.h>

#include "../include/TBABM/TBABM.h"

using Constants = TBABM::Constants;

using namespace StatisticalDistributions;
using namespace SimulationLib::JSONImport;

using std::vector;

int main(int argc, char const *argv[])
{
	Constants constants {};

	constants["tMax"] = 365*50;
	constants["periodLength"] = 30;
	constants["ageGroupWidth"] = 5;
	constants["startYear"] = 1990;

	const char *householdsFile = "household_structure.csv";

	int nTrajectories = 1;

	RNG rng(std::time(NULL));

	std::vector<std::shared_ptr<TBABM>> trajectories{};
	std::map<string, Param> params{};

	mapShortNames(fileToJSON("../params/sampleParams.json"), params);

	string populationHeader = "trajectory,time,hash,age,sex,marital,household,offspring,mom,dad\n";
	string householdHeader  = "trajectory,time,hash,size,head,spouse,directOffspring,otherOffspring,other\n";

	auto populationSurvey = std::make_shared<ofstream>("../output/populationSurvey.csv", ios_base::out);
	auto householdSurvey  = std::make_shared<ofstream>("../output/householdSurvey.csv", ios_base::out);
	*populationSurvey << populationHeader;
	*householdSurvey  << householdHeader;

	for (int i = 0; i < nTrajectories; i++) {
		auto traj = std::make_shared<TBABM>(params, 
											constants, 
											householdsFile, 
											rng.mt_(),
											populationSurvey,
											householdSurvey);
		trajectories.push_back(traj);
	}

	for (int i = 0; i < nTrajectories; i++)
		trajectories[i]->Run();

	TimeSeriesExport<int> births("../output/births.csv");
	TimeSeriesExport<int> deaths("../output/deaths.csv");
	TimeSeriesExport<int> populationSize("../output/populationSize.csv");
	TimeSeriesExport<int> marriages("../output/marriages.csv");
	TimeSeriesExport<int> divorces("../output/divorces.csv");
	TimeSeriesExport<int> households("../output/householdsCount.csv");
	TimeSeriesExport<int> singleToLooking("../output/singleToLooking.csv");

	TimeSeriesExport<int> hivNegative("../output/hivNegative.csv");
	TimeSeriesExport<int> hivPositive("../output/hivPositive.csv");
	TimeSeriesExport<int> hivPositiveART("../output/hivPositiveART.csv");

	TimeSeriesExport<int> hivInfections("../output/hivInfections.csv");
	TimeSeriesExport<int> hivDiagnosed("../output/hivDiagnosed.csv");
	TimeSeriesExport<int> hivDiagnosedVCT("../output/hivDiagnosedVCT.csv");
	TimeSeriesExport<int> hivDiagnosesVCT("../output/hivDiagnosesVCT.csv");

	PyramidTimeSeriesExport pyramid("../output/populationPyramid.csv");
	PyramidTimeSeriesExport deathPyramid("../output/deathPyramid.csv");

	using TBABMData = TBABM::TBABMData;

	bool success = true;
	for (int i = 0; i < nTrajectories; i++) {
		success &= births.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Births));
		success &= deaths.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Deaths));
		success &= populationSize.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::PopulationSize));
		success &= marriages.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Marriages));
		success &= divorces.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Divorces));
		success &= households.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Households));
		success &= singleToLooking.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::SingleToLooking));
		
		success &= hivNegative.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::HIVNegative));
		success &= hivPositive.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::HIVPositive));
		success &= hivPositiveART.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::HIVPositiveART));

		success &= hivInfections.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::HIVInfections));
		success &= hivDiagnosed.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::HIVDiagnosed));
		success &= hivDiagnosedVCT.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::HIVDiagnosedVCT));
		success &= hivDiagnosesVCT.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::HIVDiagnosesVCT));

		success &= pyramid.Add(trajectories[i]->GetData<IncidencePyramidTimeSeries>(TBABMData::Pyramid));
		success &= deathPyramid.Add(trajectories[i]->GetData<IncidencePyramidTimeSeries>(TBABMData::DeathPyramid));
	}


	if (!success) {
		printf("An attempt to add a data source to the CSV exporter failed. Quitting.\n");
		return 0;
	}

	if (births.Write()          &&
		deaths.Write()          &&
		populationSize.Write()  &&
		marriages.Write()       &&
		divorces.Write()        &&
		singleToLooking.Write() &&
		pyramid.Write()         &&
		deathPyramid.Write()    &&
		households.Write()      &&
		hivNegative.Write()     &&
		hivPositive.Write()     &&
		hivPositiveART.Write()  &&
		hivInfections.Write()   &&
		hivDiagnosed.Write()    &&
		hivDiagnosedVCT.Write() &&
		hivDiagnosesVCT.Write() ) {
		printf("Everything was written successfully!\n");
	} else {
		printf("Somethihg didn't write correctly\n");
	}

	return 0;
}