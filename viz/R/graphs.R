library(tidyverse)
library(future)
library(purrr)
library(furrr)

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
  ps <- Loader("populationSurvey")
  hs <- Loader("householdSurvey")
  ds <- Loader("deathSurvey")

  list(
    Loader                = Loader,
    ps                    = ps,
    hs                    = hs,
    ds                    = ds,
    
    deathRatePyramid      = function() deathRatePyramid(Loader),
    populationPyramid     = function() Pyramid(Loader, "pyramid", "Count", "Age group"),
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

cat$hivCD4Decline()

cat$deathPyramid()



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
    
    # Determine factor to multiply each value by, useful for 
    # rate-based and per-X-capita data
    if (!("factor") %in% names(item))
      factor <- 1
    else
      factor <- item$factor
    
    # If 'data' isn't a key, use highest-level key as name of 
    # data source. If it is a key, divide the data source named by 
    # the first element in the character vector by the data source
    # named by the second element in the character vector
    if (!("data" %in% names(item)))
      data <- Loader(name)
    else
      data <- JoinAndDivideTimeSeries(Loader(item$data[1]), Loader(item$data[2]), "value", factor)

    if ("calibration" %in% names(item))
      calibration <- geom_line(mapping=aes_string("year", item$calibration),
                               data=calibrationData,
                               inherit.aes = FALSE)
    else
      calibration <- geom_blank()
      
    
    # Produce ggplot2 object which will eventually be passed to 'multiplot'
    mutate(data, title=title, year=period+startYear) %>%
      ggplot(aes(year, value, group=trajectory)) +
        geom_line(color="grey") +
        geom_vline(xintercept=2002, linetype="dashed", color="grey") +
        geom_vline(xintercept=2008, linetype="dashed", color="grey") +
        theme_classic() +
        ylim(0, NA) +
        theme(plot.title=element_text(face="bold", size=10)) +
        labs(x="Year", y=item$y, title=title) +
        calibration
  }
  
  plan(multicore)
  future_map(names(namesMap), lambda)
}

calibrationData <- tibble(
  year = seq(2002, 2008),
  populationChildren = c(10427, 10531, 10637, 10743, 10850, 10959, 11068),
  populationAdults =   c(25903, 26162, 26424, 26688, 26955, 27224, 27497),
  populationAll = populationChildren + populationAdults,
  notifiedTBChildren = c(82, 60, 66, 69, 73, 77, 69),
  notifiedTBNaiveAdults = c(82, 60, 66, 69, 73, 77, 69),
  notifiedTBExperiencedAdults = c(172, 234, 200, 224, 216, 233, 210),
  notifiedTBAdults = notifiedTBNaiveAdults + notifiedTBExperiencedAdults,
  notifiedTBAll = notifiedTBAdults + notifiedTBChildren,
  prevalenceExperiencedAdults = c(0.097, NA, NA, NA, NA, NA, NA),
  prevalenceHIV = c(0.052, NA, NA, NA, NA, NA, NA),
  prevalenceInfectiousNaiveAdults = c(0.051, NA, NA, NA, NA, NA, NA),
  prevalenceInfectiousExperiencedAdults = c(0.0299, NA, NA, NA, NA, NA, NA),
  prevalenceInfectiousAdults = prevalenceInfectiousNaiveAdults +
                               prevalenceInfectiousExperiencedAdults
)

nIdv <- "Number of individuals"
testMap  <- list(tbSusceptible  = list(title="TB-Susceptible Individuals", y=nIdv),
                 tbLatent       = list(title="Latently-infected individuals", y=nIdv), 
                 tbInfectious   = list(title="Actively-infected individuals", y=nIdv),
                 populationSize = list(title="Population Size", y=nIdv, calibration="populationAll"),
                 hivPrevalence  = list(data=c("hivPositive", "populationSize"), title="HIV Prevalence, All Individuals", y="Prevalence (%)", factor=100))

