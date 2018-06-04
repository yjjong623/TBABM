#include <JSONParameterize.h>
#include <DataFrame.h>
#include <Param.h>

#include <RNG.h>
#include <Exponential.h>

using namespace SimulationLib;

void test1(void) {
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
		sum += d.getValue(0, 0, 25, rng); // rate=0.9614
	}

	printf("mean=%Lf\n", sum/n);
}

void test2(void) {
	string fname = "../params/time to natural death.json";
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
		sum += d.getValue(2040, 1, 101, rng); // rate=0.140521955
	}

	printf("mean=%Lf\n", sum/n); // should be around 7.1
}

void test3(void) {
	string fname = "../params/sampleParams.json";
	json j = JSONImport::fileToJSON(fname);

	for (auto it = j.begin(); it != j.end(); it++) {
		std::cout << (*it)["description"] << "\n";
		Param p = parameterize(*it);
		std::cout << p.getDescription() << "\n";
	}
}

int main(int argc, char const *argv[])
{
	test1();
	test2();
	test3();
	
	return 0;
}