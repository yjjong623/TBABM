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
           "0-15"   = 0,
           "10-20"  = 10,
           "15-25"  = 15,
           "20-30"  = 20,
           "25-35"  = 25,
           "30-40"  = 30,
           "35-45"  = 35,
           "40-50"  = 40,
           "45-55"  = 45,
           "50-60"  = 50,
           "55-65"  = 65,
           "60-70"  = 60,
           "65-75"  = 65,
           "65-inf" = 65,
           "70-80"  = 70,
           "75-85"  = 75,
           "80-90"  = 80,
           "85-95"  = 85,
           "90-inf" = 90) 
  }
  
  map(vec, mapping) %>% map(as.integer) %>% unlist()
}

CategoryToGender <- function(vec) {
  mapping <- function(cat) switch(cat, "Male", "Female")
  
  map(vec, mapping) %>% unlist()
}