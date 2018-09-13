library(tidyverse)

outputLocation <- "/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/"
paramsLocation <- "/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/params/"

CSVLoaderGen <- function(prefix) {
  function(name) read.csv(paste(prefix, name, ".csv", sep="")) %>% tbl_df()
}

FindLatestTimestamp <- function(dir) {
  fileWithBiggestTimestamp <- list.files(dir) %>% tail()
  
  # matching beginning of string, then 0-9
  pattern <- "\\d+"
  string <- fileWithBiggestTimestamp
  
  regmatches(string, regexpr(pattern, string))[1]
}

Load <- CSVLoaderGen(outputLocation)
LoadLatest <- function(file) {
  prefix <- paste(outputLocation, FindLatestTimestamp(outputLocation), "_", sep="")
  
  CSVLoaderGen(prefix)(file)
}
LoadParams <- CSVLoaderGen(paramsLocation)

JoinTimeSeries <- function(x, y, preserveNames=FALSE) {
  xName <- enquo(x) %>% quo_name()
  yName <- enquo(y) %>% quo_name()
  
  by <- c("period", "trajectory")
  joined <- left_join(x, y, by, copy=TRUE)
  
  if (preserveNames) {
    joined <- rename(joined, !!xName := value.x, !!yName := value.y)
  }
  
  joined
}

JoinAndDivideTimeSeries <- function(num, den, newColName, scale = 1) {
  JoinTimeSeries(num, den) %>%
    mutate(
      !!newColName := scale*value.x/value.y,
      value.x = NULL,
      value.y = NULL
    )
}

RateTimeSeriesPerN <- function(n, d, name, scale) JoinAndDivideTimeSeries(n, d, name, scale)

Graph_impl <- function(data, x, y) {
  ylim <- 1.1 * max(data$value, na.rm=TRUE)
  
  ggplot(data, aes(period, value, group=trajectory)) +
    geom_line(aes(color=trajectory)) +
    xlab(x) + ylab(y) +
    coord_cartesian(ylim=c(0, ylim))
}

AgeRangeToNum <- function(vec) {
  mapping <- function(str) {
    switch(as.character(str),
           "0-10"   = 0,
           "10-20"  = 10,
           "20-30"  = 20,
           "30-40"  = 30,
           "40-50"  = 40,
           "50-60"  = 50,
           "60-70"  = 60,
           "70-80"  = 70,
           "80-90"  = 80,
           "90-inf" = 90) 
  }
  
  map(vec, mapping) %>% map(as.integer) %>% unlist()
}

CategoryToGender <- function(vec) {
  mapping <- function(cat) {switch(cat, 0 = "Male", 1 = "Female")}
  
  map(vec, mapping) %>% unlist()
}

Pyramid_impl <- function(data, x, y) {
  male <- function(d) filter(d, category == 0)
  female <- function(d) filter(d, category == 1)
  
  data <- data %>% mutate(age.group = AgeRangeToNum(age.group))
  
  ggplot(data, aes(x = age.group, y = value, fill = category)) + 
    geom_bar(data = male, stat = "identity") + 
    geom_bar(aes(y=-value), data = female, stat = "identity") +
    # scale_y_continuous(breaks = seq(-25000, 25000, 5000), 
                       # labels = paste0(as.character(c(seq(-25, 0, 5), seq(5, 25, 5))), "k")) + 
    coord_flip() +
    facet_wrap(~period)
}

PopulationPyramidProportion_impl <- function(data, x, y) {
  
}

# Graph a specific run
GraphRun <- function(name, x, y) {
  data <- Load(name)
  Graph_impl(data, x, y)
}

# Graph the latest run
Graph <- function(name, x, y) {
  data <- LoadLatest(name)
  Graph_impl(data, x, y)
}

# Do a rate graph from the latest run 
GraphRateRun <- function(name1, name2, scale, x, y) {
  name1 <- Load(name1)
  name2 <- Load(name2)
  
  RateTimeSeriesPerN(name1, name2, "value", scale) %>%
    Graph_impl(x, y)
}

# Do a rate graph of a particular run
GraphRate <- function(name1, name2, scale, x, y) {
  name1 <- LoadLatest(name1)
  name2 <- LoadLatest(name2)
  
  RateTimeSeriesPerN(name1, name2, "value", scale) %>%
    Graph_impl(x, y)
}

Hist <- function(name, col, x) {
  data <- LoadLatest(name)
  ggplot(data, aes_string(col)) +
    geom_histogram() +
    xlab(x)
}

