library(tidyverse)

outputLocation <- "/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/"

CSVLoaderGen <- function(prefix) {
  function(name) {
    read.csv(paste(prefix, name, ".csv", sep=""))
  }
}

Load <- CSVLoaderGen(outputLocation)

Graph <- function(name, x, y) {
  data <- Load(name)
  cols <- colnames(data)
  colsOfInterest <- cols[2:length(cols)]
  
  plot.ts(data[,colsOfInterest], plot.type=c("single"), xlab=x, ylab=y)
  ggplot(data, aes(period, value, group=trajectory)) +
    geom_line(aes(color=trajectory)) +
    scale_color_brewer(type='div', palette=4) +
    xlab(x) + ylab(y)
}

Graph("births", "Time (weeks)", "Births")
Graph("deaths", "Time (weeks)", "Deaths")
Graph("marriages", "Time (weeks)", "Marriages")
Graph("divorces", "Time (weeks)", "Divorces")
Graph("populationSize", "Time (weeks)", "Population size")
Graph("householdsCount", "Time (weeks)", "# Households")
Graph("hivDiagnosed", "Time (Weeks)", "HIV Diagnosed")
Graph("hivDiagnosedVCT", "Time (Weeks)", "HIV Diagnosed – VCT")
Graph("hivDiagnosesVCT", "Time (Weeks)", "HIV Diagnoses – VCT")
Graph("hivInfections", "Time (Weeks)", "HIV Infections")
Graph("hivNegative", "Time (Weeks)", "HIV Negative")
Graph("hivPositive", "Time (Weeks)", "HIV Positive")
Graph("hivPositiveART", "Time (Weeks)", "HIV Positive + ART")

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
