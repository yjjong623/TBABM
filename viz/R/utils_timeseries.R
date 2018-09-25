library(dplyr)

JoinTimeSeries <- function(x, y, preserveNames=FALSE) {
  xName <- enquo(x) %>% quo_name()
  yName <- enquo(y) %>% quo_name()
  
  by <- c("period", "trajectory")
  joined <- left_join(x, y, by, copy=TRUE)
  
  if (preserveNames) {
    joined <- rename(joined, 
                     !!xName := value.x, 
                     !!yName := value.y)
  }
  
  joined
}

JoinAndDivideTimeSeries <- function(num, den, newColName, scale = 1) {
  JoinTimeSeries(num, den) %>%
    mutate(
      !!newColName := scale*value.x/value.y,
      value.x = NULL,
      value.y = NULL
    )
}

RateTimeSeriesPerN <- function(n, d, name, scale) JoinAndDivideTimeSeries(n, d, name, scale)

AgeRangeToNum <- function(vec) {
  mapping <- function(str) {
    switch(as.character(str),
           "0-10"   = 0,
           "10-20"  = 10,
           "20-30"  = 20,
           "30-40"  = 30,
           "40-50"  = 40,
           "50-60"  = 50,
           "60-70"  = 60,
           "70-80"  = 70,
           "80-90"  = 80,
           "90-inf" = 90) 
  }
  
  map(vec, mapping) %>% map(as.integer) %>% unlist()
}

CategoryToGender <- function(vec) {
  mapping <- function(cat) switch(cat, "Male", "Female")
  
  map(vec, mapping) %>% unlist()
}