Pyramid <- function(name, x, y) Pyramid_impl(LoadLatest(name), x, y)
PopulationPyramid <- function(name, x, y) Pyramid_impl(LoadLatest(name), x, y)

# Death rate pyramid
left_join(LoadLatest("deathPyramid"), 
          LoadLatest("populationPyramid"), 
          by=c("period", "trajectory", "age.group", "category")) %>%
  mutate(value = 1000*value.x / value.y) %>%
  Pyramid_impl("Deaths/1k/yr", "Age group")

Pyramid("populationPyramid")
Pyramid("deathPyramid")

GraphRate("births", "populationSize", 1000, "Time (years)", "Birth rate (births/1e3/year)") + 
  geom_hline(yintercept = 19.61) +
  ggtitle("Birth rate over 50 years")
GraphRate("deaths", "populationSize", 1000, "Time (years)", "Death rate (deaths/1e3/year)") + 
  geom_hline(yintercept = 16.99) +
  ggtitle("Death rate over 50 years")
Graph("births", "Time (years)", "Births")
Graph("deaths", "Time (years)", "Deaths")

Graph("marriages", "Time (years)", "Marriages")
GraphRate("marriages", "populationSize", 1000, "Time (years)", "Marriage rate (marriages/1e3/year)") +
  geom_hline(yintercept = 3) +
  ggtitle("Population size over 50 years")
Graph("divorces", "Time (years)", "Divorces")
GraphRate("divorces", "populationSize", 1000, "Time (years)", "Divorce rate (divorces/1e3/year)") + 
  geom_hline(yintercept = 0.4) +
  ggtitle("Divorce rate over 50 years")

Graph("populationSize", "Time (years)", "Population size")
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
         marital == "married") %>%
  ggplot(aes(offspring)) +
    geom_histogram(binwidth = 1) +
    facet_wrap(~time)

# Marital status by time
ps %>%
  group_by(time, marital) %>%
  count(marital) %>%
  ggplot(aes(time, n, fill=marital)) +
    geom_area()

follow_idvs <- function(data, n_idvs) {
  idvs <- data %>%
    sample_n(n_idvs, replace=FALSE) %>% # Sample n_idvs without replacement. Not perfect
    pull(hash)                          # Pull out the hash column into a vector
  
  data %>%
    filter(hash %in% idvs) # Filter by sampled individuals
}

View(follow_idvs(ps, 5))

follow_idvs(ps, 500) %>%
  ggplot(aes(time, household)) +
    geom_jitter(aes(color = factor(offspring > 0))) +
    geom_smooth(aes(color = factor(offspring > 0))) +
    ggtitle("Differences in household size between those with and without children") +
    labs(x="Time (years)", y="Household size")

follow_idvs(ps, 500) %>%
  ggplot(aes(time, household)) +
  geom_jitter(aes(color = factor(marital))) +
  geom_smooth(aes(color = factor(marital))) +
  ggtitle("Differences in household size by marital status") +
  labs(x="Time (years)", y="Household size") #+
  # ylim(0,10)
    
follow_idvs(ps, 500) %>%
  ggplot(aes(time, household)) +
  geom_jitter(aes(color = factor(marital))) +
  geom_smooth(aes(color = factor(marital))) +
  ggtitle("Differences in household size by marital status") +
  labs(x="Time (years)", y="Household size") +
  ylim(0,10)

# People who live alone
ps %>%
  filter(household == 1) %>%
  ggplot() +
  geom_histogram(aes(age, fill=marital)) +
  facet_wrap(~time)

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
      labs(x="Time (years)", y="Proportion", title="Proportion of people by marital status")
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
    ylim(0,10)

hs %>%
  ggplot(aes(size)) +
    facet_wrap(~time) +
    geom_histogram(binwidth=1) +
    xlim(0,20)

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

# ggplot(scaling, aes(log10(V1),log10(V2))) +
#   geom_line() +
#   scale_x_continuous(name = "Population size", labels = scales::math_format(10^.x)) +
#   scale_y_continuous(name = "Runtime (s)", labels = scales::math_format(10^.x))

# ggplot(scaling2, aes(log10(V1),log10(V2))) +
#   geom_line() +
#   scale_x_continuous(name = "Population size", labels = scales::math_format(10^.x)) +
#   scale_y_continuous(name = "Runtime (s)", labels = scales::math_format(10^.x))
# 
