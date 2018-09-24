CSVLoaderGen <- function(prefix) {
  function(name) read.csv(paste(prefix, name, ".csv", sep="")) %>% tbl_df()
}

FindLatestTimestamp <- function(dir) {
  fileWithBiggestTimestamp <- list.files(dir) %>% tail()
  
  # matching beginning of string, then 0-9
  pattern <- "\\d+"
  string <- fileWithBiggestTimestamp
  
  regmatches(string, regexpr(pattern, string))[1]
}