#include <Poisson.h>
#include <UniformDiscrete.h>
#include <RNG.h>

#include "../../include/TBABM/TBABM.h"

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::pair;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using Sex = Individual::Sex;

using namespace StatisticalDistributions;

EventFunc TBABM::ExogenousBirth(void)
{
	using IPt = Pointer<Individual>;
	using Couple = pair<IPt, IPt>;

	auto sampleSpouse = 
		[this] (void) -> Pointer<Couple> {
			
			// Get the length of the population array. Note this is different
			// from the number of individuals (at least as large).
			size_t populationArrayLength = population.size();

			// Sampler for population index
			auto idxGen	= UniformDiscrete(populationArrayLength - 1);

			// The empty std::pair which will eventually hold the couple
			auto couple = std::make_shared<Couple>(IPt{}, IPt{});

			// Exit flag for loop
			bool flag = false;
			int nTries = 0;

			while (!flag) {
				// "Index" of first couple member (but not really because
				//   the underlying data structure is a set)
				long firstIdx = idxGen(rng.mt_);

				size_t i = 0;
				auto it = population.begin();
				for(; it != population.end(); i++, it++)
					if (i == firstIdx)
						break;

				couple->first = *it;
				
				if (couple->first)
					couple->second = couple->first->spouse;

				if (couple->first  && !couple->first->dead &&\
					couple->second && !couple->second->dead)
					flag = true;

				nTries++;
			}

			return couple;
		};

	EventFunc ef = 
		[this, sampleSpouse](double t, SchedulerT scheduler) {

			// Grab the current population size
			int n = populationSize(t);

			// Grab annual birth rate of the population
			double annualBirthRate = params["annualBirthRate"].Sample(rng);

			// Calculate the monthly expected number of births
			double rate = n * annualBirthRate * 1./12;

			// Initialize Poisson distribution where mu is the expected number
			//   of births in the month
			auto dist = Poisson(rate);

			// Sample to get number of births. Poisson is a continuous distribution,
			// must cast to integer to get exact number of births
			int nBirths = static_cast<int>(dist(rng.mt_));


			for (int i = 0; i < nBirths; i++) {

				// Find a random couple from the population
				auto couple = sampleSpouse();

				auto mother = couple->first->sex == Sex::Female ? couple->first  : couple->second;
				auto father = couple->first->sex == Sex::Female ? couple->second : couple->first;

				// Immediately schedule birth
				Schedule(t, Birth(mother, father));
			}

			Schedule(t + 30, ExogenousBirth());

			return true;
		};

	return ef;
}