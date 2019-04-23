library(tidyverse)
library(purrr)

source("R/utils_csv.R")
source("R/graphs_general.R")
source("R/graphs_population.R")
source("R/graphs_household.R")
source("R/graphs_death.R")
source("R/graphs_HIV.R")
source("R/graphs_TB.R")
source("R/graphs_grid.R")
source("R/multiplot.R")

GetLatestPrefix <- function(location) paste(location, FindLatestTimestamp(location), "_", sep="")
GetPrefix <- function(location, timestamp) paste(location, timestamp, "_", sep="")

deathRatePyramid <- function (Loader) {
  left_join(Loader("deathPyramid"), 
            Loader("pyramid"), 
            by=c("period", "trajectory", "age group", "category")) %>%
        mutate(value = 1000*value.x/(value.y + 1)) %>%
        AveragedPyramid_impl("Age group", "Deaths/1,000/yr")
}

CreateGraphCatalog <- function(outputLocation, run="latest") {
  
  Loader           <- switch(run, "latest" = CSVLoaderGen(GetLatestPrefix(outputLocation)), 
                             CSVLoaderGen(GetPrefix(outputLocation, run)))

  list(
    Loader                = Loader,
    ps                    = function() Loader("populationSurvey"),
    hs                    = function() Loader("householdSurvey"),
    ds                    = function() Loader("deathSurvey"),
    
    deathRatePyramid      = function() deathRatePyramid(Loader),
    populationPyramid     = function() Pyramid(Loader, "pyramid", "Count", "Age group"),
    deathPyramid          = function() Pyramid(Loader, "deathPyramid"),
    populationProportion  = function() ProportionPyramid(Loader, "pyramid", "Age group", "Proportion"),
    birthRate             = function() GraphRate(Loader, "births", "populationSize", 1000, "Time (years)", "Birth rate (births/1,000/year)") + ggtitle("Birth rate over 50 years"),
    deathRate             = function() GraphRate(Loader, "deaths", "populationSize", 1000, "Time (years)", "Death rate (deaths/1,000/year)") + ggtitle("Death rate over 50 years"),
    births                = function() Graph(Loader, "births", "Time (years)", "Births"),
    deaths                = function() Graph(Loader, "deaths", "Time (years)", "Deaths"),
    
    marriageRate          = function() GraphRate(Loader, "marriages", "populationSize", 1000, "Time (years)", "Marriage rate (marriages/1e3/year)"),
    divorceRate           = function() GraphRate(Loader, "divorces", "populationSize", 1000, "Time (years)", "Divorce rate (divorces/1e3/year)"),
    populationSize        = function() Graph(Loader, "populationSize", "Time (years)", "Population size") + ggtitle("Population size"),
    
    hivDiagnosed          = function() Graph(Loader, "hivDiagnosed", "Time (years)", "HIV Diagnosed"),
    hivDiagnosedVCT       = function() Graph(Loader, "hivDiagnosedVCT", "Time (years)", "HIV Diagnosed – VCT"),
    hivDiagnosesVCT       = function() Graph(Loader, "hivDiagnosesVCT", "Time (years)", "HIV Diagnoses – VCT"),
    
    hivInfections         = function() Graph(Loader, "hivInfections", "Time (years)", "HIV Infections"),
    hivInfectionsPyramid  = function() AveragedPyramid(Loader, "hivInfectionsPyramid", "Age Group", "Individuals infected") + ggtitle("HIV infections by age group, absolute count"),
    hivInfectionsRatePyr  = function() left_join(Loader("hivInfectionsPyramid"), 
                                                 Loader("populationPyramid"), 
                                                 by=c("period", "trajectory", "age group", "category")) %>%
                                       mutate(value = 100000*value.x/(value.y + 1)) %>%
                                       AveragedPyramid_impl("Age group", "Infections/100,000/yr") + ggtitle("HIV infection rate by age group"),
    hivNegative           = function() Graph(Loader, "hivNegative", "Time (years)", "HIV Negative"),
    hivPositive           = function() Graph(Loader, "hivPositive", "Time (years)", "HIV Positive"),
    hivPositiveART        = function() Graph(Loader, "hivPositiveART", "Time (years)", "HIV Positive + ART"),
    
    hivCD4s               = function(size=100) CD4counts(ps, size),
    hivSurvivalWithART    = function() hivSurvivalWithART(ds),
    
    hivCD4Decline         = function(n_samples) hivCD4Decline(ps, ds, n_samples),
    hivSurvivalNoART      = function() hivSurvivalNoART(ds),
    
    # All seems to check out, but are the rates realistic?
    tbInfections          = function() Graph(Loader, "tbInfections", "Time (years)", "Infections/year") + ggtitle("Infections S->L"),
    tbIncidence           = function() Graph(Loader, "tbIncidence", "Time (years)", "Conversions/year") + ggtitle("Incidence Latent->Active"),
    tbRecoveries          = function() Graph(Loader, "tbRecoveries", "Time (years)", "Recoveries/year") + ggtitle("Recoveries"),
    
    tbTransmission        = function() tbTransmission(Loader),
    
    # At end there are supposedly 8k Susceptible, but probably some of them are dead. Check
    tbSusceptible         = function() Graph(Loader, "tbSusceptible", "Time (years)", ""),
    tbLatent              = function() Graph(Loader, "tbLatent", "Time (years)", ""), # Good, seems like most people's TB never progresses
    tbInfectious          = function() Graph(Loader, "tbInfectious", "Time (years)", ""), # This is fixed now
    
    # Seems good
    tbTreatmentBegin      = function() Graph(Loader, "tbTreatmentBegin", "Time (years)", ""),
    tbTreatmentBeginHIV   = function() GraphRate(Loader, "tbTreatmentBeginHIV", "tbTreatmentBegin", 1, "Time (years)", "Fraction"),
    tbTreatmentEnd        = function() Graph(Loader, "tbTreatmentEnd", "Time (years)", ""),
    tbTreatmentDropout    = function() Graph(Loader, "tbTreatmentDropout", "Time (years)", ""),
    
    # Might be screwy
    tbInTreatment         = function() Graph(Loader, "tbInTreatment", "Time (years)", "Individuals in treatment"),
    tbCompletedTreatment  = function() Graph(Loader, "tbCompletedTreatment", "Time (years)", "Individuals completing treatment"),
    tbDroppedTreatment    = function() Graph(Loader, "tbDroppedTreatment", "Time (years)", "Individuals dropping out"),
    
    tbEvents              = function() tbEvents(Loader),  
    tbOverview            = function() tbOverview(Loader),
    tbAll                 = function() tbAll(Loader),
    tbTreatment           = function() tbTreatment(Loader),
    
    # ageAtInfection        = function() Hist(Loader, "meanSurvivalTimeNoART", "age.at.infection", "Age at Infection"),
    # meanSurvivalTimeNoART = function() Hist(Loader, "meanSurvivalTimeNoART", "years.lived", "Years lived with HIV, no ART"),
    
    meanSize              = function() meanSize(hs()),
    nHouseholds           = function() nHouseholds(hs()),
    sizeHist              = function() sizeHist(hs()) + ggtitle("Histogram of household sizes, by years since simulation start"),
    
    childrenVsOthers      = function() childrenVsOthers(ps()),
    childrenPerWoman      = function() childrenPerWoman(ps()),
    livingAlone           = function() livingAlone(ps()),
    maritalStatus         = function() maritalStatus(ps()),
    
    demographicGrid       = function(cols=4, rows=3) DemographicGrid(Loader, cols=cols, rows=rows),
    hivGrid               = function(cols=4, rows=3) HIVGrid(Loader, cols=cols, rows=rows),
    tbGrid                = function(cols=4, rows=3) TBGrid(Loader, cols=cols, rows=rows)
  )
}

