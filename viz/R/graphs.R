library(tidyverse)
source("R/utils_csv.R")
source("R/graphs_general.R")
source("R/graphs_population.R")
source("R/graphs_household.R")
source("R/graphs_death.R")
source("R/graphs_HIV.R")
source("R/graphs_TB.R")

GetLatestPrefix <- function(location) paste(location, FindLatestTimestamp(location), "_", sep="")
GetPrefix <- function(location, timestamp) paste(location, timestamp, "_", sep="")

deathRatePyramid <- function (Loader)
  left_join(Loader("deathPyramid"), 
            Loader("populationPyramid"), 
            by=c("period", "trajectory", "age group", "category")) %>%
        mutate(value = 1000*value.x/(value.y + 1)) %>%
        AveragedPyramid_impl("Age group", "Deaths/1,000/yr")

CreateGraphCatalog <- function(outputLocation, run="latest") {
  
  Loader           <- switch(run, "latest" = CSVLoaderGen(GetLatestPrefix(outputLocation)), 
                             CSVLoaderGen(GetPrefix(outputLocation, run)))
  ps <- Loader("populationSurvey")
  hs <- Loader("householdSurvey")
  ds <- Loader("deathSurvey")

  list(
    Loader                = Loader,
    ps                    = ps,
    hs                    = hs,
    ds                    = ds,
    
    deathRatePyramid      = function() deathRatePyramid(Loader),
    populationPyramid     = function() Pyramid(Loader, "populationPyramid", "Count", "Age group"),
    deathPyramid          = function() Pyramid(Loader, "deathPyramid"),
    populationProportion  = function() ProportionPyramid(Loader, "populationPyramid", "Age group", "Proportion"),
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
    tbInfections          = function() Graph(Loader, "tbInfections", "Time (years)", "Infections/year") + ggtitle("Infections"),
    tbConversions         = function() Graph(Loader, "tbConversions", "Time (years)", "Conversions/year") + ggtitle("Conversions Latent->Active"),
    tbRecoveries          = function() Graph(Loader, "tbRecoveries", "Time (years)", "Recoveries/year") + ggtitle("Recoveries"),
    
    tbTransmission        = function() tbTransmission(Loader),
    
    # At end there are supposedly 8k Susceptible, but probably some of them are dead. Check
    tbSusceptible         = function() Graph(Loader, "tbSusceptible", "Time (years)", ""),
    tbInfected            = function() Graph(Loader, "tbInfected", "Time (years)", ""),
    tbLatent              = function() Graph(Loader, "tbLatent", "Time (years)", ""), # Good, seems like most people's TB never progresses
    tbInfectious          = function() Graph(Loader, "tbInfectious", "Time (years)", ""), # This is fixed now
    
    # Seems good
    tbTreatmentBegin      = function() Graph(Loader, "tbTreatmentBegin", "Time (years)", ""),
    tbTreatmentEnd        = function() Graph(Loader, "tbTreatmentEnd", "Time (years)", ""),
    tbTreatmentDropout    = function() Graph(Loader, "tbTreatmentDropout", "Time (years)", ""),
    
    # Might be screwy
    tbInTreatment         = function() Graph(Loader, "tbInTreatment", "Time (years)", "Individuals in treatment"),
    tbCompletedTreatment  = function() Graph(Loader, "tbCompletedTreatment", "Time (years)", "Individuals completing treatment"),
    tbDroppedTreatment    = function() Graph(Loader, "tbDroppedTreatment", "Time (years)", "Individuals dropping out"),
    
    
    
    # ageAtInfection        = function() Hist(Loader, "meanSurvivalTimeNoART", "age.at.infection", "Age at Infection"),
    # meanSurvivalTimeNoART = function() Hist(Loader, "meanSurvivalTimeNoART", "years.lived", "Years lived with HIV, no ART"),
    
    meanSize              = function() meanSize(hs),
    nHouseholds           = function() nHouseholds(hs),
    sizeHist              = function() sizeHist(hs) + ggtitle("Histogram of household sizes, by years since simulation start"),
    
    childrenVsOthers      = function() childrenVsOthers(ps),
    childrenPerWoman      = function() childrenPerWoman(ps),
    livingAlone           = function() livingAlone(ps),
    maritalStatus         = function() maritalStatus(ps)
  )
}

outputLocation <- "/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/"
cat <- CreateGraphCatalog(outputLocation)
  # geom_hline(yintercept = 19.61) + BIRTHRATE
  # geom_hline(yintercept = 16.99) + DEATHRATE
  # geom_hline(yintercept = 3) + MARRIAGERATE
  # geom_hline(yintercept = 0.4) + DIVORCERATE