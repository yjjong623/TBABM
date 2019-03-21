#include <iostream>

#include "../../include/TBABM/TB.h"
#include "../../include/TBABM/utils/termcolor.h"

void
TB::Log(Time t, string msg)
{
    std::cout << termcolor::on_green << "[" << std::left \
              << std::setw(12) << name << std::setw(5) << std::right \
              << (int)t << "] " \
              << msg << termcolor::reset << std::endl;
}

TBStatus
TB::GetTBStatus(Time t)
{
    return tb_status;
}


void
TB::SetHouseholdCallbacks(function<void(Time)>       progression, 
                          function<void(Time)>       recovery,
                          function<double(void)>     householdPrevalence,
                          function<double(TBStatus)> contactHouseholdPrevalence)
{
    assert(progression && 
           recovery && 
           householdPrevalence && 
           contactHouseholdPrevalence);

    ProgressionHandler           = progression;
    RecoveryHandler              = recovery;
    HouseholdTBPrevalence        = householdPrevalence;
    ContactHouseholdTBPrevalence = contactHouseholdPrevalence;
}

void
TB::ResetHouseholdCallbacks(void)
{
    ProgressionHandler    = nullptr;
    RecoveryHandler       = nullptr;
    HouseholdTBPrevalence = nullptr;
}



void
TB::Investigate(void)
{
    printf("TB::Investigate\n");
    return;
}

void
TB::HandleDeath(Time t)
{
    switch(tb_status) {
        case (TBStatus::Susceptible):
            data.tbSusceptible.Record(t, -1); break;

        case (TBStatus::Latent):
            data.tbLatent.Record(t, -1); break;

        case (TBStatus::Infectious):
          // Remember, while people are in treatment, they are 
          // still considered infectious
          if (tb_treatment_status != TBTreatmentStatus::Incomplete)
            data.tbInfectious.Record(t, -1);
          break;

        default: std::cout << "Error: UNSUPPORTED TBStatus!" << std::endl;
    }

    switch(tb_treatment_status) {
        case (TBTreatmentStatus::None): break;
        case (TBTreatmentStatus::Incomplete):
            data.tbInTreatment.Record(t, -1); break;
        case (TBTreatmentStatus::Complete):
            data.tbCompletedTreatment.Record(t, -1); break;
        case (TBTreatmentStatus::Dropout):
            data.tbDroppedTreatment.Record(t, -1); break;
        default: std::cout << "Error: UNSUPPORTED TBTreatmentStatus!" << std::endl;
    }

    return;
}