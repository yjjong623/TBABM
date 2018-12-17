#include "../../include/TBABM/TBABM.h"
#include <Uniform.h>
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

// Algorithm S3: Match making
EventFunc TBABM::Matchmaking(void)
{
	EventFunc ef = 
		[this](double t, SchedulerT scheduler) {

			int msSize = maleSeeking.size();
			int fsSize = femaleSeeking.size();
			int marriages = 0;

			auto it = maleSeeking.begin();
			for (; it != maleSeeking.end(); it++) {
				if (femaleSeeking.size() == 0) {
					break;
				}

				auto weights = std::vector<long double>{};

				int j = 0;
				double denominator = 0;

				for (auto it2 = femaleSeeking.begin(); j < 100 && it2 != femaleSeeking.end(); it2++) {
					assert(*it2);
					assert(*it);
					int maleAge = (t - (*it)->birthDate) / 365;
					int femaleAge = (t - (*it2)->birthDate) / 365;
					int ageDifference = std::abs(maleAge-femaleAge);

					long double weight = params["marriageAgeDifference"].pdf(ageDifference);
					denominator += weight;
					weights.push_back(weight);

					j++;
				}


				for (size_t i = 0; i < weights.size(); i++)
					weights[i] = weights[i] / denominator;

				auto wifeDist = Empirical(weights);
				int wifeIdx = wifeDist(rng.mt_);
				auto wife = femaleSeeking.begin();

				assert(wifeIdx < femaleSeeking.size());

				std::advance(wife, wifeIdx);
				assert(*wife);


				Schedule(t, Marriage(*it, *wife));
				marriages++;

				femaleSeeking.erase(wife);
			}

			// Erase males who have been matched to a female
			maleSeeking.erase(maleSeeking.begin(), it);

			Schedule(t + 30, Matchmaking());

			return true;
		};


	return ef;
}