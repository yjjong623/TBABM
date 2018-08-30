#include "../../include/TBABM/TBABM.h"
#include <Exponential.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using namespace StatisticalDistributions;
using Sex = Individual::Sex;

void TBABM::InitialMortalityCheck(Pointer<Individual> idv, double t, double dt)
{
	int gender = idv->sex == Sex::Male ? 0 : 1;
	double age = idv->age(t);
	auto startYear = constants["startYear"];

	double timeToDeath = fileData["naturalDeath"].getValue(startYear+(int)t/365, gender, age, rng);

	if (timeToDeath < dt)
		Schedule(t + 365*timeToDeath, Death(idv));

	return;
}