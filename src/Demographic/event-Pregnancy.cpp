#include "../../include/TBABM/TBABM.h"

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;


EventFunc TBABM::Pregnancy(Pointer<Individual> mother)
{
	EventFunc ef = 
		[this, mother](double t, SchedulerT scheduler) {
			mother->pregnant = true;

			return true;
		};

	return ef;
}