bigMap  <- list(birthRate         = list(data=c("births", "populationSize"), title="Birth rate", y="Births/1000/year", factor=1000),
                deathRate         = list(data=c("deaths", "populationSize"), title="Death rate", y="Deaths/1000/year", factor=1000),
                populationSize    = list(title="Population Size", y=nIdv),
                marriageRate      = list(data=c("marriages", "populationSize"), title="Marriage rate", y="Marriages/1000/year", factor=1000),
                divorceRate       = list(data=c("divorces", "populationSize"), title="Divorce rate", y="Divorces/1000/year", factor=1000),
                hivPrevalence     = list(data=c("hivPositive", "populationSize"), title="HIV Prevalence, All Individuals", y="Prevalence (%)", factor=100),
                hivARTPrevalence  = list(data=c("hivPositiveART", "populationSize"), title="HIV ART Prevalence, All Individuals", y="Prevalence (%)", factor=100),
                tbSusceptible     = list(title="TB-Susceptible Individuals", y=nIdv),
                tbLatent          = list(data=c("tbLatent", "populationSize"), title="Latently-infected individuals", y="Prevalence (%)", factor=100), 
                tbInfectious      = list(data=c("tbInfectious", "populationSize"), title="Actively-infected individuals", y="Prevalence (%)", factor=100),
                tbInfections      = list(title="TB Infections, all individuals", y="Infections/year"),
                tbIncidence       = list(title="TB Incidence, all individuals", y="Active incidence/year"),
                tbRecoveries      = list(title="TB Recoveries, all individuals", y="Recoveries/year"),
                tbTreatmentBegin     = list(title="TB case notifications, all", y="Case notifications/year"),
                tbTreatmentEnd       = list(title="TB Treatment Completion", y="Completions/year"),
                tbTreatmentDropout   = list(title="TB Treatment Dropout", y="Dropouts/year"),
                tbInfectionsHousehold= list(title="Household infections", y="Infections/year"),
                tbInfectionsCommunity= list(title="Community infections", y="Infections/year"))

mimicMap <- list(tbTreatmentBegin     = list(title="Tuberculosis case notifications, all", y="Number of case notifications"),
                 populationSize       = list(title="All individuals", y="Number of individuals"),
                 txIndividuals        = list(data=c("tbCompletedTreatment", "populationSize"), title="Treatment-experienced individuals", y="Proportion of individuals (%)", factor=100),
                 tbTreatmentBegin     = list(title="Tuberculosis case notifications, all", y="Number of case notifications"),
                 populationSize       = list(title="All individuals", y="Number of individuals"),
                 tbInfectious         = list(data=c("tbInfectious", "populationSize"), title="Adults", y="Tuberculosis prevalence (%)", factor=100),
                 tbTreatmentBegin     = list(title="Tuberculosis case notifications, all", y="Number of case notifications"),
                 hivPrevalence        = list(data=c("hivPositive", "populationSize"), title="HIV prevalence, all individuals", y="Prevalence (%)", factor=100),
                 tbInfectious         = list(data=c("tbInfectious", "populationSize"), title="Adults", y="Tuberculosis prevalence (%)", factor=100))


# Current format: {
#   timeSeriesName: {
#     title: STRING (optional), 
#     y: STRING (non-optional),
#     calibration: STRING (must name column in calibrationData)
#   }
# }
#
# OR {
#   title: STRING,
#   data: two-element vector,
#   y: STRING,
#   factor: INT,
#   calibration: STRING (must name column in calibrationData)
# }


reviewLatestModelRun <- function () do.call(multiplot, flatten(list(grid(CreateGraphCatalog(outputLocation)$Loader, testMap, 1990), cols=2)))
reviewLatestModelRun()


do.call(multiplot, flatten(list(grid(cat$Loader, testMap, 1990), cols=4)))
do.call(multiplot, flatten(list(grid(cat$Loader, bigMap, 1990), cols=4)))
do.call(multiplot, flatten(list(grid(cat$Loader, mimicMap, 1990), cols=3)))

insFOIs <- function() {
  runs <- c(1552405442, 1552405546, 1552405640, 1552405734, 1552405813)
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
        coord_cartesian(ylim=c(0, 300)) +
        theme_classic() +
        theme(legend.position = "top") +
        theme(legend.background = element_rect(fill="lightblue",
                                               size=0.5, linetype="solid", 
                                               colour ="darkblue")) +
        scale_color_discrete(name="Infection source", breaks=c("tbInfectionsCommunity", "tbInfectionsHousehold"), labels=c("Global/Community", "Household")) +
        labs(x="Time (years)", y="Infections/year", color="Infection source") +
        facet_grid(vars(household, global), labeller = "label_both")
}

insFOIs()
  # geom_hline(yintercept = 19.61) + BIRTHRATE
  # geom_hline(yintercept = 16.99) + DEATHRATE
  # geom_hline(yintercept = 3) + MARRIAGERATE
  # geom_hline(yintercept = 0.4) + DIVORCERATE



pool_size <- c(1, 2, 4, 8)
times <- c(123.13, 72.73, 56.94, 56.78)
tibble(pool_size=pool_size, times=times, relative_improvement=map()) %>%
  ggplot(aes(pool_size, times)) +
    geom_line() +
    geom_point() +
    lims(x=c(0,8), y=c(0,150))
