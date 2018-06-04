library(tidyverse)

births <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/births.csv")
deaths <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/deaths.csv")
marriages <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/marriages.csv")
divorces <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/divorces.csv")
populationSize <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/populationSize.csv")
households <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/householdsCount.csv")
histT0 <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/histogramT1.csv")
histT10 <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/output/histogramT10.csv")
scaling <- read.table("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/TBABM/scaling.tsv")
scaling2 <- read.table("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/SIRlib/scaling.tsv")

plot.ts(births[,c("births", "births.1", "births.2", "births.3", "births.4")], plot.type=c("single"), xlab="Time (months)", ylab="Births/month")
plot.ts(deaths[,c("deaths", "deaths.1", "deaths.2", "deaths.3", "deaths.4")], plot.type=c("single"), xlab="Time (months)", ylab="Deaths/month")
plot.ts(marriages[,c("marriages", "marriages.1", "marriages.2", "marriages.3", "marriages.4")], plot.type=c("single"), xlab="Time (months)", ylab="Marriages/month")
plot.ts(divorces[,c("divorces", "divorces.1", "divorces.2", "divorces.3", "divorces.4")], plot.type=c("single"), xlab="Time (months)", ylab="Divorces/month")
plot.ts(populationSize[,c("populationSize", "populationSize.1", "populationSize.2", "populationSize.3", "populationSize.4")], plot.type=c("single"), xlab="Time (months)", ylab="Population size")
plot.ts(households[,c("households", "households.1", "households.2", "households.3", "households.4")], plot.type=c("single"), xlab="Time (years)", ylab="# Households")

library(ggplot2)

ggplot(histT0, aes(x=X9)) +
  geom_histogram(binwidth = 1) +
  xlim(0,10) +
  xlab("Household size") +
  ylab("Frequency")

ggplot(histT10, aes(x=X5)) +
  geom_histogram(binwidth = 1) +
  xlim(0,10) +
  xlab("Household size") +
  ylab("Frequency")


ggplot(scaling, aes(log10(V1),log10(V2))) +
  geom_line() +
  scale_x_continuous(name = "Population size", labels = scales::math_format(10^.x)) +
  scale_y_continuous(name = "Runtime (s)", labels = scales::math_format(10^.x))

ggplot(scaling2, aes(log10(V1),log10(V2))) +
  geom_line() +
  scale_x_continuous(name = "Population size", labels = scales::math_format(10^.x)) +
  scale_y_continuous(name = "Runtime (s)", labels = scales::math_format(10^.x))

S <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/SIRlib/TestSIRlib/serial-susceptible.csv")
I <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/SIRlib/TestSIRlib/serial-infected.csv")
R <- read.csv("/Users/marcusrussi/Desktop/Yaesoubi-Cohen-Lab/repos/SIRlib/TestSIRlib/serial-recovered.csv")

SIR <- merge(S, I) %>% merge(R)

ggplot(SIR, aes(Period)) +
  geom_line(aes(y=Infected), colour="red") +
  geom_line(aes(y=Infected.1), colour="red") +
  geom_line(aes(y=Infected.2), colour="red") +
  geom_line(aes(y=Infected.3), colour="red") +
  geom_line(aes(y=Infected.4), colour="red") +
  geom_line(aes(y=Infected.5), colour="red") +
  geom_line(aes(y=Infected.6), colour="red") +
  geom_line(aes(y=Infected.7), colour="red") +
  geom_line(aes(y=Infected.8), colour="red") +
  geom_line(aes(y=Infected.9), colour="red") +
  geom_line(aes(y=Susceptible), colour="yellow") +
  geom_line(aes(y=Susceptible.1), colour="yellow") +
  geom_line(aes(y=Susceptible.2), colour="yellow") +
  geom_line(aes(y=Susceptible.3), colour="yellow") +
  geom_line(aes(y=Susceptible.4), colour="yellow") +
  geom_line(aes(y=Susceptible.5), colour="yellow") +
  geom_line(aes(y=Susceptible.6), colour="yellow") +
  geom_line(aes(y=Susceptible.7), colour="yellow") +
  geom_line(aes(y=Susceptible.8), colour="yellow") +
  geom_line(aes(y=Susceptible.9), colour="yellow") +
  geom_line(aes(y=Recovered), colour="green") +
  geom_line(aes(y=Recovered.1), colour="green") +
  geom_line(aes(y=Recovered.2), colour="green") +
  geom_line(aes(y=Recovered.3), colour="green") +
  geom_line(aes(y=Recovered.4), colour="green") +
  geom_line(aes(y=Recovered.5), colour="green") +
  geom_line(aes(y=Recovered.6), colour="green") +
  geom_line(aes(y=Recovered.7), colour="green") +
  geom_line(aes(y=Recovered.8), colour="green") +
  geom_line(aes(y=Recovered.9), colour="green") +
  xlab("Time (weeks)") + 
  ylab("People")

ggplot(SIR, aes(Period)) +
  geom_line(aes(y=Infected))

names(S)[2:10]
