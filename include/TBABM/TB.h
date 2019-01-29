#include <functional>
#include <map>
#include <string>

#include <RNG.h>
#include <EventQueue.h>
#include <Param.h>
#include <DataFrame.h>
#include <IncidenceTimeSeries.h>

#include "IndividualTypes.h"
#include "TBTypes.h"

using std::function;
using std::string;

using namespace SimulationLib;

template <typename T>
using Pointer = std::shared_ptr<T>;

using Params = std::map<std::string, Param>;

using EQ = EventQueue<double, bool>;

template <typename SexT>
class TB
{
public:
	using Time        = double;
	using Alive       = bool;
	using Age         = int;
	using Year        = int;
	using CD4         = double;
	using HouseholdTB = bool;

	TB(int current_time,
	   string name,
	   SexT sex,
	   EQ& event_queue,
	   RNG& rng,
	   TBData initData,
	   function<Age(Time)> AgeStatus,
	   function<Alive(void)> AliveStatus,
	   function<CD4(Time)> CD4Count,
	   function<HouseholdTB(void)> HouseholdStatus,
	   function<HIVStatus(void)> HIVStatus,

	   function<void(Time)> DeathHandler,
	   
	   double risk_window, // unit: [days]
	   
	   // Pointer<IncidenceTimeSeries<int>> TBIncidence,

	   // Pointer<Params> params,
	   // Pointer<map<string, DataFrameFile>> fileData,

	   TBStatus tb_status = TBStatus::Susceptible) :
		name(name),
		sex(sex),
		event_queue(event_queue),
		rng(rng),
		data(initData),
		AgeStatus(AgeStatus),
		AliveStatus(AliveStatus),
		CD4Count(CD4Count),
		HouseholdStatus(HouseholdStatus),
		HIVStatus(HIVStatus),
		DeathHandler(DeathHandler),
		risk_window(risk_window),
		// TBIncidence(TBIncidence),
		// params(params)
		// fileData(fileData),
		tb_status(tb_status),
		tb_treatment_status(TBTreatmentStatus::None),
		risk_window_id(0)
	{
		InfectionRiskEvaluate(current_time);

		data.tbSusceptible.Record(current_time, +1);
	}

	~TB(void);

	TBStatus GetTBStatus(Time);
	void RiskReeval(Time);
	void Investigate(void);

private:

	void Log(Time, string);

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
	bool InfectionRiskEvaluate(Time, int local_risk_window = 0);

	// Marks an individual as latently infected. May transition
	// to infectous TB through reactivation.
	// 
	// NOTE: Reinfection not implemented
	void InfectLatent(Time, StrainType);

	// Marks an individual as infectous and may or may not
	// schedule the beginning of treatment.
	// 
	// If no treatment, recovery or death is scheduled.
	bool InfectInfectious(Time, StrainType);

	// Marks an individual as having begun treatment.
	// Decides if they will complete treatment, or drop
	// out. Schedules either event.
	void TreatmentBegin(Time);

	// Sets tb_treatment_status to Incomplete
	void TreatmentDropout(Time);

	// Sets tb_treatment_status to Complete, and 
	// calls Recovery
	void TreatmentComplete(Time);

	// Marks the individual as recovered, and sets
	// status tb_status to Latent
	void Recovery(Time, RecoveryType);

	double risk_window; // unit: [days]
	int risk_window_id;

	TBStatus tb_status;
	TBTreatmentStatus tb_treatment_status;


	// All of these are from the constructor
	string name;
	SexT sex;

	EQ& event_queue;
	RNG& rng;
	TBData data;

    function<Age(Time)> AgeStatus;
    function<Alive(void)> AliveStatus;
    function<CD4(Time)> CD4Count;
    function<HouseholdTB(void)> HouseholdStatus;
    function<HIVStatus(void)> HIVStatus;
 
    function<void(Time)> DeathHandler;
        
    // Pointer<IncidenceTimeSeries<int>> TBIncidence;
 
    // Pointer<Params> params;
    // Pointer<map<string, DataFrameFile>> fileData;
};