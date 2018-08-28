library(tidyverse)

outputLocation <- "/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/"
paramsLocation <- "/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/params/"

CSVLoaderGen <- function(prefix) {
  function(name) read.csv(paste(prefix, name, ".csv", sep="")) %>% tbl_df()
}

Load <- CSVLoaderGen(outputLocation)
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


Graph <- function(name, x, y) {
  data <- Load(name)
  Graph_impl(data, x, y)
}

GraphRate <- function(name1, name2, scale, x, y) {
  name1 <- Load(name1)
  name2 <- Load(name2)
  
  RateTimeSeriesPerN(name1, name2, "value", scale) %>%
    Graph_impl(x, y)
}

Hist <- function(name, col, x) {
  data <- Load(name)
  ggplot(data, aes_string(col)) +
    geom_histogram() +
    xlab(x)
}

Pyramid <- function(name, x, y) Pyramid_impl(Load(name), x, y)
PopulationPyramid <- function(name, x, y) Pyramid_impl(Load(name), x, y)

Pyramid("populationPyramid")
Pyramid("deathPyramid")
GraphRate("births", "populationSize", 12000, "Time (months)", "Birth rate (births/1e3/year)") + 
  geom_smooth() + 
  geom_hline(yintercept = 19.61)
GraphRate("deaths", "populationSize", 12000, "Time (months)", "Death rate (deaths/1e3/year)") + 
  geom_smooth() + 
  geom_hline(yintercept = 16.99)
Graph("births", "Time (months)", "Births")
Graph("deaths", "Time (months)", "Deaths")
Graph("marriages", "Time (months)", "Marriages")
GraphRate("marriages", "populationSize", 12000, "Time (months)", "Marriage rate (marriages/1e3/year)") + 
  geom_smooth() + 
  geom_hline(yintercept = 3)
Graph("divorces", "Time (months)", "Divorces")
GraphRate("divorces", "populationSize", 12000, "Time (months)", "Divorce rate (divorces/1e3/year)") + 
  geom_smooth() +
  geom_hline(yintercept = 0.4)
Graph("populationSize", "Time (months)", "Population size")
Graph("householdsCount", "Time (months)", "# Households")
Graph("singleToLooking", "Time (months)", "Number of events")
GraphRate("singleToLooking", "populationSize", 12000, "Time (months)", "SingleToLooking rate (events/1e3/year)") +
  geom_smooth() +
  geom_hline(yintercept = 3.0)
Graph("hivDiagnosed", "Time (months)", "HIV Diagnosed")
Graph("hivDiagnosedVCT", "Time (months)", "HIV Diagnosed – VCT")
Graph("hivDiagnosesVCT", "Time (months)", "HIV Diagnoses – VCT")
Graph("hivInfections", "Time (months)", "HIV Infections")3.
Graph("hivNegative", "Time (months)", "HIV Negative")
Graph("hivPositive", "Time (months)", "HIV Positive")
Graph("hivPositiveART", "Time (months)", "HIV Positive + ART")
Hist("meanSurvivalTimeNoART", "age.at.infection", "Age at Infection")
Hist("meanSurvivalTimeNoART", "years.lived", "Years lived with HIV, no ART")

# scaling <- read.table("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/scaling.tsv")
# scaling2 <- read.table("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/SIRlib/scaling.tsv")

# Plot mean, median, and max of household size of each individual. Note that
# this will differ substantially from a histogram of the sizes of households
# in a population 
ps <- Load("populationSurvey")
hs <- Load("householdSurvey")

################################################
# POPULATION SURVEY GRAPHS
################################################

ps %>% 
  group_by(time) %>% 
  summarize(householdMax    = max(household),
            householdMean   = mean(household),
            householdMedian = median(household)) %>%
  ggplot() +
    geom_line(aes(time, householdMax), color="blue") +
    geom_line(aes(time, householdMean), color="red") +
    geom_line(aes(time, householdMedian), color="black") +
    ggtitle("Mean, median, max of individual household size") +
    labs(x="Time (days)", y="Size of household")

# Histogram of household size by year, all individuals
ps %>%
  ggplot() +
    geom_histogram(aes(household, fill=marital), binwidth=1) +
    facet_wrap(~(time-1)/365) +
    ggtitle("Histogram of household size by year, all individuals, coupleFormsNewHousehold=0.1") +
    labs(x="Household size", y="Count")

