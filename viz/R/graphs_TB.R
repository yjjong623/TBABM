library(tidyverse)

Prepper <- function(Loader) function(string) mutate(Loader(string), type=string)

tbTransmission = function(Loader) {
  household <- Loader("tbInfectionsHousehold") %>% mutate(source="Household")
  community <- Loader("tbInfectionsCommunity") %>% mutate(source="Community")
  
  combined <- bind_rows(household, community)
  
  combined %>%
    ggplot(aes(period, value, color=source, group=interaction(source, trajectory))) +
      geom_line()
}

# TB events
tbEvents <- function(Loader) {
  map(c("tbInfections", "tbConversions", "tbRecoveries"), Prepper(Loader)) %>%
    bind_rows() %>%
    ggplot(aes(period, value, color=type, group=interaction(type, trajectory))) + 
    geom_line() +
    labs(x="Time (years)", y = "Events/year", title="Infection events") +
    theme(legend.title = element_blank()) +
    scale_color_discrete(name="Data type", 
                         breaks=c("tbInfections", "tbConversions", "tbRecoveries"),
                         labels=c("TB Conversions", "TB Infections", "TB Recoveries"))
}

# TB compared to population size
tbOverview <- function(Loader) {
  map(c("populationSize", "tbLatent", "tbInfectious"), Prepper(Loader)) %>%
    bind_rows() %>%
    ggplot(aes(period, value, color=type, group=interaction(type, trajectory))) + 
    geom_line() +
    labs(x="Time (years)", y = "People", title="TB and Population Size") +
    theme(legend.title = element_blank()) +
    scale_color_discrete(name="Data type", 
                         breaks=c("populationSize", "tbLatent", "tbInfectious"),
                         labels=c("Population size", "Latent TB", "Infectious TB"))
}

# TB + Tx events
tbAll <- function(Loader) {
  map(c("tbInfections", "tbConversions", "tbRecoveries", "tbTreatmentBegin", "tbTreatmentEnd", "tbTreatmentDropout"), Prepper(Loader)) %>%
    bind_rows() %>%
    ggplot(aes(period, value, color=type, group=interaction(type, trajectory))) + 
    geom_line() +
    labs(x="Time (years)", y = "Events/year", title="All TB Events") +
    theme(legend.title = element_blank()) +
    scale_color_discrete(name="Data type", 
                         breaks=c("tbInfections", "tbConversions", "tbRecoveries", "tbTreatmentBegin", "tbTreatmentEnd", "tbTreatmentDropout"),
                         labels=c("TB Conversions", "TB Infections", "TB Recoveries", "Tx Init", "Tx End", "Tx Dropout"))
}


# TB + Tx pools
tbTreatment <- function(Loader) {
  map(c("tbInTreatment", "tbCompletedTreatment", "tbDroppedTreatment"), Prepper(Loader)) %>%
    bind_rows() %>%
    ggplot(aes(period, value, color=type, group=interaction(type, trajectory))) + 
    geom_line() +
    labs(x="Time (years)", y = "# People", title="Treatment status distribution") +
    theme(legend.title = element_blank()) +
    scale_color_discrete(name="Data type", 
                         breaks=c("tbInTreatment", "tbCompletedTreatment", "tbDroppedTreatment"),
                         labels=c("In Treatment", "Completed Treatment", "Dropped Treatment"))
}

