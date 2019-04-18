source("R/graph_catalog.R")

GraphAndSaveRun <- function(folder_name) {
  cat <- CreateGraphCatalog(folder_name)
  
  slash <- ifelse(folder_name[length(folder_name)] == '/', "", "/")
  
  filename <- paste0(folder_name, slash, 'TBgrid.png')
  
  # Set up a place to render the graph to
  png(filename=filename, width=1200, height=800, units="px")
  
  # Render the grid
  cat$tbGrid()
  
  # Save the grid
  dev.off()
}

main <- function(args) {
  if (length(args))
    
  stat <- file.info(args)
  
  if (stat$isdir != TRUE) {
    printf("ERROR: One of the arguments is not a directory. Exiting.")
    printf("USAGE: [dir1]+ where dir1 is a directory containing results from 1 model run")
    quit(status=1)
  }
  
  walk(args, GraphAndSaveRun)
}

main(commandArgs(trailingOnly=TRUE))