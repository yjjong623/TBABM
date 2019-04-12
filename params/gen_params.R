suppressPackageStartupMessages(library(tidyverse, quietly=TRUE, 
                   verbose=FALSE,
                   warn.conflicts=FALSE))
library(tools)

datasheet_spec <- cols(
  description = col_character(),
  `short-name` = col_character(),
  type = col_character(),
  distribution = col_character(),
  `parameter-description` = col_character(),
  `parameter-1` = col_double(),
  `parameter-2` = col_double(),
  `parameter-3` = col_double(),
  `parameter-4` = col_double(),
  `included-in-calibration` = col_logical()
)

rangefile_spec <- cols(
  name = col_character(),
  lower = col_double(),
  upper = col_double(),
  step = col_double()
)

GenRunSheets <- function(proto_fname, rangefile_fname) {
  
  # Read the prototype file and the substitutions file in and convert them to
  # tibbles
  proto <- read_csv(proto_fname, col_types=datasheet_spec) %>% as_tibble()
  substitutions <- read_csv(rangefile_fname, col_types=rangefile_spec) %>% as_tibble()
  
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
  substitutions <- read_csv(rangefile_fname, col_types=datasheet_spec) %>% as_tibble()
  
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

# print(commandArgs(trailingOnly = TRUE))
main <- function(args) {
  n_args <- length(args)
  n_rangefiles <- n_args - 1
  
  if (n_args < 2) {
    print("Usage: PROTOTYPE_FILE [RANGEFILE]+")
    return();
  }
  
  proto_fname <- args[1]
  rangefiles_fnames <- args[-1]
  
  GenPrefix <- function(rangefile_fname) 
                 paste0(tools::file_path_sans_ext(rangefile_fname), "_")
  
  runsheet_sets <- map(rangefiles_fnames, ~GenRunSheets(proto_fname, .))
  runsheet_prefixes <- map(rangefiles_fnames, GenPrefix)
  # print(runsheet_prefixes)
  runsheets_written <- map2(runsheet_sets, runsheet_prefixes,
                            ~WriteRunSheets(.x, prefix=.y))
}

main(commandArgs(trailingOnly=TRUE))


# for(i in c(-1:3, 9)) print(switch(i, 1,2,3,4))

# test_sheets <- GenRunSheets("prototype.csv", "rangefile_example.csv")
# InspectNewRunSheets(test_sheets, "rangefile_example.csv")
# PlotNewRunSheets(test_sheets, "rangefile_example.csv")
# WriteRunSheets(test_sheets, "TEST_")
#
# test_sheets_larger <- GenRunSheets("prototype.csv", "rangefile_example_larger.csv")
# PlotNewRunSheets(test_sheets_larger, "rangefile_example_larger.csv")
# WriteRunSheets(test_sheets_larger)
