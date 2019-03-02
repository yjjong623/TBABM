#include <iostream>

#include "../../include/TBABM/TB.h"
#include "../../include/TBABM/utils/termcolor.h"

template <typename T>
void
TB<T>::Log(Time t, string msg)
{
	std::cout << termcolor::on_green << "[" << std::left \
			  << std::setw(12) << name << std::setw(5) << std::right \
			  << (int)t << "] " \
			  << msg << termcolor::reset << std::endl;
}

template <typename T>
TBStatus
TB<T>::GetTBStatus(Time t)
{
	return tb_status;
}


template <typename T>
void
TB<T>::SetHouseholdCallbacks(function<void(Time)>   progression, 
							 function<void(Time)>   recovery,
							 function<double(void)> householdPrevalence)
{
	if (progression && recovery && householdPrevalence) {
		ProgressionHandler    = progression;
		RecoveryHandler       = recovery;
		HouseholdTBPrevalence = householdPrevalence;
	}
}

template <typename T>
void
TB<T>::ResetHouseholdCallbacks(void)
{
	ProgressionHandler    = nullptr;
	RecoveryHandler       = nullptr;
	HouseholdTBPrevalence = nullptr;
}



template <typename T>
void
TB<T>::Investigate(void)
{
	printf("TB<T>::Investigate\n");
	return;
}

template <typename T>
void
TB<T>::HandleDeath(Time t)
{
	switch(tb_status) {
		case (TBStatus::Susceptible):
			data.tbSusceptible.Record(t, -1); break;
		case (TBStatus::Latent):
			data.tbLatent.Record(t, -1);
			data.tbInfected.Record(t, -1); break;
		case (TBStatus::Infectious):
			data.tbInfectious.Record(t, -1); break;
			data.tbInfected.Record(t, -1); break;
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