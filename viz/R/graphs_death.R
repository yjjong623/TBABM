library(ggplot2)
library(dplyr)

hivSurvivalWithART <- function(ds) {
  age_groups <- c(0, 25, 35, 45, 55, 1e4)
  
  ART_deaths <- ds %>% filter(HIV == "true" & ART == "true")
  
  grouped <- ART_deaths %>%
    mutate(hiv_infection_age = pmax(0, age-(time-HIV_date)/365)) %>%
    transmute(sex        = sex,
              age_group  = cut(hiv_infection_age, age_groups, include.lowest = TRUE),
              cd4_group  = cut(baseline_CD4, c(0, 50, 100, 200, 10000), include.lowest=TRUE),
              livedOnART = (time-ART_date)/365) %>%
    group_by(age_group, cd4_group, sex) %>%
    summarize(life_expectancy = mean(livedOnART))
  
  ggplot(grouped, aes(age_group, life_expectancy, fill=cd4_group)) +
    geom_bar(stat="identity", position=position_dodge2(preserve="single")) +
    facet_wrap(~sex)
}
