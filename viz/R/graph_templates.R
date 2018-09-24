library(tidyverse)
source("utils_timeseries.R")

Graph_impl <- function(data, xlab, ylab) {
  ylim <- 1.1 * max(data$value, na.rm=TRUE)
  
  ggplot(data, aes(period, 
                   value, 
                   color=trajectory, 
                   group=trajectory)) +
    geom_line() +
    labs(x=xlab, y=ylab) +
    coord_cartesian(ylim=c(0, ylim)) +
    theme_bw()
}

Pyramid_impl <- function(data, x, y) {
  male   <- function(d) filter(d, category == 0)
  female <- function(d) filter(d, category == 1)
  
  data <- data %>% mutate(age.group = AgeRangeToNum(age.group))
  
  ggplot(data, aes(x = age.group, y = value, fill = category)) + 
    geom_bar(data = male, stat = "identity") + 
    geom_bar(aes(y=-value), data = female, stat = "identity") +
    coord_flip() +
    facet_wrap(~period) + 
    theme_bw()
}

PopulationPyramidProportion_impl <- function(data, x, y, every) {
  male   <- function(d) filter(d, category == 0)
  female <- function(d) filter(d, category == 1)
  
  data %>%
    filter((period %% every) == 0) %>%
    group_by(period, trajectory) %>%
    mutate(total = sum(value),
           proportion = value / total,
           age.group = AgeRangeToNum(age.group)) %>%
    ggplot(aes(age.group, proportion, fill=interaction(category), group=interaction(category, trajectory))) +
    geom_bar(data=male,   aes(y=proportion),  stat="identity", position="dodge") +
    geom_bar(data=female, aes(y=-proportion), stat="identity", position="dodge") +
    coord_flip() +
    facet_wrap(~period) +
    labs(x=x, y=y, fill="Gender")
}

# Graph a run. Assumes you want the latest run
GraphRun <- function(name, x, y, traj="latest") {
  data <- ifelse(traj=="latest", LoadLatest, Load)(name)
  Graph_impl(data, x, y)
}

# Do a rate graph from a particular run 
GraphRate <- function(name1, name2, scale, x, y, traj="latest") {
  data1 <- ifelse(traj=="latest", LoadLatest, Load)(name1)
  data2 <- ifelse(traj=="latest", LoadLatest, Load)(name2)
  
  RateTimeSeriesPerN(data1, data2, "value", scale) %>%
    Graph_impl(x, y)
}

Hist <- function(name, col, xlab) {
  data <- LoadLatest(name)
  ggplot(data, aes_string(col)) +
    geom_histogram() +
    labs(x=xlab)
}


Pyramid <- function(name, x, y) Pyramid_impl(LoadLatest(name), x, y)

ProportionPyramid <- function(name, x, y, every=5) PopulationPyramidProportion_impl(LoadLatest(name), x, y, every)