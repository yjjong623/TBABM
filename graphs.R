library(tidyverse)

outputLocation <- "/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/"

CSVLoaderGen <- function(prefix) {
  function(name) read.csv(paste(prefix, name, ".csv", sep="")) %>% tbl_df()
}

Load <- CSVLoaderGen(outputLocation)

Graph_impl <- function(data, x, y) {
  ylim <- 1.1 * max(data$value, na.rm=TRUE)
  
  ggplot(data, aes(period, value, group=trajectory)) +
    geom_line(aes(color=trajectory)) +
    xlab(x) + ylab(y) +
    coord_cartesian(ylim=c(0, ylim))
}

Graph <- function(name, x, y) {
  data <- Load(name)
  Graph_impl(data, x, y)
}

Hist <- function(name, col, x) {
  data <- Load(name)
  ggplot(data, aes_string(col)) +
    geom_histogram() +
    xlab(x)
}

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

JoinAndDivide <- function(num, den, by, newColName, scale = 1) {
  JoinTimeSeries(num, den) %>%
    mutate(
      !!newColName := scale*value.x/value.y,
      value.x := NULL,
      value.y := NULL
    )
}

JoinAndDivideTimeSeries <- function(num, den, newColName, scale = 1) {
  JoinAndDivide(num, den, c("period", "trajectory"), newColName, scale)
}

RateTimeSeries <- function(n, d, name) JoinAndDivideTimeSeries(n,d,name)

RateTimeSeriesPerN <- function(n, d, name, scale) JoinAndDivideTimeSeries(n,d,name,scale)

RateTimeSeries(births, populationSize, "birthRate")
RateTimeSeriesPerN(births, populationSize, "birthRate", 1e3)

births <- Load("births")
populationSize <- Load("populationSize")


Graph("births", "Time (months)", "Births")
Graph("deaths", "Time (months)", "Deaths")
Graph("marriages", "Time (months)", "Marriages")
Graph("divorces", "Time (months)", "Divorces")
Graph("populationSize", "Time (months)", "Population size")
Graph("householdsCount", "Time (months)", "# Households")
Graph("hivDiagnosed", "Time (months)", "HIV Diagnosed")
Graph("hivDiagnosedVCT", "Time (months)", "HIV Diagnosed – VCT")
Graph("hivDiagnosesVCT", "Time (months)", "HIV Diagnoses – VCT")
Graph("hivInfections", "Time (months)", "HIV Infections")
Graph("hivNegative", "Time (months)", "HIV Negative")
Graph("hivPositive", "Time (months)", "HIV Positive")
Graph("hivPositiveART", "Time (months)", "HIV Positive + ART")
Hist("meanSurvivalTimeNoART", "age.at.infection", "Age at Infection")
Hist("meanSurvivalTimeNoART", "years.lived", "Years lived with HIV, no ART")

meanSurvivalTimeNoART <- Load("meanSurvivalTimeNoART")

mean(meanSurvivalTimeNoART[,2])
mean(meanSurvivalTimeNoART[,1])

sort(meanSurvivalTimeNoART[,2])

# histT0 <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/histogramT1.csv")
# histT10 <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/histogramT10.csv")
# scaling <- read.table("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/scaling.tsv")
# scaling2 <- read.table("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/SIRlib/scaling.tsv")

# ggplot(histT0, aes(x=X9)) +
#   geom_histogram(binwidth = 1) +
#   xlim(0,10) +
#   xlab("Household size") +
#   ylab("Frequency")
# 
# ggplot(histT10, aes(x=X5)) +
#   geom_histogram(binwidth = 1) +
#   xlim(0,10) +
#   xlab("Household size") +
#   ylab("Frequency")


# ggplot(scaling, aes(log10(V1),log10(V2))) +
#   geom_line() +
#   scale_x_continuous(name = "Population size", labels = scales::math_format(10^.x)) +
#   scale_y_continuous(name = "Runtime (s)", labels = scales::math_format(10^.x))

# ggplot(scaling2, aes(log10(V1),log10(V2))) +
#   geom_line() +
#   scale_x_continuous(name = "Population size", labels = scales::math_format(10^.x)) +
#   scale_y_continuous(name = "Runtime (s)", labels = scales::math_format(10^.x))
# 
# S <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/SIRlib/TestSIRlib/serial-susceptible.csv")
# I <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/SIRlib/TestSIRlib/serial-infected.csv")
# R <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/SIRlib/TestSIRlib/serial-recovered.csv")
# 
# SIR <- merge(S, I) %>% merge(R)
# 
# ggplot(SIR, aes(Period)) +
#   geom_line(aes(y=Infected), colour="red") +
#   geom_line(aes(y=Infected.1), colour="red") +
#   geom_line(aes(y=Infected.2), colour="red") +
#   geom_line(aes(y=Infected.3), colour="red") +
#   geom_line(aes(y=Infected.4), colour="red") +
#   geom_line(aes(y=Infected.5), colour="red") +
#   geom_line(aes(y=Infected.6), colour="red") +
#   geom_line(aes(y=Infected.7), colour="red") +
#   geom_line(aes(y=Infected.8), colour="red") +
#   geom_line(aes(y=Infected.9), colour="red") +
#   geom_line(aes(y=Susceptible), colour="yellow") +
#   geom_line(aes(y=Susceptible.1), colour="yellow") +
#   geom_line(aes(y=Susceptible.2), colour="yellow") +
#   geom_line(aes(y=Susceptible.3), colour="yellow") +
#   geom_line(aes(y=Susceptible.4), colour="yellow") +
#   geom_line(aes(y=Susceptible.5), colour="yellow") +
#   geom_line(aes(y=Susceptible.6), colour="yellow") +
#   geom_line(aes(y=Susceptible.7), colour="yellow") +
#   geom_line(aes(y=Susceptible.8), colour="yellow") +
#   geom_line(aes(y=Susceptible.9), colour="yellow") +
#   geom_line(aes(y=Recovered), colour="green") +
#   geom_line(aes(y=Recovered.1), colour="green") +
#   geom_line(aes(y=Recovered.2), colour="green") +
#   geom_line(aes(y=Recovered.3), colour="green") +
#   geom_line(aes(y=Recovered.4), colour="green") +
#   geom_line(aes(y=Recovered.5), colour="green") +
#   geom_line(aes(y=Recovered.6), colour="green") +
#   geom_line(aes(y=Recovered.7), colour="green") +
#   geom_line(aes(y=Recovered.8), colour="green") +
#   geom_line(aes(y=Recovered.9), colour="green") +
#   xlab("Time (weeks)") + 
#   ylab("People")
# 
# ggplot(SIR, aes(Period)) +
#   geom_line(aes(y=Infected))
# 
# names(S)[2:10]
