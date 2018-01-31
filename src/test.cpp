#include <Normal.h>
#include <Uniform.h>

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
	distributions["marriageAgeDifference"]   = std::make_shared<Normal>(1,1);
	distributions["leavingHousehold"]        = std::make_shared<Normal>(1,1);
	distributions["timeToLooking"]           = std::make_shared<Normal>(1,1);
	distributions["coupleFormsNewHousehold"] = std::make_shared<Normal>(1,1);
	distributions["marriageDuration"]        = std::make_shared<Normal>(1,1);
	distributions["timeToBirth"]             = std::make_shared<Normal>(1,1);

	// These are active
	constants["naturalDeath-0-M"] = 1./20.;
	constants["naturalDeath-0-F"] = 1./20.;
	constants["tMax"] = 365*100;
	constants["periodLength"] = 365;

	const char *householdsFile = "household_structure.csv";

	TBABM sim(distributions, constants, householdsFile);

	sim.Run();

	return 0;
}