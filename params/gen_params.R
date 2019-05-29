suppressPackageStartupMessages(library(tidyverse, quietly=TRUE, 
                   verbose=FALSE,
                   warn.conflicts=FALSE))
library(tools)
library(jsonlite)

# Spec for the RunSheet
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

# Spec for the rangefile
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
  
  # Given a list of substitutions, outputs a new parameter sheet, as a tibble, which
  # includes these substitutions
  # 'substitutions" is a keyed list, containing 'short-name':'parameter-1' pairs.
  # 
  # Right now, substitution can only be done on 'parameter-1', though this may change
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

# For each generated RunSheet, create a list of the parameters in that RunSheet
# that were in the set of parameters that the rangefile designated as variable
InspectNewRunSheets <- function(NewRunSheets, rangefile_fname) {
  substitutions <- read_csv(rangefile_fname, col_types=datasheet_spec) %>% as_tibble()
  
  map(NewRunSheets, ~.[which(.$`short-name` %in% substitutions$name), c('short-name', 'parameter-1')])
}

# Given a list of new RunSheets, and the filename of the rangefile
# that generated them, create a line graph of all of the parameters that
# are being varied, where X is the name of the parameter being varied, and
# Y is its value in each of the RunSheetss
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
  
  # Determine the number of digits needed to represent the ID of the
  # last RunSheet
  num_sheets <- length(runsheets)
  num_digits <- floor(log10(num_sheets)) + 1
  
  WriteRunSheet <- function(runsheet, sheet_id) {
    
    # Format the RunSheet ID: add an appropriate amount of leading 0's
    number <- formatC(sheet_id, width=num_digits, format="d", flag="0")
    
    # Generate paths to write .csv and .json file to
    path_csv <- paste0(prefix, number, ".csv")
    path_json <- paste0(prefix, number, ".json")
    
    # Write the .csv and .json files to disk. It's important to make sure NA is
    # represented as the empty string, so that an empty cell is represented as
    # ",,".
    write_csv(runsheet, path_csv, na="")
    write_json(runsheet, path_json, na='null')
  }
  
  map2(runsheets, seq(num_sheets), WriteRunSheet)
}

main <- function(args) {
  n_args <- length(args)
  n_rangefiles <- n_args - 1
  
  if (n_args < 2) {
    print("Usage: PROTOTYPE_FILE [RANGEFILE]+")
    return();
  }
  
  proto_fname <- args[1]
  rangefiles_fnames <- args[-1]
  
  # GenPrefix <- function(rangefile_fname) 
  #                paste0(tools::file_path_sans_ext(rangefile_fname), "_")
  GenPrefix <- function(rangefile_fname) return("")
  
  runsheet_sets <- map(rangefiles_fnames, ~GenRunSheets(proto_fname, .))
  runsheet_prefixes <- map(rangefiles_fnames, GenPrefix)
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
