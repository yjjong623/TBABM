#include <string>
#include "Individual.h"
#include "Household.h"

using IPt = Pointer<Individual>;
using HPt = Pointer<Household>;

string Ihash(IPt idv); // Hash of individual
string Hhash(HPt hh);  // Hash of household

string age(IPt idv, int t); // Age (years)
string sex(IPt idv); // Sex ("male"/"female")

string marital(IPt idv); // "single"/"married"/"divorced"/"looking"
string numChildren(IPt idv);
string mom(IPt idv); // "dead"/"alive"
string dad(IPt idv); // "dead"/"alive"

string causeDeath(DeathCause cause_death); // "HIV"/"natural"

string HIV(IPt idv); // bool
string HIV_date(IPt idv); // in days
string ART(IPt idv); // bool
string ART_date(IPt idv); // in days
string CD4(IPt idv, double t, double m_30);
string ART_baseline_CD4(IPt idv, double m_30);