library(ggplot2)

################################################
# HOUSEHOLD SURVEY GRAPHS
################################################

meanSize <- function(hs) {
  hs %>%
  group_by(time) %>%
  summarize(
    medianSize    = median(size),
    meanSize      = mean(size),
    maxSize       = max(size),
    spousePresent = mean(spouse),
    meanOffspring = mean(directOffspring),
    meanOther     = mean(other)
  ) %>%
  ggplot(aes(time)) +
  geom_line(aes(y=meanSize, color="Mean")) +
  geom_line(aes(y=medianSize, color="Median")) +
  geom_line(aes(y=maxSize, color="Max")) +
  labs(x="Time (days)", y="Mean household size") +
  scale_y_continuous(breaks=seq(0,10,1), limits=c(0,10))
}

nHouseholds <- function(hs) {
  hs %>%
  group_by(time, trajectory) %>%
  count() %>%
  ggplot(aes(time, n, color=trajectory, group=trajectory)) +
  geom_line()
}


sizeHist <- function(hs) {
  hs %>%
  filter(((time-1)/365) %% 5 == 0) %>%
  ggplot(aes(size)) +
  facet_wrap(~(time-1)/365) +
  geom_histogram(aes(y=..density..), 
                 binwidth=1) +
  xlim(0,20) +
  theme_bw() +
  labs(x="Household size",
       y="Density")
}

childrenVsOthers <- function(hs) {
  hs %>%
  filter(size >= 0 & size <= 10) %>%
  group_by(time, size) %>%
  summarize(meanKids = mean(directOffspring + otherOffspring),
            meanOther = mean(other))  %>%
  ggplot(aes(time)) +
  facet_wrap(~size) +
  geom_line(aes(y=meanKids, color="Offspring")) +
  geom_line(aes(y=meanOther, color="Other"))
}