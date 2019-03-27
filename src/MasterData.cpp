#include "../include/TBABM/MasterData.h"

MasterData::MasterData(int tMax, int pLength, std::vector<int> ageBreaks) :
	births(         "births",          0, tMax, periodLength),
	deaths(         "deaths",          0, tMax, periodLength),
	marriages(      "marriages",       0, tMax, periodLength),
	divorces(       "divorces",        0, tMax, periodLength),
	populationSize( "populationSize",     tMax, periodLength),
	singleToLooking("singleToLooking", 0, tMax, periodLength),

	hivNegative(   "hivNegative",    tMax, periodLength),
	hivPositive(   "hivPositive",    tMax, periodLength),
	hivPositiveART("hivPositiveART", tMax, periodLength),
	hivPositivePyramid("hivPositive pyramid", 0, tMax, pLength, 2, ageBreaks),

	hivInfectionsPyramid("hivInfections pyramid", 0, tMax, pLength, 2, ageBreaks),
	hivInfections(  "hivInfections", 0,   tMax, periodLength),
	hivDiagnosed(   "hivDiagnosed",       tMax, periodLength),
	hivDiagnosedVCT("hivDiagnosedVCT",    tMax, periodLength),
	hivDiagnosesVCT("hivDiagnosesVCT", 0, tMax, periodLength),

	tbInfections( "tbInfections",  0, tMax, periodLength),
	tbIncidence(  "tbIncidence"  , 0, tMax, periodLength),
	tbRecoveries( "tbRecoveries",  0, tMax, periodLength),

	tbInfectionsHousehold( "tbInfectionsHousehold",  0, tMax, periodLength),
	tbInfectionsCommunity( "tbInfectionsCommunity",  0, tMax, periodLength),

	tbSusceptible("tbSusceptible", tMax, periodLength),
	tbLatent(     "tbLatent",      tMax, periodLength),
	tbInfectious( "tbInfectious",  tMax, periodLength),

	tbExperienced("tbExperienced", tMax, periodLength),
	tbExperiencedPyr("tbExperiencedPyr", 0, tMax, periodLength, 2, ageBreaks),

	tbTreatmentBegin(   "tbTreatmentBegin",   0, tMax, periodLength),
	tbTreatmentBeginHIV("tbTreatmentBeginHIV",0, tMax, periodLength),
	tbTreatmentEnd(     "tbTreatmentEnd",     0, tMax, periodLength),
	tbTreatmentDropout( "tbTreatmentDropout", 0, tMax, periodLength),

	tbInTreatment(       "tbInTreatment",        tMax, periodLength),
	tbCompletedTreatment("tbCompletedTreatment", tMax, periodLength),
	tbDroppedTreatment(  "tbDroppedTreatment",   tMax, periodLength),

	activeHouseholdContacts("activeHouseholdContacts"),

	pyramid("Population pyramid", 0, tMax, pLength, 2, ageBreaks),
	deathPyramid("Death pyramid", 0, tMax, pLength, 2, ageBreaks),
	householdsCount("households", 0, tMax, pLength)
{
	printf("All data initialized\n");
}

IndividualInitData
MasterData::GenIndividualInitData(void) const
{
	return CreateIndividualInitData(
		tbInfections,
		tbIncidence,
		tbRecoveries,
		tbInfectionsHousehold,
		tbInfectionsCommunity,
		tbSusceptible,
		tbLatent,
		tbInfectious,
		tbExperienced,
		tbExperiencedPyr,
		tbTreatmentBegin,
		tbTreatmentBeginHIV,
		tbTreatmentEnd,
		tbTreatmentDropout,
		tbInTreatment,
		tbCompletedTreatment,
		tbDroppedTreatment,
		activeHouseholdContacts
	);
}