# Marital status by time
ps %>%
  group_by(time, marital) %>%
  count(marital) %>%
  ggplot(aes(time, n, fill=marital)) +
    geom_area()

# Histogram of household size by year, married individuals
ps %>%
  filter(marital == "married")  %>%
  ggplot() +
  geom_histogram(aes(household, fill=marital), binwidth=1) +
  facet_wrap(~(time-1)/365) +
  ggtitle("Histogram of household size by year, all individuals, coupleFormsNewHousehold=0.1") +
  labs(x="Household size", y="Count")

Lfollow_idvs <- function(data, n_idvs) {
  idvs <- data %>%
    sample_n(n_idvs, replace=FALSE) %>% # Sample n_idvs without replacement. Not perfect
    pull(hash)                          # Pull out the hash column into a vector
  
  data %>%
    filter(hash %in% idvs) # Filter by sampled individuals
}

View(follow_idvs(ps, 5))

follow_idvs(ps, 500) %>%
  ggplot(aes((time-1)/365, household)) +
    geom_jitter(aes(color = factor(offspring > 0))) +
    geom_smooth(aes(color = factor(offspring > 0))) +
    ggtitle("Differences in household size between those with and without children") +
    labs(x="Time (years)", y="Household size")

follow_idvs(ps, 500) %>%
  ggplot(aes((time-1)/365, household)) +
  geom_jitter(aes(color = factor(marital))) +
  geom_smooth(aes(color = factor(marital))) +
  ggtitle("Differences in household size by marital status") +
  labs(x="Time (years)", y="Household size") #+
  # ylim(0,10)
    
follow_idvs(ps, 500) %>%
  ggplot(aes((time-1)/365, household)) +
  geom_jitter(aes(color = factor(marital))) +
  geom_smooth(aes(color = factor(marital))) +
  ggtitle("Differences in household size by marital status") +
  labs(x="Time (years)", y="Household size") +
  ylim(0,10)

ps %>%
  filter(household == 1) %>%
  ggplot() +
  geom_histogram(aes(age)) +
  facet_wrap(~((time-1)/365))

orphans <- ps %>%
  follow_idvs(500) %>%
  filter(age <= 15, household == 1) %>%
  pull(hash)

unique(orphans) %>% length()

ps %>%
  filter(hash %in% orphans) %>%
  arrange(hash, time) %>%
  View()

ps %>%
  group_by(time) %>%
  summarize(populationSize = length(hash),
            married  = sum(marital == "married") / populationSize,
            single   = sum(marital == "single") / populationSize,
            divorced = sum(marital == "divorced") / populationSize,
            looking  = sum(marital == "looking") / populationSize) %>%
  select(-populationSize) %>%
  gather("field", "value", 2:5) %>%
  arrange(time, field) %>%
  ggplot(aes(time, value, color=field)) +
    geom_line()
    
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
    ggtitle("Household size differences between married and unmarried households") +
    ylim(0,10)

hs %>%
  ggplot(aes(size)) +
    facet_wrap(~time) +
    geom_histogram(binwidth=1) +
    xlim(0,10)

zeroes <- hs %>% filter(size == 0) %>% pull(hash) %>% sample(16)
hs %>%
  filter(hash %in% zeroes) %>%
  ggplot(aes(time)) +
    geom_line(aes(y=head, color="Head")) +
    geom_line(aes(y=spouse, color="Spouse")) +
    geom_line(aes(y=directOffspring, color="Offspring")) +
    geom_line(aes(y=other, color="Other")) +
    facet_wrap(~hash)

View(hs %>% filter(hash %in% zeroes) %>% arrange(hash, time))

hs %>% 
  group_by(time) %>%
  summarize(meanSize = mean(size),
            medianSize = median(size)) %>% View()

# Fraction of households that have zero members
hs %>%
  group_by(time) %>%
  summarize(
    fracZero = sum(size == 0) / length(size)
  ) %>%
  ggplot(aes(time, fracZero)) +
    geom_point() +
    ylim(0, 1.0)

# Big households: Differences between married and unparried households
hs %>%
  filter(size > 70) %>%
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
