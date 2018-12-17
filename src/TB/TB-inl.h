#include "../../include/TBABM/IndividualTypes.h"
#include "../../include/TBABM/TB.h"


template <typename T>
TB<T>::~TB(void)
{
	return;
}


template <typename T>
TBStatus
TB<T>::GetTBStatus(void)
{
	printf("TB<T>::GetTBStatus\n");
	return tb_status;
}

template <typename T>
void
TB<T>::RiskReeval(void)
{
	printf("TB<T>::RiskReeval\n");
	return;
}

template <typename T>
void
TB<T>::Investigate(void)
{
	printf("TB<T>::Investigate\n");
	return;
}

// Evaluates the risk of infection according to age,
// sex, year, CD4 count, and household TB presence.
// May schedule TB infection.
// 
// If infection is scheduled, 'InfectionRiskEvaluate'
// will not schedule itself again. If infection is
// not scheduled, then the next 'risk_window' is computed
// from the same properties used to model infection
// risk, and 'InfectionRiskEvaluate' is rescheduled.
// 
// When scheduling an infection, the only StrainType
// supported right now is 'Unspecified'.
template <typename T>
void
TB<T>::InfectionRiskEvaluate(Time)
{
	printf("TB<T>::InfectionRiskEvaluate\n");
	return;
}

// Marks an individual as infected and may or may not
// schedule the beginning of treatment.
// 
// If no treatment, recovery or death is scheduled.
template <typename T>
void
TB<T>::Infect(Time, StrainType)
{
	printf("TB<T>::Infect\n");
	return;
}

// Marks an individual as having begun treatment.
// Decides if they will complete treatment, or drop
// out. Schedules either event.
template <typename T>
void
TB<T>::TreatmentBegin(Time)
{
	printf("TB::TreatmentBegin\n");
	return;
}


template <typename T>
void
TB<T>::TreatmentDropout(Time)
{
	printf("TB::TreatmentDropout\n");
	return;
}

template <typename T>
void
TB<T>::TreatmentComplete(Time)
{
	printf("TB::TreatmentComplete\n");
	return;
}


template <typename T>
void
TB<T>::Recovery(Time, RecoveryType)
{
	printf("TB::Recovery\n");
	return;
}