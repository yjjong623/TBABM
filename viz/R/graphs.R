library(tidyverse)
source("R/utils_csv.R")
source("R/graphs_general.R")
source("R/graphs_population.R")
source("R/graphs_household.R")
source("R/graphs_death.R")
source("R/graphs_HIV.R")
source("R/graphs_TB.R")
source("R/multiplot.R")

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





cat$tbTreatmentBegin()
cat$tbTreatmentBeginHIV()
cat$tbOverview()
cat$tbEvents()
cat$tbTransmission()

grid <- function(Loader, namesMap, startYear) {
  lambda <- function(name) {
    # 'item' is either a string describing the title, or a list
    # with a 'data' key and a 'name' key
    item <- namesMap[[name]]
    title <- ifelse(is.list(item), item$title, item)
    
    if (!("factor") %in% names(item))
      factor <- 1
    else
      factor <- item$factor
    
    if (!("data" %in% names(item)))
      data <- Loader(name)
    else
      data <- JoinAndDivideTimeSeries(Loader(item$data[1]), Loader(item$data[2]), "value", factor)

    mutate(data, title=title, year=period+startYear) %>%
      ggplot(aes(year, value, group=trajectory)) +
        geom_line(color="grey") +
        geom_vline(xintercept=2002, linetype="dashed", color="grey") +
        geom_vline(xintercept=2008, linetype="dashed", color="grey") +
        theme_classic() +
        theme(plot.title=element_text(face="bold", size=10)) +
        labs(x="Year", y=item$y, title=title)
  }
  
  map(names(namesMap), lambda)
}

nIdv <- "Number of individuals"
testMap  <- list(tbSusceptible  = list(title="TB-Susceptible Individuals", y=nIdv),
                 tbLatent       = list(title="Latently-infected individuals", y=nIdv), 
                 tbInfectious   = list(title="Actively-infected individuals", y=nIdv),
                 populationSize = list(title="Population Size", y=nIdv),
                 hivPrevalence  = list(data=c("hivPositive", "populationSize"), title="HIV Prevalence, All Individuals", y="Prevalence (%)", factor=100))

bigMap  <- list(birthRate         = list(data=c("births", "populationSize"), title="Birth rate", y="Births/1000/year", factor=1000),
                deathRate         = list(data=c("deaths", "populationSize"), title="Death rate", y="Deaths/1000/year", factor=1000),
                populationSize    = list(title="Population Size", y=nIdv),
                marriageRate      = list(data=c("marriages", "populationSize"), title="Marriage rate", y="Marriages/1000/year", factor=1000),
                divorceRate       = list(data=c("divorces", "populationSize"), title="Divorce rate", y="Divorces/1000/year", factor=1000),
                hivPrevalence     = list(data=c("hivPositive", "populationSize"), title="HIV Prevalence, All Individuals", y="Prevalence (%)", factor=100),
                hivARTPrevalence  = list(data=c("hivPositiveART", "populationSize"), title="HIV ART Prevalence, All Individuals", y="Prevalence (%)", factor=100),
                tbSusceptible     = list(title="TB-Susceptible Individuals", y=nIdv),
                tbLatent          = list(title="Latently-infected individuals", y=nIdv), 
                tbInfectious      = list(title="Actively-infected individuals", y=nIdv),
                tbConversions     = list(title="TB Conversions, all individuals", y="Infections/year"),
                tbInfections      = list(title="TB Infections, all individuals", y="Infections/year"),
                tbRecoveries      = list(title="TB Recoveries, all individuals", y="Recoveries/year"))

do.call(multiplot, flatten(list(grid(cat$Loader, testMap, 1990), cols=3)))
do.call(multiplot, flatten(list(grid(cat$Loader, bigMap, 1990), cols=4)))

FOIs <- function() {
  runs <- c(1, 2, 3, 4, 5)
  household <- c(50, 5, 0.5, 0.05, 0.005)
  global <- c(0.005, 0.05, 0.5, 5, 50)
  datas <- c("tbInfectionsHousehold", "tbInfectionsCommunity")
  
  process <- function(run, household, global) {
      Loader <- CSVLoaderGen(GetPrefix(outputLocation, as.character(run)))
      
      map(datas, ~mutate(Loader(.), type=., 
                                    household=household, 
                                    global=global,
                                    seed=run)) %>%
        bind_rows()
    }
    
    pmap(list(runs, household, global), ~process(..1, ..2, ..3)) %>%
      bind_rows() %>%
      ggplot(aes(period, value, color=type, group=interaction(trajectory, type, seed))) +
        geom_line() +
        coord_cartesian(ylim=c(0, 500)) +
        facet_wrap(vars(household, global), labeller = "label_both")
}
  # geom_hline(yintercept = 19.61) + BIRTHRATE
  # geom_hline(yintercept = 16.99) + DEATHRATE
  # geom_hline(yintercept = 3) + MARRIAGERATE
  # geom_hline(yintercept = 0.4) + DIVORCERATE