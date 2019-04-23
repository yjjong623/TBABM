addTimeSeries <- function(a, b) {
  joined <- full_join(a, b, by=c("trajectory", "period"))
  
  joined %>% mutate(value = value.x + value.y)
}

grid <- function(Loaders, runNames, namesMap, startYear) {

  colorMapping <- list(tb="turquoise1", hiv="firebrick1", demographic="grey69")
  
  transparentBg <- theme(
    panel.background = element_rect(fill = "transparent") # bg of the panel
    , plot.background = element_rect(fill = "transparent", color = NA) # bg of the plot
    , panel.grid.major = element_blank() # get rid of major grid
    , panel.grid.minor = element_blank() # get rid of minor grid
    , legend.background = element_rect(fill = "transparent") # get rid of legend bg
    , legend.box.background = element_rect(fill = "transparent") # get rid of legend panel bg
  )
  
  lambda <- function(name) {
    # 'item' is either a string describing the title, or a list
    # with a 'data' key and a 'name' key
    item <- namesMap[[name]]
    title <- ifelse(is.list(item), item$title, item)
    color <- colorMapping[[item$tag]]
    
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
      data <- map2(Loaders, runNames, function(load, runName) {load(name) %>% mutate(runName=runName)}) %>% bind_rows()
    else
      data <- map2(Loaders, runNames, function(load, runName) {JoinAndDivideTimeSeries(load(item$data[1]), load(item$data[2]), "value", factor) %>% mutate(runName=runName)}) %>% bind_rows()
    
    if ("calibration" %in% names(item)) {
      calibration_line <- geom_line(mapping=aes_string("year", item$calibration),
                                    data=calibrationData,
                                    inherit.aes = FALSE)
      calibration_points <- geom_point(mapping=aes_string("year", item$calibration),
                                       data=calibrationData,
                                       inherit.aes=FALSE)
    } else {
      calibration_line <- geom_blank()
      calibration_points <- geom_blank()
    }
    
    if ("guess" %in% names(item)) {
      guess_line <- geom_hline(yintercept = item$guess, color="grey")
    } else {
      guess_line <- geom_blank()
    }
    
    ymin <- ifelse("min" %in% names(item), item$min, 0)
    ymax <- ifelse("max" %in% names(item), item$max, NA)
    
    render_legend <- ifelse(length(runNames) > 1, TRUE, FALSE)
    
    # Produce ggplot2 object which will eventually be passed to 'multiplot'
    mutate(data, title=title, year=period+startYear) %>%
      ggplot(aes(year, value, group=trajectory, color=runName)) +
      geom_line(color=color) +
      geom_vline(xintercept=2002, linetype="dashed", color="grey") +
      geom_vline(xintercept=2008, linetype="dashed", color="grey") +
      theme_classic() +
      theme(plot.title=element_text(face="bold", size=10)) +
      transparentBg +
      coord_cartesian(ylim=c(ymin, ymax)) +
      guides(color=render_legend) +
      labs(x="Year", y=item$y, title=title) +
      calibration_line +
      guess_line +
      calibration_points
      # theme_void()
  }
  
  map(names(namesMap), lambda)
}

calibrationData <- tibble(
  year = seq(2002, 2008),
  populationChildren = c(10427, 10531, 10637, 10743, 10850, 10959, 11068),
  populationAdults =   c(25903, 26162, 26424, 26688, 26955, 27224, 27497),
  populationAll = populationChildren + populationAdults,
  notifiedTBChildren = c(82, 60, 66, 69, 73, 77, 69),
  notifiedTBExperiencedAdults = c(105, 119, 130, 109, 130, 126, 137),
  notifiedTBNaiveAdults = c(172, 234, 200, 224, 216, 233, 210),
  notifiedTBAdults = notifiedTBNaiveAdults + notifiedTBExperiencedAdults,
  notifiedTBAll = notifiedTBAdults + notifiedTBChildren,
  prevalenceExperiencedAdults = 100*c(0.097, NA, NA, NA, NA, NA, NA),
  prevalenceHIV = 100*c(0.052, NA, NA, NA, NA, NA, NA),
  prevalenceInfectiousNaiveAdults = 100*c(0.0051, NA, NA, NA, NA, NA, NA),
  prevalenceInfectiousExperiencedAdults = 100*c(0.0299, NA, NA, NA, NA, NA, NA),
  prevalenceInfectiousAdults = prevalenceInfectiousNaiveAdults + prevalenceInfectiousExperiencedAdults
)