# insFOIs <- function() {
#   runs <- c(1552405442, 1552405546, 1552405640, 1552405734, 1552405813)
#   household <- c(50, 5, 0.5, 0.05, 0.005)
#   global <- c(0.005, 0.05, 0.5, 5, 50)
#   datas <- c("tbInfectionsHousehold", "tbInfectionsCommunity")
#   
#   process <- function(run, household, global) {
#       Loader <- CSVLoaderGen(GetPrefix(outputLocation, as.character(run)))
#       
#       map(datas, ~mutate(Loader(.), type=., 
#                                     household=household, 
#                                     global=global,
#                                     seed=run)) %>%
#         bind_rows()
#   }
#     
#     pmap(list(runs, household, global), ~process(..1, ..2, ..3)) %>%
#       bind_rows() %>%
#       ggplot(aes(period, value, color=type, group=interaction(trajectory, type, seed))) +
#         geom_line() +
#         coord_cartesian(ylim=c(0, 300)) +
#         theme_classic() +
#         theme(legend.position = "top") +
#         theme(legend.background = element_rect(fill="lightblue",
#                                                size=0.5, linetype="solid", 
#                                                colour ="darkblue")) +
#         scale_color_discrete(name="Infection source", breaks=c("tbInfectionsCommunity", "tbInfectionsHousehold"), labels=c("Global/Community", "Household")) +
#         labs(x="Time (years)", y="Infections/year", color="Infection source") +
#         facet_grid(vars(household, global), labeller = "label_both")
# }
# 
# insFOIs()
  # geom_hline(yintercept = 19.61) + BIRTHRATE
  # geom_hline(yintercept = 16.99) + DEATHRATE
  # geom_hline(yintercept = 3) + MARRIAGERATE
  # geom_hline(yintercept = 0.4) + DIVORCERATE
  