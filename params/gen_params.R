library(tidyr)

GenRunSheets <- function(proto_fname, rangefile_fname) {
  
  # Read the prototype file and the substitutions file in and convert them to
  # tibbles
  proto <- read_csv(proto_fname) %>% as_tibble()
  substitutions <- read_csv(rangefile_fname) %>% as_tibble()
  
  # Cross all of the different parameter combinations together. The result of
  # this call is an unkeyed list of lists, where the innermost lists contain k-v
  # pairs. Each innermost list represents a set of parameters that will be used
  # to modify the prototype RunSheet. Thus, the keys of the innermost lists
  # correspond to the 'short-name' of a parameter, and the values, for right
  # now, are what 'parameter-1' should be set to.
  crossed <- pmap(substitutions, function(name, lower, upper, step) seq(lower, upper, step)) %>%
    setNames(substitutions$name) %>% cross()

  # print(crossed)
  
  # Given a list of substitutions, outputs a new parameter sheet, as a tibble, which
  # includes these substitutions
  # 'substitutions" is a keyed list, containing 'short-name':'parameter-1' pairs.
  GenRunSheet <- function(substitutions, new_runsheet) {
    for (name in names(substitutions)) {
      row <- which(new_runsheet$`short-name` == name)
      column <- 'parameter-1'
      
      new_runsheet[row, column] <- substitutions[[name]]
    }
    
    new_runsheet
  }
  
  crossed %>% map(~GenRunSheet(., proto))
}

InspectNewRunSheets <- function(NewRunSheets, rangefile_fname) {
  substitutions <- read_csv(rangefile_fname) %>% as_tibble()
  
  map(NewRunSheets, ~.[which(.$`short-name` %in% substitutions$name), c('short-name', 'parameter-1')])
}

PlotNewRunSheets <- function(new_runsheets, rangefile_fname) {
  all_sheets <- InspectNewRunSheets(new_runsheets, rangefile_fname) %>%
    bind_rows(.id="group")
  
  ggplot(all_sheets, aes(`short-name`, 
                         `parameter-1`, 
                         group=group, 
                         color=group)) + 
    geom_line() +
    geom_point()
}

WriteRunSheets <- function(runsheets, prefix="RunSheet_") {
  num_sheets <- length(runsheets)
  num_digits <- floor(log10(num_sheets)) + 1
  
  WriteRunSheet <- function(runsheet, sheet_id) {
    number <- formatC(sheet_id, width=num_digits, format="d", flag="0")
    path <- paste0(prefix, number, ".csv")
    write_csv(runsheet, path, na="")
  }
  
  map2(runsheets, seq(num_sheets), WriteRunSheet)
}

# test_sheets <- GenRunSheets("prototype.csv", "rangefile_example.csv")
# InspectNewRunSheets(test_sheets, "rangefile_example.csv")
# PlotNewRunSheets(test_sheets, "rangefile_example.csv")
# WriteRunSheets(test_sheets, "TEST_")
# 
# test_sheets_larger <- GenRunSheets("prototype.csv", "rangefile_example_larger.csv")
# PlotNewRunSheets(test_sheets_larger, "rangefile_example_larger.csv")
# WriteRunSheets(test_sheets_larger)