nIdv <- "Number of individuals"

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

mimicMapPlus <- list(tbTreatmentBeginChildren          = list(title="Tx notifications, children", 
                                                              y="Number of case notifications", 
                                                              calibration="notifiedTBChildren",
                                                              min=0, max=300, tag="tb"),
                     populationChildren                = list(title="Children", 
                                                              y="Number of individuals", 
                                                              calibration="populationChildren",
                                                              min=9200, max=15000, tag="demographic"),
                     txIndividuals                     = list(data=c("tbTxExperiencedAdults", "populationSize"), 
                                                              title="Tx-experienced adults", 
                                                              y="Proportion of individuals (%)", 
                                                              factor=100,
                                                              calibration="prevalenceExperiencedAdults",
                                                              min=0, max=25, tag="tb"),
                     tbTreatmentBeginAdultsNaive       = list(title="Notifications, Tx-naive adults", 
                                                              y="Number of case notifications",
                                                              calibration="notifiedTBNaiveAdults",
                                                              min=0, max=600, tag="tb"),
                     populationAdults                  = list(title="Adults", 
                                                              y="Number of individuals",
                                                              calibration="populationAdults",
                                                              min=22500, max=35000, tag="demographic"),
                     tbInfectiousNaive                 = list(data=c("tbTxNaiveInfectiousAdults", "tbTxNaiveAdults"), 
                                                              title="Tx-naive adults", 
                                                              y="Tuberculosis prevalence (%)", 
                                                              factor=100,
                                                              calibration="prevalenceInfectiousNaiveAdults",
                                                              min=0, max=2.0, tag="tb"),
                     tbTreatmentBeginAdultsExperienced = list(title="Notifications, Tx-experienced adults", 
                                                              y="Number of case notifications",
                                                              calibration="notifiedTBExperiencedAdults",
                                                              min=0, max=400, tag="tb"),
                     hivPrevalence                     = list(data=c("hivPositive", "populationSize"), 
                                                              title="HIV prevalence, all", 
                                                              y="Prevalence (%)", 
                                                              factor=100,
                                                              calibration="prevalenceHIV", # INCORRECT right now, includes everyhtig when it should include just adults
                                                              min=0, max=15, tag="hiv"), 
                     tbInfectiousExperienced           = list(data=c("tbTxExperiencedInfectiousAdults", "tbTxExperiencedAdults"), 
                                                              title="Tx-experienced adults", 
                                                              y="Tuberculosis prevalence (%)", 
                                                              factor=100,
                                                              calibration="prevalenceInfectiousExperiencedAdults",
                                                              min=0, max=10, tag="tb"),
                     tbLatent                          = list(data=c("tbLatent", "populationSize"), title="Latently-infected prevalence", y="Prevalence (%)", factor=100, min=0, max=100, tag="tb"), 
                     tbInfectionsHousehold             = list(title="Household infections", y="Infections/year", min=0, max=2500, tag="tb"),
                     tbInfectionsCommunity             = list(title="Community infections", y="Infections/year", min=0, max=2500, tag="tb"))

