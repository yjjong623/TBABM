library(tidyverse)
source("R/utils_csv.R")
source("R/graph_templates.R", local=TRUE)

GetPrefix = function(location) paste(location, FindLatestTimestamp(location), "_", sep="")

CreateGraphCatalog <- function(outputLocation)
  with(
    list(Load = CSVLoaderGen(outputLocation),
         LoadLatest = CSVLoaderGen(GetPrefix(outputLocation))),
    list(
      deathRatePyramid = left_join(LoadLatest("deathPyramid"), 
                                   LoadLatest("populationPyramid"), 
                                   by=c("period", "trajectory", "age.group", "category")) %>%
        mutate(value = 1000*value.x / value.y) %>%
        Pyramid_impl("Deaths/1k/yr", "Age group"),
      populationPyramid = function() Pyramid("populationPyramid", "Count", "Age group"),
      deathPyramid = function() Pyramid("deathPyramid"),
      populationProportionPyramid = function() ProportionPyramid("populationPyramid", "Age group", "Proportion"),
      birthRate = function() GraphRate("births", "populationSize", 1000, "Time (years)", "Birth rate (births/1e3/year)"),
      deathRate = function() GraphRate("deaths", "populationSize", 1000, "Time (years)", "Death rate (deaths/1e3/year)"),
      births = function() GraphRate("births", "populationSize", 1000, "Time (years)", "Birth rate (births/1e3/year)"),
      deaths = function() GraphRate("deaths", "populationSize", 1000, "Time (years)", "Death rate (deaths/1e3/year)"),
      marriageRate = function() GraphRate("marriages", "populationSize", 1000, "Time (years)", "Marriage rate (marriages/1e3/year)"),
      divorceRate = function() GraphRate("divorces", "populationSize", 1000, "Time (years)", "Divorce rate (divorces/1e3/year)"),
      populationSize = Graph("populationSize", "Time (years)", "Population size") + guides(color=FALSE),
      
      hivDiagnosed          = function() Graph("hivDiagnosed", "Time (years)", "HIV Diagnosed"),
      hivDiagnosedVCT       = function() Graph("hivDiagnosedVCT", "Time (years)", "HIV Diagnosed – VCT"),
      hivDiagnosesVCT       = function() Graph("hivDiagnosesVCT", "Time (years)", "HIV Diagnoses – VCT"),
      hivInfections         = function() Graph("hivInfections", "Time (years)", "HIV Infections"),
      hivNegative           = function() Graph("hivNegative", "Time (years)", "HIV Negative"),
      hivPositive           = function() Graph("hivPositive", "Time (years)", "HIV Positive"),
      hivPositiveART        = function() Graph("hivPositiveART", "Time (years)", "HIV Positive + ART"),
      meanSurvivalTimeNoART = function() Hist("meanSurvivalTimeNoART", "age.at.infection", "Age at Infection"),
      meanSurvivalTimeNoART = function() Hist("meanSurvivalTimeNoART", "years.lived", "Years lived with HIV, no ART")    
    )
  )


GraphRate("births", "populationSize", 1000, "Time (years)", "Birth rate (births/1e3/year)") + 
  # geom_hline(yintercept = 19.61) + BIRTHRATE
  ggtitle("Birth rate over 50 years") +
  guides(color=FALSE)
  
GraphRate("deaths", "populationSize", 1000, "Time (years)", "Death rate (deaths/1e3/year)") + 
  # geom_hline(yintercept = 16.99) + DEATHRATE
  ggtitle("Death rate over 50 years") +
  guides(color=FALSE)

Graph("births", "Time (years)", "Births")
Graph("deaths", "Time (years)", "Deaths")

Graph("marriages", "Time (years)", "Marriages")
GraphRate("marriages", "populationSize", 1000, "Time (years)", "Marriage rate (marriages/1e3/year)") +
  # geom_hline(yintercept = 3) + MARRIAGERATE
  ggtitle("Marriage rate over 50 years") +
  guides(color=FALSE)

Graph("divorces", "Time (years)", "Divorces")
GraphRate("divorces", "populationSize", 1000, "Time (years)", "Divorce rate (divorces/1e3/year)") + 
  # geom_hline(yintercept = 0.4) + DIVORCERATE
  ggtitle("Divorce rate over 50 years") +
  guides(color=FALSE)

Graph("populationSize", "Time (years)", "Population size") + guides(color=FALSE)
Graph("householdsCount", "Time (years)", "# Households")
Graph("singleToLooking", "Time (years)", "Number of events")
GraphRate("singleToLooking", "populationSize", 1000, "Time (years)", "SingleToLooking rate (events/1e3/year)") +
  geom_hline(yintercept = 6.0)

Graph("hivDiagnosed", "Time (years)", "HIV Diagnosed")
Graph("hivDiagnosedVCT", "Time (years)", "HIV Diagnosed – VCT")
Graph("hivDiagnosesVCT", "Time (years)", "HIV Diagnoses – VCT")
Graph("hivInfections", "Time (years)", "HIV Infections")
Graph("hivNegative", "Time (years)", "HIV Negative")
Graph("hivPositive", "Time (years)", "HIV Positive")
Graph("hivPositiveART", "Time (years)", "HIV Positive + ART")
Hist("meanSurvivalTimeNoART", "age.at.infection", "Age at Infection")
Hist("meanSurvivalTimeNoART", "years.lived", "Years lived with HIV, no ART")

View(Load("meanSurvivalTimeNoART"))

