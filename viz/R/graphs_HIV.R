library(ggplot2)
library(dplyr)

hivCD4Decline <- function(ps, ds, n_samples) {
  # For everybody who gets HIV, create a list of the first survey time
  # where they registered as infected
  hivppl <- ps %>% 
    group_by(hash) %>%
    summarize(HIV.date = first(time[HIV == "true"])) %>%
    filter(!is.na(HIV.date))
  
  good_stuff <- inner_join(ps, hivppl, by="hash") %>%
    mutate(time_ = time - HIV.date) %>%
    group_by(hash) %>%
    mutate(lastObs = max(time),
           initial.CD4 = first(CD4[HIV=="true"]),
           CD4 = ifelse(HIV=="true", CD4, initial.CD4),
           age.at.infection = first(age[HIV=="true"]),
           age.group = cut(age.at.infection, c(-1, 15, 25, 35, 45, 1e6), right=FALSE))
  
  ppl <- good_stuff %>% pull(hash) %>% unique() %>% sample(n_samples)
  
  deaths <- filter(ds, hash %in% ppl) %>%
    mutate(age.group=cut(age-(time-HIV_date)/365, c(-1, 15, 25, 35, 45, 1e6), right=FALSE)) %>%
  
  # Graph of CD4 decline and AIDS development
  good_stuff %>% 
    filter(hash %in% ppl) %>%
    ggplot(aes(time_/365, CD4, group=hash)) +
    geom_line(lineend="butt") +
    geom_point(data=deaths, aes((time-HIV_date)/365, CD4), inherit.aes=FALSE) +
    geom_hline(yintercept = 200, color="red") +
    lims(x=c(0, 25))
  
  # Graph of CD4 decline and AIDS development, by age group
  good_stuff %>% 
    filter(hash %in% ppl) %>%
    ggplot(aes(time_/365, CD4, group=hash)) +
    geom_line() +
    geom_point(data=deaths, aes((time-HIV_date)/365, CD4, color=cause), inherit.aes=FALSE) +
    geom_hline(yintercept = 200, color="blue") +
    coord_cartesian(xlim=c(0, 20), ylim=c(0, 1750)) +
    labs(color="Cause of death",
         x="Years since infection",
         y="CD4 count (cells/ml)",
         title="CD4 decline following HIV infection",
         subtitle="Blue line represents AIDS/CD4=200, stratified by age group") +
    facet_wrap(~age.group)
}

hivSurvivalNoART <- function(ds) {
  # Histogram of years of infection by age group
  ds %>%
    filter(HIV == "true" & ART == "false") %>%
    mutate(year.of.death = cut(time/365, seq(0, 50, 1), right=FALSE, labels=FALSE) + 1990,
           years.of.infection = (time-HIV_date)/365,
           age.group = cut(age-years.of.infection, c(-1, 15, 25, 35, 45, 1e6), right = FALSE)) %>%
    ggplot(aes(years.of.infection, y=..density..)) +
    geom_histogram(aes(fill=age.group)) +
    geom_label(data=function(d) group_by(d, age.group) %>% summarize(myoi = mean(years.of.infection)),
               aes(x=myoi, y=0, label=format(myoi, nsmall=1, digits=2) %>% paste0("yr")), fontface="bold") +
    lims(x=c(0, 30)) +
    labs(x="Survival time", 
         y="Density", 
         fill="Age group",
         title="Years of infection, by age group",
         subtitle="HIV-positive, ART-naive individuals") +
    facet_wrap(~age.group)
}