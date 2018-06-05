#include <vector>

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
	constants["ageGroupWidth"] = 10;
	constants["startYear"] = 1990;

	const char *householdsFile = "household_structure.csv";

	int nTrajectories = 1;

	RNG rng(std::time(NULL));

	std::vector<std::shared_ptr<TBABM>> trajectories{};
	std::map<string, Param> params{};

	mapShortNames(fileToJSON("../params/sampleParams.json"), params);

	for (int i = 0; i < nTrajectories; i++)
		trajectories.push_back(std::make_shared<TBABM>(params, constants, householdsFile, rng.mt_()));

	for (int i = 0; i < nTrajectories; i++)
		trajectories[i]->Run();

	TimeSeriesExport<int> births("../output/births.csv");
	TimeSeriesExport<int> deaths("../output/deaths.csv");
	TimeSeriesExport<int> populationSize("../output/populationSize.csv");
	TimeSeriesExport<int> marriages("../output/marriages.csv");
	TimeSeriesExport<int> divorces("../output/divorces.csv");
	TimeSeriesExport<int> households("../output/householdsCount.csv");
	PyramidTimeSeriesExport pyramid("../output/populationPyramid.csv");

	using TBABMData = TBABM::TBABMData;
	for (int i = 0; i < nTrajectories; i++) {
		births.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Births));
		deaths.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Deaths));
		populationSize.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::PopulationSize));
		marriages.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Marriages));
		divorces.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Divorces));
		households.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Households));
	}

	pyramid.Add(trajectories[0]->GetData<IncidencePyramidTimeSeries>(TBABMData::Pyramid));

	if (births.Write()&&
	deaths.Write()&&
	populationSize.Write()&&
	marriages.Write()&&
	divorces.Write()&&
	pyramid.Write()&&
	households.Write()) {
		printf("everything was written successfully!\n");
	} else {
		printf("Somethihg didn't write correctly\n");
	}

	return 0;
}