# Plot mean, median, and max of household size of each individual. Note that
# this will differ substantially from a histogram of the sizes of households
# in a population 
ps <- LoadLatest("populationSurvey")
hs <- LoadLatest("householdSurvey")

################################################
# POPULATION SURVEY GRAPHS
################################################

# Histogram of number of children for married females
ps %>%
  filter(sex == "female" & 
         age > 15 &
         ((time-1)/365) %% 5 == 0) %>%
  ggplot(aes(offspring)) +
    geom_histogram(aes(y=..density..), binwidth = 1) +
    facet_wrap(~time)

ps %>%
  filter(((time-1)/365) %% 5 == 0 &
         sex == "female" &
         marital == "married") %>%
  group_by(time) %>%
  summarize(meanKids = mean(offspring),
            medianKids = median(offspring),
            maxKids = max(offspring))

# Marital status by time
ps %>%
  group_by(time, marital) %>%
  count(marital) %>%
  ggplot(aes(time, n, fill=marital)) +
    geom_area()

# People who live alone
ps %>%
  filter(household == 1) %>%
  ggplot() +
  geom_histogram(aes(age, fill=marital)) +
  facet_wrap(~time)

# Proportion of people by marital status
with(
  list(maxTime = (max(ps$time)-1)/365),
  ps %>%
    group_by(time, trajectory) %>%
    summarize(populationSize = length(hash),
              married  = sum(marital == "married")  / populationSize,
              single   = sum(marital == "single")   / populationSize,
              divorced = sum(marital == "divorced") / populationSize,
              looking  = sum(marital == "looking")  / populationSize) %>% 
    select(-populationSize) %>% 
    gather("maritalStatus", "proportion", 3:6) %>%
    arrange(time, maritalStatus, trajectory) %>%
    ggplot(aes((time-1)/365, proportion, color=maritalStatus, group=interaction(trajectory, maritalStatus))) +
      geom_line() +
      scale_y_continuous(breaks=seq(0,1,0.1), 
                         limits=c(0,1)) +
      scale_x_continuous(breaks=seq(0,maxTime, 5),
                         limits=c(0, maxTime)) +
      labs(x="Time (years)", 
           y="Proportion", 
           title="Proportion of people by marital status",
           fill="Marital status") +
      theme_bw()
)

################################################
# HOUSEHOLD SURVEY GRAPHS
################################################
hs %>%
  group_by(time) %>%
  summarize(
    medianSize    = median(size),
    meanSize      = mean(size),
    maxSize       = max(size),
    spousePresent = mean(spouse),
    meanOffspring = mean(directOffspring),
    meanOther     = mean(other)
  ) %>%
  ggplot(aes(time)) +
    geom_line(aes(y=meanSize, color="Mean")) +
    geom_line(aes(y=medianSize, color="Median")) +
    geom_line(aes(y=maxSize, color="Max")) +
    labs(x="Time (days)", y="Mean household size") +
    scale_y_continuous(breaks=seq(0,10,1), limits=c(0,10))

hs %>%
  group_by(time, trajectory) %>%
  count() %>%
  ggplot(aes(time, n, color=trajectory, group=trajectory)) +
    geom_line()

hs %>%
  filter(((time-1)/365) %% 5 == 0) %>%
  ggplot(aes(size)) +
    facet_wrap(~(time-1)/365) +
    geom_histogram(aes(y=..density.., fill=((directOffspring+otherOffspring)>0)), 
                   binwidth=1) +
    xlim(0,20) +
    theme_bw() +
    labs(fill="Offspring present in household",
         x="Household size",
         y="Density")

hs %>%
  filter(((time-1)/365) %% 5 == 0) %>%
  ggplot(aes(size)) +
  facet_wrap(~(time-1)/365) +
  geom_histogram(aes(y=..density..), 
                 binwidth=1) +
  xlim(0,20) +
  theme_bw() +
  labs(fill="Offspring present in household",
       x="Household size",
       y="Density")

hs %>%
  filter(size == 2) %>%
  group_by(time) %>%
  summarise(meanSpouse = mean(spouse),
            meanOffspring = mean(directOffspring + otherOffspring),
            meanOther = mean(other)) %>%
  gather("memberType", "value", 2:4) %>%
  arrange(time, memberType) %>%
  ggplot(aes((time-1)/365)) +
    geom_line(aes(y=value, color=memberType, group=memberType))

hs %>%
  filter(size >= 0 & size <= 10) %>%
  group_by(time, size) %>%
  summarize(meanKids = mean(directOffspring + otherOffspring),
            meanOther = mean(other))  %>%
  ggplot(aes(time)) +
    facet_wrap(~size) +
      geom_line(aes(y=meanKids, color="Offspring")) +
      geom_line(aes(y=meanOther, color="Other"))
    
# Big households: Differences between married and unparried households
hs %>%
  filter(size > 20) %>%
  ggplot(aes(time, color=spouse))  +
    geom_point(aes(y=size)) +
    geom_smooth(aes(y=size, color=spouse))

# Portion of offspring who are descendents of head or spouse
hs %>%
  # filter(hash %in% bigHouseholds) %>%
  mutate(offspring = otherOffspring+directOffspring,
         directPortion = directOffspring/offspring,
         otherPortion = otherOffspring/offspring) %>% 
  arrange(hash, time) %>%
  ggplot(aes(time)) +
  geom_smooth(aes(y=directPortion)) +
  geom_jitter(aes(y=directPortion, color=hash)) +
  ggtitle("Portion of offspring who are descendents of head or spouse") +
  labs(x="Time (Days)", y="Portion of offspring who are direct descendents")