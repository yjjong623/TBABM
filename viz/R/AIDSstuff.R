# Join multiple data frames
attachBy <- function(l, by, ...) {
  r <- list(...)
  if (length(r) == 1)
    left_join(l, r[[1]], by) 
  else
    joined <- left_join(l, r[[1]], by)
  do.call(attachBy, c(list(joined), list(by), r[-1]))
}

# Combie the filtering and grouping operations
filterAndGroup <- function(d, filter_, grouping_) {
  filter_ <- enquo(filter_)
  grouping_ <- enquo(grouping_)
  
  filter(d, !!filter_) %>%
    group_by(!!grouping_)
}

GroupByIdv <- filterAndGroup %>% partial(grouping_ = hash)

AllPeople <-  function(ps) GroupByIdv(ps, filter_ = time > 0)
HIVPeople <-  function(ps) GroupByIdv(ps, filter_ = HIV == "true")
ARTPeople <-  function(ps) GroupByIdv(ps, filter_ = ART == "true")
AIDSPeople <- function(ps) GroupByIdv(ps, filter_ = HIV == "true" & CD4 <= 350)

hiv_stats <- HIVPeople(cat$ps) %>% summarize(
  time.of.infection = min(time),
  CD4.at.infection = first(CD4),
  enrolls.in.ART = any(ART == "true"),
  time.of.ART = ifelse(enrolls.in.ART, time[ART == "true"], NA),
  CD4.at.ART.enrollment = ifelse(enrolls.in.ART, CD4[time.of.ART], NA),
  develops.AIDS = any(CD4 <= 350),
  time.of.AIDS = ifelse(develops.AIDS, first(time[CD4 <= 350 & HIV == "true"]), NA),
  time.to.AIDS = ifelse(develops.AIDS,
                        (time.of.AIDS-time.of.infection)/365.,
                        NA)
)

################################################################
# SIMULATOR!
################################################################

riskFun     <- function(CD4, m, p) m*exp(-p*CD4)
CD4         <- function(initCD4, t, m, k, a) initCD4*exp(-m*(1+k)^((a-80)/15)*t)

timeOfDeath <- function(initCD4, risk_m, risk_p, m, k, a, ts=seq(0,60,1/12)) {
  CD4s    <- CD4(initCD4, ts, m, k, a)
  risks   <- riskFun(CD4s, risk_m, risk_p)
  samples <- rexp(length(ts), risks)
  dies    <- samples < 1/12
  
  first(ts[dies==TRUE]) + first(samples[dies==TRUE])
}

stub <- function(risk_m, risk_p) with(
  {
    risk_m <- risk_m
    risk_p <- risk_p
    m      <- 0.6 # originally: 0.171
    k_sh   <- 100
    k_sc   <- 0.0025
    n_ppl  <- 10000
    age_dist <- partial(runif, min=15, max=75)
    age_grps <- c(15, 25, 35, 45, 1e3)
    CD4_dist <- partial(rlnorm, meanlog=7, sdlog=0.32)
    people   <- tibble(init.CD4 = CD4_dist(n_ppl),
                       age      = age_dist(n_ppl))
  },
  mutate(people,
         k             = rgamma(n_ppl, shape=k_sh, scale=k_sc),
         time.of.death = pmap_dbl(list(init.CD4, k, age), ~timeOfDeath(..1, risk_m, risk_p, m, ..2, ..3)),
         CD4.at.death  = CD4(init.CD4, time.of.death, m, k, age),
         age.group     = cut(age, age_grps, include.lowest=TRUE)) %>%
    group_by(age.group) %>%
    summarize(tod.mean   = mean(time.of.death),
              tod.median = median(time.of.death),
              CD4.mean   = mean(CD4.at.death),
              CD4.median = median(CD4.at.death),
              CD4.sd     = sd(CD4.at.death),
              risk_m     = risk_m,
              risk_p     = risk_p)
)

simulator <- function(m_lo, m_hi, p_lo, p_hi, n) {
  map2(runif(n, m_lo, m_hi), runif(n, p_lo, p_hi), ~stub(.x, .y)) %>%
    reduce(union_all)
}

goals <- tibble(
  age.group = c("[15,25]", "(25,35]", "(35,45]", "(45,1e+03]"),
  targets = c(11.7, 10.5, 9.5, 6.3)
)

simulator(0.15, 0.16, 0.003, 0.0031, 50) %>%
  ggplot(aes(age.group, 
             tod.mean, 
             color=interaction(risk_m, risk_p), 
             group=interaction(risk_m, risk_p))) + 
  geom_line() + 
  geom_point(data=goals, aes(age.group, targets), inherit.aes=FALSE) +
  lims(y=c(0, 15))

################################################################
# END SIMULATOR!
################################################################




# Distributions for time to CD4<350,100
time.to.AIDS <- good_stuff %>%
  group_by(hash) %>%
  summarize(
    time.to.200 = first(time_[CD4 < 200]/365),
    time.to.100 = first(time_[CD4 < 100]/365)
  )

time.to.AIDS %>%
  ggplot(aes(time.to.200)) +
  geom_histogram(binwidth = 1) +
  lims(x=c(0,20))


cat$ps %>%
  filter(hash %in% (sample_n(cat$ds, 500) %>% pull(hash))) %>%
  mutate(age.group = cut())

everybodyWhoGetsHIV <- function(ps) ps %>% filter(HIV=="true") %>% pull(hash) %>% unique()

sampleHIVPeopleByAgeGroup <- function(ps, n) {
  grouped <- filter(ps, hash %in% everybodyWhoGetsHIV(ps)) %>%
    group_by(hash) %>%
    mutate(age.at.infection = first(age[HIV=="true"]),
           age.group = cut(age.at.infection, c(15, 25, 35, 45, 1e6), include.lowest = TRUE)) %>%
    ungroup() %>%
    group_by(age.group)

  hashes <- grouped %>%
    sample_n(n) %>%
    pull(hash)

  filter(grouped, hash %in% hashes)
}

sampleHIVPeopleByAgeGroup(cat$ps, 10) %>%
  ggplot(aes(time, CD4)) + geom_jitter() + facet_wrap(~age.group)