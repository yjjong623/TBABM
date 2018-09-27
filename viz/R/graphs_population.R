library(ggplot2)

################################################
# POPULATION SURVEY GRAPHS
################################################

# Histogram of number of children for married females
childrenPerWoman <- function(ps) {
  ps %>%
  filter(sex == "female" & 
           age > 15 &
           ((time-1)/365) %% 5 == 0) %>%
  ggplot(aes(offspring)) +
  geom_histogram(aes(y=..density..), binwidth = 1) +
  facet_wrap(~time)
}

childrenStats <- function(ps) {
  ps %>%
  filter(((time-1)/365) %% 5 == 0 &
           sex == "female" &
           marital == "married") %>%
  group_by(time) %>%
  summarize(meanKids = mean(offspring),
            medianKids = median(offspring),
            maxKids = max(offspring))
}

# People who live alone
livingAlone <- function(ps) {
  ps %>%
  filter(household == 1) %>%
  ggplot() +
  geom_histogram(aes(age, fill=marital)) +
  facet_wrap(~time)
}

# Proportion of people by marital status
maritalStatus <- function(ps) with(
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

childrenVsOthers <- function(ps) {
  hs %>%
    filter(size >= 0 & size <= 10) %>%
    group_by(time, size) %>%
    summarize(meanKids = mean(directOffspring + otherOffspring),
              meanOther = mean(other))  %>%
    ggplot(aes(time)) +
    facet_wrap(~size) +
    geom_line(aes(y=meanKids, color="Offspring")) +
    geom_line(aes(y=meanOther, color="Other"))
}

childrenVsOthers <- function(ps) {
  classifier <- function(age) cut(age, 
                                  c(0,5,18,110), 
                                  include.lowest=TRUE, 
                                  right=FALSE,
                                  labels=c("baby", "adolescent", "adult"))
  
  ps %>%
    mutate(ageGroup = classifier(age)) %>%
    group_by(trajectory, householdHash, time, ageGroup) %>%
    count() %>%
    ungroup() %>%
    spread(ageGroup, n) %>% 
    select(-`<NA>`) %>%
    replace_na(list(baby=0, adolescent=0, adult=0)) %>% 
    mutate(size = baby+adolescent+adult) %>%
    gather("ageGroup", "n", 4:6) %>% 
    group_by(trajectory, time, size, ageGroup) %>%
    summarize(mean = mean(n)) %>% 
    filter(size <= 10) %>%
    ggplot(aes(time, mean, color=ageGroup, group=interaction(trajectory, ageGroup))) +
    geom_line() + 
    facet_wrap(~size)
}
