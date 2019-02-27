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
	constants["periodLength"] = 365;
	constants["ageGroupWidth"] = 3;
	constants["startYear"] = 1990;

	const char *householdsFile = "household_structure.csv";

	int nTrajectories = 5;

	auto timestamp = argc == 1 ? std::time(NULL) : atol(argv[1]);

	RNG rng(timestamp);

	std::ofstream seedLog;
	seedLog.open("../output/seed_log.txt", std::ios_base::app);
	seedLog << timestamp << std::endl;


	string outputPrefix = "../output/" + to_string(timestamp) + "_";

	std::vector<std::shared_ptr<TBABM>> trajectories{};
	std::map<string, Param> params{};

	mapShortNames(fileToJSON("../params/sampleParams.json"), params);

	string populationHeader = "trajectory,time,hash,age,sex,marital,household,householdHash,offspring,mom,dad,HIV,ART,CD4,TBStatus\n";
	string householdHeader  = "trajectory,time,hash,size,head,spouse,directOffspring,otherOffspring,other\n";
	string deathHeader = "trajectory,time,hash,age,sex,cause,HIV,HIV_date,ART,ART_date,CD4,baseline_CD4\n";
	auto populationSurvey = std::make_shared<ofstream>(outputPrefix + "populationSurvey.csv", ios_base::out);
	auto householdSurvey  = std::make_shared<ofstream>(outputPrefix + "householdSurvey.csv", ios_base::out);
	auto deathSurvey      = std::make_shared<ofstream>(outputPrefix + "deathSurvey.csv", ios_base::out);
	*populationSurvey << populationHeader;
	*householdSurvey  << householdHeader;
	*deathSurvey      << deathHeader;

	for (int i = 0; i < nTrajectories; i++) {
		auto traj = std::make_shared<TBABM>(params, 
											constants, 
											householdsFile, 
											rng.mt_(),
											populationSurvey,
											householdSurvey,
											deathSurvey);
		trajectories.push_back(traj);
	}

	for (int i = 0; i < nTrajectories; i++)
		trajectories[i]->Run();

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
	TimeSeriesExport<int> tbTreatmentEnd(outputPrefix + "tbTreatmentEnd.csv");
	TimeSeriesExport<int> tbTreatmentDropout(outputPrefix + "tbTreatmentDropout.csv");
	TimeSeriesExport<int> tbInTreatment(outputPrefix + "tbInTreatment.csv");
	TimeSeriesExport<int> tbCompletedTreatment(outputPrefix + "tbCompletedTreatment.csv");
	TimeSeriesExport<int> tbDroppedTreatment(outputPrefix + "tbDroppedTreatment.csv");

	PyramidTimeSeriesExport pyramid(outputPrefix + "populationPyramid.csv");
	PyramidTimeSeriesExport deathPyramid(outputPrefix + "deathPyramid.csv");
	PyramidTimeSeriesExport hivInfectionsPyramid(outputPrefix + "hivInfectionsPyramid.csv");
	PyramidTimeSeriesExport hivPositivePyramid(outputPrefix + "hivPositivePyramid.csv");

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

		success &= tbInfections.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::TBInfections));
		success &= tbConversions.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::TBConversions));
		success &= tbRecoveries.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::TBRecoveries));

		success &= tbInfectionsHousehold.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::TBInfectionsHousehold));
		success &= tbInfectionsCommunity.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::TBInfectionsCommunity));

		success &= tbSusceptible.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::TBSusceptible));
		success &= tbInfected.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::TBInfected));
		success &= tbLatent.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::TBLatent));
		success &= tbInfectious.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::TBInfectious));
		success &= tbTreatmentBegin.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::TBTreatmentBegin));
		success &= tbTreatmentEnd.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::TBTreatmentEnd));
		success &= tbTreatmentDropout.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::TBTreatmentDropout));
		success &= tbInTreatment.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::TBInTreatment));
		success &= tbCompletedTreatment.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::TBCompletedTreatment));
		success &= tbDroppedTreatment.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::TBDroppedTreatment));

		success &= pyramid.Add(trajectories[i]->GetData<IncidencePyramidTimeSeries>(TBABMData::Pyramid));
		success &= deathPyramid.Add(trajectories[i]->GetData<IncidencePyramidTimeSeries>(TBABMData::DeathPyramid));
		success &= hivInfectionsPyramid.Add(trajectories[i]->GetData<IncidencePyramidTimeSeries>(TBABMData::HIVInfectionsPyramid));
		success &= hivPositivePyramid.Add(trajectories[i]->GetData<PrevalencePyramidTimeSeries>(TBABMData::HIVPositivePyramid));
	}


	if (!success) {
		printf("An attempt to add a data source to the CSV exporter failed. Quitting.\n");
		return 0;
	}

	if (births.Write()               &&
		deaths.Write()               &&
		populationSize.Write()       &&
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
		tbTreatmentEnd.Write()       &&
		tbTreatmentDropout.Write()   &&
		tbInTreatment.Write()        &&
		tbCompletedTreatment.Write() &&
		tbDroppedTreatment.Write()       ) {
		printf("Everything was written successfully!\n");
	} else {
		printf("Somethihg didn't write correctly\n");
	}

	return 0;
}