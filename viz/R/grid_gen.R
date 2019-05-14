source("R/graph_catalog.R")

initial.dir<-getwd()
print(initial.dir)
quit()

GraphAndSaveRun <- function(folder_name) {
  cat <- CreateGraphCatalog(folder_name)
  slash <- ifelse(folder_name[length(folder_name)] == '/', "", "/")
  filename_prefix <- paste0(folder_name, slash)
  
  grids <- c("demographicGrid", "hivGrid", "tbGrid")
  
  display_4k     <- list(name="lg", ppi=157, width=19, height=12.5, maxCols=6)
  display_laptop <- list(name="sm", ppi=135, width=9.5, height=8, maxCols=3)
  display_retina <- list(name="sm_2x", ppi=220, width=9.5, height=8, maxCols=4)
  
  displays <- list(display_4k, display_laptop, display_retina)
  
  thingsToRender <- list(grid=grids, display=displays) %>% cross()
  
  render <- function(spec) {
    print(spec)
    filename <- paste0(filename_prefix, spec$grid, "_", spec$display$name, ".png")
    
    ncols = min(4, spec$display$maxCols)
    nrows = max(3, ceiling(12/ncols))
  
    # Set up a place to render the graph to  
    png(filename=filename,
        width=spec$display$width,
        height=spec$display$height,
        units="in",
        res=spec$display$ppi,
        bg="transparent")
    
    # Render the grid
    cat[[spec$grid]](cols=ncols, rows=nrows)
    
    # Save the grid
    dev.off()
  }
  
  walk(thingsToRender, render)
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
