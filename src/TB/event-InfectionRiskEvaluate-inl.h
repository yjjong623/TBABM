#include <Exponential.h>
#include <Bernoulli.h>

#include "../../include/TBABM/TB.h"

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
#define SMALLFLOAT 0.0000000001

bool
TB::InfectionRiskEvaluate_impl(Time t, int risk_window_local, shared_p<TB> l_ptr)
{
	if (risk_window_local != risk_window_id)
		return true;

	if (!AliveStatus())
		return true;

	// Can't get infected if you're not susceptible
	if (tb_status != TBStatus::Susceptible)
		return true;

	// std::cout << termcolor::on_blue << "[" << std::left \
	// 		  << std::setw(12) << name << std::setw(5) << std::right \
	// 		  << (int)ts << "] InfectionRiskEvaluate" << termcolor::reset << " " \
	// 		  << std::setw(2) << AgeStatus(ts) << " years old, " \
	// 		  << std::setw(4) << (int)CD4Count(ts) << " cells/ml, " \
	// 		  << (int)(HIVStatus() == HIVStatus::Positive) << " HIV status, " \
	// 		  << GlobalTBPrevalence(ts) << " g_prev, " \
	// 		  << HouseholdTBPrevalence(ts) << " h_prev" << termcolor::reset << std::endl;

	bool init_infection {false};
	long double p_init_infection {0.12};

	long double risk_global     {GlobalTBPrevalence(t) * params["TB_risk_global"].Sample(rng)};
	long double risk_household  {ContactHouseholdTBPrevalence(tb_status) * params["TB_risk_household"].Sample(rng)};

	// Time to infection for global and local. If any of these risks are zero,
	// change to a really small number since you can't sample 0-rate exponentials
	long double tti_global     {Exponential(risk_global    > 0 ? risk_global     : SMALLFLOAT)(rng.mt_)};
	long double tti_household  {Exponential(risk_household > 0 ? risk_household  : SMALLFLOAT)(rng.mt_)};

	// First risk window of the simulation, we infect a bunch of people
	if (t < risk_window && Bernoulli(p_init_infection)(rng.mt_))
		init_infection = true;

	// If they do get infected, this is what the infection source is. Note: During 'seeding'
	// (0 < t < risk_window) all infection is from the community
	Source infection_source {tti_global < tti_household ? Source::Global : Source::Household};
	if (init_infection)
		infection_source = Source::Global;

	long double master_infection_time {init_infection ? 0.0 : std::min(tti_global, tti_household)};

	if (master_infection_time < risk_window/365) // Will this individual be infected now?
		if (!params["TB_rapidprog_risk"].Sample(rng)) { // Will they become latently infected, or progress rapidly?
			InfectLatent(t + master_infection_time, infection_source, StrainType::Unspecified);
		} else {
			InfectInfectious(t + master_infection_time, infection_source, StrainType::Unspecified);
		}
	else {
		InfectionRiskEvaluate(t + risk_window, risk_window_local, l_ptr);
	}

	return true;	
}

void
TB::InfectionRiskEvaluate(Time t, int risk_window_local, shared_p<TB> l_ptr)
{
	if (!l_ptr)
		l_ptr = GetLifetimePtr();

	auto lambda = [this, risk_window_local, l_ptr] (auto ts, auto) -> bool {
		return InfectionRiskEvaluate_impl(ts, risk_window_local, l_ptr);
	};

	eq.QuickSchedule(t, lambda);
	return;
}

void
TB::InfectionRiskEvaluate_initial(Time t, int risk_window_local)
{
	auto lambda = [this, risk_window_local] (auto ts, auto) -> bool {
		return InfectionRiskEvaluate_impl(ts, risk_window_local);
	};

	eq.QuickSchedule(t, lambda);
	return;
}