demographicMap <- list(birthRate=     list(title="Birth rate",
                                           y="Births/1000/year",
                                           data=c("births", "populationSize"),
                                           factor=1000,
                                           min=0, max=30,
                                           guess=19.61,
                                           tag="demographic"),
                       deathRate=     list(title="Death rate",
                                           y="Deaths/1000/year",
                                           data=c("deaths", "populationSize"),
                                           factor=1000,
                                           min=0, max=30,
                                           guess=16.99,
                                           tag="demographic"),
                       marriageRate=  list(title="Marriage rate",
                                           y="Marriages/1000/year",
                                           data=c("marriages", "populationSize"),
                                           factor=1000,
                                           min=0, max=30,
                                           guess=3,
                                           tag="demographic"),
                       divorceRate=   list(title="Divorce rate",
                                           y="Divorces/1000/year",
                                           data=c("divorces", "populationSize"),
                                           factor=1000,
                                           min=0, max=30,
                                           guess=0.4,
                                           tag="demographic"),
                       populationSize=list(title="Population size",
                                           y="Number of individuals",
                                           min=0, max=70000,
                                           tag="demographic",
                                           calibration="populationAll"))

hivMap <- list(hivPositivePrevalence=list(title="HIV Positive",
                                          y="Proportion of population (%)",
                                          data=c("hivPositive", "populationSize"),
                                          factor=100,
                                          min=0, max=40,
                                          tag="hiv"),
               hivDiagnosed=list(title="HIV Diagnosed",
                                 y="Proportion of population (%)",
                                 data=c("hivDiagnosed", "populationSize"),
                                 factor=100,
                                 min=0, max=40,
                                 tag="hiv"),
               hivDiagnosesVCT=list(title="HIV Diagnosis Rate, VCT",
                                    data=c("hivDiagnosesVCT", "populationSize"),
                                    y="Diagnoses/1000/yr",
                                    factor=1000,
                                    min=0, max=100,
                                    tag="hiv"),
               hivInfections=list(title="HIV Infection rate",
                                  y="Infections/1000/yr",
                                  factor=1000,
                                  min=0, max=100,
                                  tag="hiv"),
               hivPositiveARTUptake=list(title="HIV-Positive ART Prevalence",
                                         y="Proportion of HIV+ Individuals (%)",
                                         data=c("hivPositiveART", "hivPositive"),
                                         factor=100,
                                         min=0, max=100,
                                         tag="hiv"))

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


reviewLatestModelRun <- function (map, cols) {do.call(multiplot, flatten(list(grid(CreateGraphCatalog(outputLocation)$Loader, map, 1990), cols=cols)))}

generate_grid <- function(Loader, map, cols, rows=ceiling(length(map)/cols)) { 
  to_render <- grid(list(Loader), list("Run"), map, 1990)
  
  do.call(multiplot, flatten(list(to_render, cols=cols, rows=rows)))
}

DemographicGrid <- function(Loader, cols=4, rows=3) generate_grid(Loader, demographicMap, cols=cols, rows=rows)
HIVGrid <-         function(Loader, cols=4, rows=3) generate_grid(Loader, hivMap, cols=cols, rows=rows)
TBGrid <-          function(Loader, cols=4, rows=3) generate_grid(Loader, mimicMapPlus, cols=cols, rows=rows)

# reviewLatestModelRun(testMap, 3)
# reviewLatestModelRun(bigMap, 5)
# reviewLatestModelRun(mimicMap, 3)
# 
# do.call(multiplot, 
#         flatten(
#           list(
#             grid(list(CreateGraphCatalog(outputLocation, "scalingConstants_1/1554824796")$Loader,
#                       CreateGraphCatalog(outputLocation, "scalingConstants_2/1554825002")$Loader,
#                       CreateGraphCatalog(outputLocation, "scalingConstants_3/1554825188")$Loader,
#                       CreateGraphCatalog(outputLocation, "scalingConstants_4/1554825348")$Loader),
#                  list("C 002 H 1", "C 004 H 1", "C 016 H 1", "C 100 H 1"),
#                  mimicMapPlus, 
#                  1990), 
#             cols=4)))
# 
# do.call(multiplot, flatten(list(grid(cat$Loader, testMap, 1990), cols=4)))
# do.call(multiplot, flatten(list(grid(cat$Loader, bigMap, 1990), cols=4)))
# do.call(multiplot, flatten(list(grid(cat$Loader, mimicMap, 1990), cols=3)))
