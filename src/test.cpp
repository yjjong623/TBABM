#include <Normal.h>
#include <Weibull.h>
#include <Uniform.h>
#include <CSVExport.h>

#include "../include/TBABM/TBABM.h"

using Distributions = TBABM::Distributions;
using Constants = TBABM::Constants;

using namespace StatisticalDistributions;

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
	distributions["timeToLooking"]           = std::make_shared<Exponential>(1/50.,1);

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
	constants["tMax"] = 365*100;
	constants["periodLength"] = 30;
	constants["ageGroupWidth"] = 10;

	const char *householdsFile = "household_structure.csv";

	TBABM sim1(distributions, constants, householdsFile);
	TBABM sim2(distributions, constants, householdsFile);
	TBABM sim3(distributions, constants, householdsFile);
	TBABM sim4(distributions, constants, householdsFile);
	TBABM sim5(distributions, constants, householdsFile);

	sim1.Run();
	// sim2.Run();
	// sim3.Run();
	// sim4.Run();
	// sim5.Run();

	// TimeSeriesExport<int> births("births.csv");
	// births.Add(&sim1.births);
	// births.Add(&sim2.births);
	// births.Add(&sim3.births);
	// births.Add(&sim4.births);
	// births.Add(&sim5.births);
	// TimeSeriesExport<int> deaths("deaths.csv");
	// deaths.Add(&sim1.deaths);
	// deaths.Add(&sim1.deaths);
	// deaths.Add(&sim1.deaths);
	// deaths.Add(&sim1.deaths);
	// deaths.Add(&sim1.deaths);
	// TimeSeriesExport<int> populationSize("populationSize.csv");
	// populationSize.Add(&sim1.populationSize);
	// populationSize.Add(&sim1.populationSize);
	// populationSize.Add(&sim1.populationSize);
	// populationSize.Add(&sim1.populationSize);
	// populationSize.Add(&sim1.populationSize);
	// TimeSeriesExport<int> marriages("marriages.csv");
	// marriages.Add(&sim1.marriages);
	// marriages.Add(&sim1.marriages);
	// marriages.Add(&sim1.marriages);
	// marriages.Add(&sim1.marriages);
	// marriages.Add(&sim1.marriages);

	// births.Write();
	// deaths.Write();
	// populationSize.Write();
	// marriages.Write();

	return 0;
}