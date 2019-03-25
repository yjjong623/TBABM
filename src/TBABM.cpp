#include "../include/TBABM/TBABM.h"
#include <Uniform.h>
#include <Empirical.h>
#include <cassert>
#include <fstream>
#include <iostream>

using std::vector;

using EventFunc = TBABM::EventFunc;
using SchedulerT = EventQueue<double,bool>::SchedulerT;

using namespace StatisticalDistributions;

template <>
PrevalenceTimeSeries<int>
TBABM::GetData<PrevalenceTimeSeries<int>>(TBABMData field)
{
    switch(field) {
        case TBABMData::PopulationSize:  return populationSize; break;
 
        case TBABMData::HIVNegative:     return hivNegative; break;
        case TBABMData::HIVPositive:     return hivPositive; break;
        case TBABMData::HIVPositiveART:  return hivPositiveART; break;
 
        case TBABMData::HIVDiagnosed:    return hivDiagnosed; break;
        case TBABMData::HIVDiagnosedVCT: return hivDiagnosedVCT; break;

        case TBABMData::TBSusceptible:   return tbSusceptible; break;
        case TBABMData::TBLatent:        return tbLatent; break;
        case TBABMData::TBInfectious:    return tbInfectious; break;

        case TBABMData::TBExperienced:   return tbExperienced; break;

        case TBABMData::TBInTreatment:   return tbInTreatment; break;
        case TBABMData::TBCompletedTreatment:
                                         return tbCompletedTreatment; break;
        case TBABMData::TBDroppedTreatment:
                                         return tbDroppedTreatment; break;
 
        default:                         
            throw std::invalid_argument("Asked for a PrevalenceTimeSeries that doesn't exist");
    }
}

template <>
IncidenceTimeSeries<int>
TBABM::GetData<IncidenceTimeSeries<int>>(TBABMData field)
{
    switch(field) {
        case TBABMData::Marriages:       return marriages; break;
        case TBABMData::Births:          return births; break;
        case TBABMData::Deaths:          return deaths; break;
        case TBABMData::Divorces:        return divorces; break;
        case TBABMData::Households:      return householdsCount; break;
        case TBABMData::SingleToLooking: return singleToLooking; break;

        case TBABMData::HIVInfections:   return hivInfections; break;
        case TBABMData::HIVDiagnosesVCT: return hivDiagnosesVCT; break;

        case TBABMData::TBInfections:    return tbInfections; break;
        case TBABMData::TBIncidence:     return tbIncidence; break;
        case TBABMData::TBRecoveries:    return tbRecoveries; break;

        case TBABMData::TBInfectionsHousehold: return tbInfectionsHousehold; break;
        case TBABMData::TBInfectionsCommunity: return tbInfectionsCommunity; break;

        case TBABMData::TBTreatmentBegin:return tbTreatmentBegin; break;
        case TBABMData::TBTreatmentBeginHIV:return tbTreatmentBeginHIV; break;
        case TBABMData::TBTreatmentEnd:  return tbTreatmentEnd; break;
        case TBABMData::TBTreatmentDropout:
                                         return tbTreatmentDropout; break;

        default:
            throw std::invalid_argument("Asked for an IncidenceTimeSeries that doesn't exist");
    }
}

template <>
IncidencePyramidTimeSeries
TBABM::GetData<IncidencePyramidTimeSeries>(TBABMData field)
{
    switch(field) {
        case TBABMData::Pyramid:              return pyramid; break;
        case TBABMData::DeathPyramid:         return deathPyramid; break;
        case TBABMData::HIVInfectionsPyramid: return hivInfectionsPyramid; break;
        default:
            throw std::invalid_argument("Asked for a IncidencePyramidTimeSeries that doesn't exist");
    }
}

template <>
PrevalencePyramidTimeSeries
TBABM::GetData<PrevalencePyramidTimeSeries>(TBABMData field)
{
    switch(field) {
        case TBABMData::HIVPositivePyramid: return hivPositivePyramid; break;
        case TBABMData::TBExperienced:      return tbExperiencedPyr; break;
        default:
            throw std::invalid_argument("Asked for a PrevalencePyramidTimeSeries that doesn't exist");
    }
}

template <>
DiscreteTimeStatistic
TBABM::GetData<DiscreteTimeStatistic>(TBABMData field)
{
    switch(field) {
        case TBABMData::ActiveHouseholdContacts: return activeHouseholdContacts; break;
        default:
            throw std::invalid_argument("Asked for a DiscreteTimeStatistic that doesn't exist");
    }
}

