#include "../../include/TBABM/TBABM.h"
#include "../../include/TBABM/Individual.h"

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;



EventFunc TBABM::Survey(void)
{
	using Sex = Individual::Sex;
	using MS  = Individual::MarriageStatus;
	using IPt = Pointer<Individual>;
	using HPt = Pointer<Household>;

	// Get unique hash for each individual
	auto Ihash = [] (IPt idv) -> string { return to_string(std::hash<IPt>()(idv)); };
	auto Hhash = [] (HPt hh)  -> string { return to_string(std::hash<HPt>()(hh));  };

	// Translate age to string
	auto age = [] (IPt idv, auto t) -> string { return to_string(idv->age(t)); };

	// Translate scoped enum to string
	auto sex = [] (IPt idv) -> string { return idv->sex == Sex::Male ? "male" : "female"; };

	// Translate scoped enum to string
	auto marital = [] (IPt idv) -> string {
		switch (idv->marriageStatus) {
			case MS::Single:   return "single";   break;			
			case MS::Married:  return "married";  break;
			case MS::Divorced: return "divorced"; break;
			case MS::Looking:  return "looking";  break;
			default:		   return "UNSUPPORTED MARITAL";
		}
	};

	auto numChildren = [] (IPt idv) { return to_string(idv->numOffspring()); };

	auto mom = [] (IPt idv) { return (!idv->mother || idv->mother->dead) ? "dead" : "alive"; };
	auto dad = [] (IPt idv) { return (!idv->father || idv->father->dead) ? "dead" : "alive"; };


	EventFunc ef = 
		[&] (double t, SchedulerT scheduler) {
			string buf;
			string s = ",";

			///////////////////////////////////////////////////////
			// Population survey
			///////////////////////////////////////////////////////
			for (auto it = population.begin(); it!= population.end(); it++) {
				auto idv = *it;

				if (!idv || idv->dead)
					continue;

				string line = to_string(seed) + s \
							+ to_string(t) + s \
							+ Ihash(idv) + s \
							+ age(idv, t) + s \
							+ sex(idv) + s \
							+ marital(idv) + s \
							+ to_string(households[idv->householdID]->size()) + s \
							+ numChildren(idv) + s \
							+ mom(idv) + s
							+ dad(idv)
							+ "\n";

				buf += line;
			}

			*populationSurvey << buf;
			buf.clear();

			///////////////////////////////////////////////////////
			// Household survey
			///////////////////////////////////////////////////////
			for (auto it = households.begin(); it != households.end(); it++) {
				auto hh = it->second;

				if (!hh || hh->size() == 0) continue;

				auto head =   hh->head;
				auto spouse = hh->spouse;

				int directOffspring {0};
				int otherOffspring  {0};

				for (auto i : hh->offspring)
					if (i->father == head || i->mother == head || \
						i->father == spouse || i->mother == spouse)
						directOffspring += 1;
					else
						otherOffspring += 1;

				string line = to_string(seed) 		      + s \
							+ to_string(t)    		      + s \
							+ Hhash(hh)		  		      + s \
							+ to_string(hh->size())       + s \
							+ to_string(!!hh->head)       + s \
							+ to_string(!!hh->spouse)     + s \
							+ to_string(directOffspring)  + s \
							+ to_string(otherOffspring)   + s \
							+ to_string(hh->other.size())     \
							+ "\n";

				buf += line;
			}

			*householdSurvey << buf;

			Schedule(t + 365, Survey());

			return true;
		};

	return ef;
}

// data interested in:
// for each person:
//  - seed of trajectory they're in
//  - time
// 	- hash
// 	- age
// 	- sex
// 	- single, married, divorced, or looking
// 	- household size
// 	- number of children
// 	- mom alive?
// 	- dad alive?
// 	
// 	for each household:
// 	- trajectory
// 	- time
// 	- hash
// 	- size
// 	- head
// 	- spouse
// 	- directOffspring
// 	- otherOffspring
// 	- other