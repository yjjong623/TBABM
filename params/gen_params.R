library(tidyr)

proto <- read_csv("prototype.csv") %>% as_tibble()
substitutions <- read_csv("rangefile_example.csv") %>% as_tibble()

subst_list <- pmap(substitutions, function(name, lower, upper, step) seq(lower, upper, step)) %>%
  setNames(substitutions$name) %>%
  cross()

generateNewParamSheet <- function(substitutions, proto) {
  for (name in names(substitutions)) {
    proto[which(proto$`short-name` == name), 'parameter-1'] <- substitutions[[name]]
  }
  
  proto
}

new_sheets <- subst_list %>%
  map(~generateNewParamSheet(., proto))

map(new_sheets, ~.[which(.$`short-name` %in% substitutions$name), c('short-name', 'parameter-1')])
