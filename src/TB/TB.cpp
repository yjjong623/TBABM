#include "../../include/TBABM/IndividualTypes.h"

#include "TB-inl.h"
#include "event-RiskReeval-inl.h"
#include "event-InfectionRiskEvaluate-inl.h"
#include "event-InfectLatent-inl.h"
#include "event-InfectInfectious-inl.h"
#include "event-TreatmentBegin-inl.h"
#include "event-TreatmentDropout-inl.h"
#include "event-TreatmentComplete-inl.h"
#include "event-Recovery-inl.h"

template class TB<Sex>;
// Add more supported types here...