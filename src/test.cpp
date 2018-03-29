#include <vector>

#include <Normal.h>
#include <Weibull.h>
#include <Uniform.h>
#include <CSVExport.h>
#include <RNG.h>

#include "../include/TBABM/TBABM.h"

using Distributions = TBABM::Distributions;
using Constants = TBABM::Constants;

using namespace StatisticalDistributions;

using std::vector;

int main(int argc, char const *argv[])
{
	Distributions distributions {};
	Constants constants {};

	distributions["sex"] = std::make_shared<Uniform>(0, 1);

	// The following are inactive:
	distributions["coupleFormsNewHousehold"] = std::make_shared<Normal>(1,1);
	distributions["timeToBirth"]             = std::make_shared<Normal>(1,1);

	// These are active
	distributions["marriageAgeDifference"]   = std::make_shared<Normal>(7,3.2);
	distributions["marriageDuration"]        = std::make_shared<Weibull>(1.34,12.91);
	distributions["leavingHousehold"]        = std::make_shared<Normal>(365*5,2);
	distributions["timeToLooking"]           = std::make_shared<Exponential>(1,1);

	constants["naturalDeath-0-M"]  = 1./20.;
	constants["naturalDeath-0-F"]  = 1./20.;
	constants["naturalDeath-10-M"] = 1./20.;
	constants["naturalDeath-10-F"] = 1./20.;
	constants["naturalDeath-20-M"] = 1./20.;
	constants["naturalDeath-20-F"] = 1./20.;
	constants["naturalDeath-30-M"] = 1./20.;
	constants["naturalDeath-30-F"] = 1./20.;
	constants["naturalDeath-40-M"] = 1./20.;
	constants["naturalDeath-40-F"] = 1./20.;
	constants["naturalDeath-50-M"] = 1./20.;
	constants["naturalDeath-50-F"] = 1./20.;
	constants["naturalDeath-60-M"] = 1./20.;
	constants["naturalDeath-60-F"] = 1./20.;
	constants["naturalDeath-70-M"] = 1./20.;
	constants["naturalDeath-70-F"] = 1./20.;
	constants["naturalDeath-80-M"] = 1./20.;
	constants["naturalDeath-80-F"] = 1./20.;
	constants["naturalDeath-90-M"] = 1./1.1;
	constants["naturalDeath-90-F"] = 1./1.1;
	constants["tMax"] = 365*10;
	constants["periodLength"] = 30;
	constants["ageGroupWidth"] = 10;

	const char *householdsFile = "household_structure.csv";

	int nTrajectories = 5;

	RNG rng(std::time(NULL));

	std::vector<std::shared_ptr<TBABM>> trajectories{};

	for (int i = 0; i < nTrajectories; i++)
		trajectories.push_back(std::make_shared<TBABM>(distributions, constants, householdsFile, rng.mt_()));

	for (int i = 0; i < nTrajectories; i++)
		trajectories[i]->Run();

	TimeSeriesExport<int> births("births.csv");
	TimeSeriesExport<int> deaths("deaths.csv");
	TimeSeriesExport<int> populationSize("populationSize.csv");
	TimeSeriesExport<int> marriages("marriages.csv");
	TimeSeriesExport<int> divorces("divorces.csv");

	using TBABMData = TBABM::TBABMData;
	for (int i = 0; i < nTrajectories; i++) {
		births.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Births));
		deaths.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Deaths));
		populationSize.Add(trajectories[i]->GetData<PrevalenceTimeSeries<int>>(TBABMData::PopulationSize));
		marriages.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Marriages));
		divorces.Add(trajectories[i]->GetData<IncidenceTimeSeries<int>>(TBABMData::Divorces));
	}

	if (births.Write()&&
	deaths.Write()&&
	populationSize.Write()&&
	marriages.Write()&&
	divorces.Write()) {
		printf("everything was written successfully!\n");
	}

	return 0;
}