bool
TBABM::WriteSurveys(ofstream& ps, 
                    ofstream& hs, 
                    ofstream& ds)
{
    if ((ps << populationSurvey) &&
        (hs << householdSurvey) &&
        (ds << deathSurvey))
        return true;
    else
        return false;
}

bool TBABM::Run(void)
{
    CreatePopulation(0, constants["populationSize"]);
    Schedule(1, Matchmaking());
    Schedule(1, UpdatePyramid());
    Schedule(1, UpdateHouseholds());
    Schedule(1, ARTGuidelineChange());
    Schedule(1, Survey());

    // Schedule(1, ExogenousBirth());

    int events_processed {0};

    while (!eq.Empty()) {
        auto e = eq.Top();
        assert(e);
        
        if (e->t > constants["tMax"])
            break;

        eq.Pop();
        e->run();
        events_processed += 1;
    }

    // Pop off all the events that were greater than tMax
    while (!eq.Empty())
        eq.Pop();

    printf("Events processed: %d\n", events_processed);

    // printf("Making individual\n");
    // auto household = householdGen.GetHousehold(1000000,0,rng);
    // printf("Household generated\n");
    // printf("Household size: %d\n", household->size());
    // printf("HEAD Use count without extra ptr: %ld\n", household->head.use_count());
    // auto head = household->head;
    // printf("HEAD Use count with extra ptr: %ld\n", household->head.use_count());
    // printf("HOUSEHOLD Use count before household reset: %ld\n", household.use_count());
    // household.reset(); // Now, the only pointer to the household should be the one we have
    // printf("HEAD Use count after household reset: %ld\n", head.use_count());
    // // assert(head.unique());

    births.Close();
    deaths.Close();
    marriages.Close();
    populationSize.Close();
    divorces.Close();
    singleToLooking.Close();

    pyramid.Close();
    deathPyramid.Close();
    hivInfectionsPyramid.Close();
    hivPositivePyramid.Close();
    householdsCount.Close();

    hivNegative.Close();
    hivPositive.Close();
    hivPositiveART.Close();

    hivDiagnosed.Close();
    hivDiagnosedVCT.Close();

    hivInfections.Close();
    hivDiagnosesVCT.Close();

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

    return true;
}

void TBABM::Schedule(int t, EventQueue<>::EventFunc ef)
{
    auto f = eq.MakeScheduledEvent(t, ef);
    eq.Schedule(f);
    return;
}

void TBABM::PurgeReferencesToIndividual(weak_p<Individual> host_w,
                                        weak_p<Individual> idv_w)
{
    // Return if host or individual do not exist
    auto host = host_w.lock();
    auto idv = idv_w.lock();
    if (!host || !idv)
        return;

    // Reset pointers for spouse, mother, father (rough equivalent of setting
    // pointers to NULL)
    if (host->spouse.lock() && host->spouse.lock() == idv)
        host->spouse.reset();
    if (host->mother.lock() && host->mother.lock() == idv)
        host->mother.reset();
    if (host->father.lock() && host->father.lock() == idv)
        host->father.reset();

    // Erase 'idv' from offspring
    for (auto it = host->offspring.begin(); it != host->offspring.end(); it++)
        if ((*it).lock() == idv) {
            host->offspring.erase(it);
            break;
        }

    // Erase 'idv' from list of people 'idv' has lived with before
    for (auto it = host->livedWithBefore.begin(); it != host->livedWithBefore.end(); it++)
        if ((*it).lock() == idv) {
            it = host->livedWithBefore.erase(it);
            break;
        }
}

void TBABM::DeleteIndividual(weak_p<Individual> idv_w)
{
    auto idv = idv_w.lock();
    assert(idv);

    for (size_t i = 0; i < idv->livedWithBefore.size(); i++)
        PurgeReferencesToIndividual(idv->livedWithBefore[i], idv);
}

void TBABM::ChangeHousehold(weak_p<Individual> idv_w, int t, int newHID, HouseholdPosition newRole)
{
    auto idv = idv_w.lock();
    if (!idv)
        return;
    if (idv->dead)
        return;

    // printf("\tChanging household of %ld::%lu\n", idv->householdID, std::hash<Pointer<Individual>>()(idv));
    int oldHID = idv->householdID;

    auto oldHousehold = households[oldHID];
    auto newHousehold = households[newHID];

    assert(idv);
    assert(oldHousehold);
    assert(newHousehold);

    if (oldHousehold != newHousehold)
        oldHousehold->RemoveIndividual(idv, t);
    
    newHousehold->AddIndividual(idv, t, newRole);

    // Clear household if there's nobody left in it
    // if (oldHousehold->size() == 0)
        // households[oldHID].reset();

    return;
}
