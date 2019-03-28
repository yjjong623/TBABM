#include "../include/TBABM/MasterData.h"

MasterData::MasterData(int tMax, int pLength, std::vector<double> ageBreaks) :
	births(         "births",          0, tMax, pLength),
	deaths(         "deaths",          0, tMax, pLength),
	marriages(      "marriages",       0, tMax, pLength),
	divorces(       "divorces",        0, tMax, pLength),
	populationSize( "populationSize",     tMax, pLength),
	singleToLooking("singleToLooking", 0, tMax, pLength),

	hivNegative(   "hivNegative",    tMax, pLength),
	hivPositive(   "hivPositive",    tMax, pLength),
	hivPositiveART("hivPositiveART", tMax, pLength),
	hivPositivePyramid("hivPositive pyramid", 0, tMax, pLength, 2, ageBreaks),

	hivInfectionsPyramid("hivInfections pyramid", 0, tMax, pLength, 2, ageBreaks),
	hivInfections(  "hivInfections", 0,   tMax, pLength),
	hivDiagnosed(   "hivDiagnosed",       tMax, pLength),
	hivDiagnosedVCT("hivDiagnosedVCT",    tMax, pLength),
	hivDiagnosesVCT("hivDiagnosesVCT", 0, tMax, pLength),

	tbInfections( "tbInfections",  0, tMax, pLength),
	tbIncidence(  "tbIncidence"  , 0, tMax, pLength),
	tbRecoveries( "tbRecoveries",  0, tMax, pLength),

	tbInfectionsHousehold( "tbInfectionsHousehold",  0, tMax, pLength),
	tbInfectionsCommunity( "tbInfectionsCommunity",  0, tMax, pLength),

	tbSusceptible("tbSusceptible", tMax, pLength),
	tbLatent(     "tbLatent",      tMax, pLength),
	tbInfectious( "tbInfectious",  tMax, pLength),

	tbExperienced("tbExperienced", tMax, pLength),
	tbExperiencedPyr("tbExperiencedPyr", 0, tMax, pLength, 2, ageBreaks),

	tbTreatmentBegin(   "tbTreatmentBegin",   0, tMax, pLength),
	tbTreatmentBeginHIV("tbTreatmentBeginHIV",0, tMax, pLength),
	tbTreatmentEnd(     "tbTreatmentEnd",     0, tMax, pLength),
	tbTreatmentDropout( "tbTreatmentDropout", 0, tMax, pLength),

	tbInTreatment(       "tbInTreatment",        tMax, pLength),
	tbCompletedTreatment("tbCompletedTreatment", tMax, pLength),
	tbDroppedTreatment(  "tbDroppedTreatment",   tMax, pLength),

	activeHouseholdContacts("activeHouseholdContacts"),

	pyramid("Population pyramid", 0, tMax, pLength, 2, ageBreaks),
	deathPyramid("Death pyramid", 0, tMax, pLength, 2, ageBreaks),
	householdsCount("households", 0, tMax, pLength)
{
	printf("All data initialized\n");
}

IndividualInitData
MasterData::GenIndividualInitData(void)
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

void
MasterData::Close(void)
{
	births.Close();
	deaths.Close();
	marriages.Close();
	divorces.Close();
	populationSize.Close();
	singleToLooking.Close();
	pyramid.Close();
	deathPyramid.Close();
	householdsCount.Close();

	hivNegative.Close();
	hivInfections.Close();
	hivPositive.Close();
	hivPositiveART.Close();
	hivDiagnosed.Close();
	hivDiagnosedVCT.Close();
	hivDiagnosesVCT.Close();
	hivPositivePyramid.Close();
	hivInfectionsPyramid.Close();

	tbInfections.Close();
	tbIncidence.Close();
	tbRecoveries.Close();
	tbInfectionsHousehold.Close();
	tbInfectionsCommunity.Close();
	tbSusceptible.Close();
	tbLatent.Close();
	tbInfectious.Close();
	tbExperienced.Close();
	tbExperiencedPyr.Close();

	tbTreatmentBegin.Close();
	tbTreatmentBeginHIV.Close();
	tbTreatmentEnd.Close();
	tbTreatmentDropout.Close();
	tbInTreatment.Close();
	tbCompletedTreatment.Close();
	tbDroppedTreatment.Close();

	return;
}