#include "../../include/TBABM/TBABM.h"

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;


EventFunc TBABM::Pregnancy(Pointer<Individual> mother, 
						   Pointer<Individual> father)
{
	EventFunc ef = 
		[this, mother, father] (double t, SchedulerT scheduler) {

			if (!mother || mother->dead)
				return true;

			if (mother->pregnant)
				return true;

			mother->pregnant = true;

			Schedule(t + 9*30, Birth(mother, father));

			return true;
		};

	return ef;
}