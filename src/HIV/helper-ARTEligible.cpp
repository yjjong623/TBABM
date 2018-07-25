#include "../../include/TBABM/TBABM.h"
#include <Bernoulli.h>

using EventFunc  = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using namespace StatisticalDistributions;

using HIVStatus = Individual::HIVStatus;
using Sex       = Individual::Sex;

bool TBABM::ARTEligible(int t, Pointer<Individual> idv)
{
	if (!idv || idv->dead || idv->hivStatus == HIVStatus::Negative)
		return false;

	int year      = constants["startYear"] + (int)t/365;
	bool coin     = Bernoulli(0.5)(rng.mt_); // Flip a fair coin
	double m_30   = params["HIV_m_30"].Sample(rng);
	int CD4       = idv->CD4count(t, m_30);
	bool pregnant = idv->pregnant;

	if (1990 <= year && year < 2009)
		return CD4 < 200;
	
	else if (2009 == year)
		return (CD4 < 200) \
			|| (coin && pregnant && 200 <= CD4 && CD4 < 350);

	else if (2010 == year)
		return (CD4 < 200)
			|| (pregnant && 200 <= CD4 && CD4 < 350);
	
	else if (2011 == year)
		return CD4 < 350;
	
	else if (2012 <= year)
		return CD4 < 350;

	return false;
}