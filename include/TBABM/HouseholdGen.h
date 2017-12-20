#include <cstdio>
#include <iostream>

#include "Household.h"
#include "Individual.h"

using Individual::HouseholdPosition;
using Individual::Sex;

class HouseholdGen {
public:
	using Distributions = map<string, StatisticalDistribution>;
	using Constants = map<string, int>;

	struct MicroIndividual {
		HouseholdPosition role;
		Sex sex;
		int age;
	};

	using MicroFamily = std::vector<MicroIndividual>;

	Pointer<Household> GetHousehold(int hid);

	HouseholdGen(const Distributions d, const Constants c, const char *file) : 
		distributions(d), constants(c), file(file) {
			FILE *ifile = fopen(file, "r");
			int c;
			int lines = 2;

			// Skip first two lines
			while ((c = getc(ifile)) != EOF && c != '\n');
			while ((c = getc(ifile)) != EOF && c != '\n');

			while (!feof(ifile)) {
				lines++;

				fscanf(ifile, "%i", &c);
				if (getc(ifile) != ',') {
					cerr << "Line #" << lines << " has a noninteger number of people. Treating as 0" << endl;
					c = 0;
				}

				MicroFamily f;
				while (c--) {
					MicroIndividual idv;
					int r, s; // Hold temporary values

					HouseholdPosition role;
					Sex sex;
					int age;

					fscanf(ifile, "%i,%i,%i,", &r, &s, &age);

					switch (r) {
						case (1):  role = HouseholdPosition::Head; break;
						case (2):  role = HouseholdPosition::Spouse; break;
						case (3):  role = HouseholdPosition::Offspring; break;
						case (4):  role = HouseholdPosition::Offspring; break;
						case (5):  role = HouseholdPosition::Other; break;
						case (6):  role = HouseholdPosition::Other; break;
						case (7):  role = HouseholdPosition::Other; break;
						case (8):  role = HouseholdPosition::Other; break;
						case (9):  role = HouseholdPosition::Other; break;
						case (10): role = HouseholdPosition::Other; break;
						case (11): role = HouseholdPosition::Other; break;
						case (12): role = HouseholdPosition::Other; break;
						case (13): role = HouseholdPosition::Other; break;
						case (98): role = HouseholdPosition::Other; break;
						case (99): role = HouseholdPosition::Other; break;
						default:   role = HouseholdPosition::Other;
					}

					switch (s) {
						case(1): sex = Sex::Male;   break;
						case(2): sex = Sex::Female; break;
						default: sex = Sex::Male;
					}

					f.push_back(idv);

					while((c = getc(ifile)) != EOF && c != '\n');
				}

				families.push_back(f);
			}
		};
private:
	std::vector<MicroFamily> families;

	const Distributions distributions;
	const Constants constants;	
	const char *file;
}