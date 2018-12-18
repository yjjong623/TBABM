#include "../../include/TBABM/IndividualTypes.h"
#include "../../include/TBABM/TB.h"

using SchedulerT = EQ::SchedulerT;

template <typename T>
TB<T>::~TB(void)
{
	return;
}


template <typename T>
TBStatus
TB<T>::GetTBStatus(Time t)
{
	printf("[%d] GetTBStatus: ", (int)t);

	switch(tb_status) {
		case (TBStatus::Susceptible) : printf("Susceptible\n"); break;
		case (TBStatus::Latent)      : printf("Latent\n"); break;
		case (TBStatus::Infectious)  : printf("Infectious\n"); break;
		default					     : printf("UNSUPPORTED TB type!\n");
	}

	auto pretend_event = [] (Time t, SchedulerT scheduler) -> bool {
		printf("\t[%d] Pretend, yay!\n", (int)t);
		return true;
	};

	event_queue.QuickSchedule(t, pretend_event);

	return tb_status;
}

template <typename T>
void
TB<T>::RiskReeval(void)
{
	printf("RiskReeval\n");


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
bool
TB<T>::InfectionRiskEvaluate(Time t)
{
	printf("[%d] InfectionRiskEvaluate:\n", (int)t);
		printf("\t%d years old\n", AgeStatus(t));
		printf("\t%d CD4 count\n", (int)CD4Count(t));
		printf("\t%d household status\n", (int)HouseholdStatus());
		printf("\t%d HIV status\n", HIVStatus() == HIVStatus::Positive);

	// event_queue.QuickSchedule(t + risk_window, [this] (auto t, auto sched) -> bool { return InfectionRiskEvaluate(t); });

	return true;
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