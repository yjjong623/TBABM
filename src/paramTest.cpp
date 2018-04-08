#include <JSONParameterize.h>
#include <DataFrame.h>
#include <Param.h>

#include <RNG.h>
#include <Exponential.h>

using namespace SimulationLib;

long double sample (RNG &rng)
{
	Exponential e(1,0);
	return e.Sample(rng);
}

int main(int argc, char const *argv[])
{
	string fname = "../params/time to first birth.json";
	json j = JSONImport::fileToJSON(fname);

	RNG rng(std::time(nullptr));

	DataFrameFile d {j};
	long double sum = 0;
	size_t n = 1000000;

	printf("timebracket=%f\n", d.getTimeBracket());
	printf("agebracket=%f\n", d.getAgeBracket());
	printf("timecats=%d\n", d.getTimeCats());
	printf("agecats=%d\n", d.getAgeCats());
	printf("timestart=%f\n", d.getTimeStart());
	printf("agestart=%f\n", d.getAgeStart());

	for (size_t i = 0; i < n; i++) {
		sum += d.getValue(0, 0, 25, rng); // rate=0.2638
	}

	printf("mean=%Lf\n", sum/n);
	
	return 0;
}