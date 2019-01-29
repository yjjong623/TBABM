#include "../../include/TBABM/IndividualTypes.h"

IndividualHandlers 
CreateIndividualHandlers(function<void(Pointer<Individual>, int, DeathCause)> death)
{
	return {death};
}

IndividualSimContext
CreateIndividualSimContext(
	int current_time, 
	EQ& event_queue, 
	RNG &rng,
	map<string, DataFrameFile>& fileData,
	Params& params)
{
	return {current_time, 
			event_queue, 
		    rng,
		    fileData,
		    params};
}
