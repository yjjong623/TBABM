cat$ps

attachBy <- function(l, by, ...) {
  r <- list(...)
  if (length(r) == 1)
    left_join(l, r[[1]], by) 
  else
    joined <- left_join(l, r[[1]], by)
  do.call(attachBy, c(list(joined), list(by), r[-1]))
}

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

mean(hiv_stats$time.to.AIDS, na.rm=TRUE)

attachBy(cat$ps, "hash", artInit)


tryGamma <- function(shape, rate) {
  xs <- seq(0, 20, 0.05)
  
  table <- data.frame(x=xs, density=dgamma(xs, shape=shape, rate=rate), cumulative=pgamma(xs, shape=shape, rate=rate))
  
  ggplot(table, aes(x)) + 
    geom_line(aes(y=density, color="Density")) + 
    geom_line(aes(y=cumulative, color="Cumulative")) +
    geom_hline(yintercept = 0.5) +
    scale_x_continuous(breaks=seq(0,20,1)) + 
    lims(y=c(0,1.0))
}

tryGamma(1,1)

betterDeath <- naturalDeath %>% gather("year", "parameter-1", 3:33) %>%
  transmute(year = substr(year,2,5),
            sex = recode(Sex, Male="M", Female="F"),
            age = Age,
            distribution = map_chr(Sex, ~"Exponential"),
            `parameter-description` = map_chr(Sex, ~"(rate-shift)"),
            `parameter-1` = `parameter-1`,
            `parameter-2` = map_chr(Sex, ~0),
            `parameter-3` = map_chr(Sex, ~""),
            `parameter-4` = map_chr(Sex, ~"")) %>%
  arrange(desc(sex))

write_csv(betterDeath, "naturalDeath_new.csv")


betterDeath %>%
  ggplot(aes(age, `parameter-1`, group=interaction(sex,year), color=year)) + geom_line()


as.tibble(list(x=seq(0,1400,1), 
               y=dgamma(seq(0,1400,1), shape=2, scale=160))) %>%
  ggplot(aes(x, y)) + 
  geom_line() +
  geom_vline(xintercept = c(350, 700), color="blue")

################################################################
# SIMULATOR!
################################################################

riskFun     <- function(CD4, m, p) m*exp(-p*CD4)
CD4         <- function(initCD4, t, m, k, a) initCD4*exp(-m*(1+k)^((a-45)/15)*t)

timeOfDeath <- function(initCD4, risk_m, risk_p, m, k_sh, k_sc, a, ts=seq(0,100,5)) {
  CD4s    <- CD4(initCD4, ts, m, rgamma(1, shape=k_sh, scale=k_sc), a)
  risks   <- riskFun(CD4s, risk_m, risk_p)
  samples <- rexp(length(ts), risks)
  dies    <- samples < 5
  
  first(ts[dies==TRUE]) + first(samples[dies==TRUE])
}

# MASS::truehist(replicate(10000,timeOfDeath(1000, risk_m=0.4, risk_p=0.013, m=0.17, k_sh=45, k_sc=0.0085, a=32)))

with(
  {
    risk_m <- 0.62
    risk_p <- 0.0028
    m      <- 0.171
    k_sh   <- 45
    k_sc   <- 0.0085
    n_ppl  <- 10000
    age_dist <- partial(runif, min=15, max=75)
    age_grps <- c(15, 25, 35, 45, 1e3)
    CD4_dist <- partial(rlnorm, meanlog=7, sdlog=0.32)
    people   <- tibble(init.CD4 = CD4_dist(n_ppl),
                       age      = age_dist(n_ppl))
  },
  mutate(people,
         time.of.death = map2_dbl(init.CD4, age, ~timeOfDeath(.x, risk_m, risk_p, m, k_sh, k_sc, .y)),
         age.group     = cut(age, age_grps, include.lowest=TRUE)
  ) %>%
    group_by(age.group) %>%
    summarize(tod.mean   = mean(time.of.death),
              tod.median = median(time.of.death))
)

stub <- function(risk_m, risk_p) with(
  {
    risk_m <- risk_m
    risk_p <- risk_p
    m      <- 0.171
    k_sh   <- 100
    k_sc   <- 0.0085
    n_ppl  <- 2000
    age_dist <- partial(runif, min=15, max=75)
    age_grps <- c(15, 25, 35, 45, 1e3)
    CD4_dist <- partial(rlnorm, meanlog=7, sdlog=0.32)
    people   <- tibble(init.CD4 = CD4_dist(n_ppl),
                       age      = age_dist(n_ppl))
  },
  mutate(people,
         time.of.death = map2_dbl(init.CD4, age, ~timeOfDeath(.x, risk_m, risk_p, m, k_sh, k_sc, .y)),
         age.group     = cut(age, age_grps, include.lowest=TRUE)
  ) %>%
    group_by(age.group) %>%
    summarize(tod.mean   = mean(time.of.death),
              tod.median = median(time.of.death),
              risk_m     = risk_m,
              risk_p     = risk_p)
)

simulator <- function(m_lo, m_hi, p_lo, p_hi, n) {
  map2(runif(n, m_lo, m_hi), runif(n, p_lo, p_hi), ~stub(.x, .y)) %>%
    reduce(union_all)
}

# pretty good: 0.53, 0.63, 0.0029, 0.0033  with k_sh=45, k_sc=0.0085
# best we got: 0.55, 0.62, 0.0024, 0.00262 with k_sh = 100, k_sc= 0.0085
simulator(0.55, 0.61, 0.0024, 0.00262, 50) %>%
  ggplot(aes(age.group, 
             tod.mean, 
             color=interaction(risk_m, risk_p), 
             group=interaction(risk_m, risk_p))) + 
  geom_line() + 
  lims(y=c(0, 15))

################################################################
# END SIMULATOR!
################################################################


# %>% ggplot(aes(CD4, time.to.death, color=interaction(AIDS, time.to.death < 5 & AIDS))) +
# geom_jitter()

hivppl <- cat$ps %>% 
  group_by(hash) %>%
  summarize(HIV.date = first(time[HIV == "true"])) %>%
  filter(!is.na(HIV.date))

good_stuff <- inner_join(cat$ps, hivppl, by="hash") %>%
  mutate(time_ = time - HIV.date) %>%
  group_by(hash) %>%
  mutate(lastObs = max(time),
         initial.CD4 = first(CD4[HIV=="true"]),
         CD4 = ifelse(HIV=="true", CD4, initial.CD4))

ppl <- good_stuff %>% pull(hash) %>% unique() %>% sample(200)

# Graph of CD4 decline and AIDS development
good_stuff %>% 
  filter(hash %in% ppl) %>%
  ggplot(aes(time_/365, CD4, color=interaction(CD4 < 350, ART, HIV), group=hash)) +
    geom_line(lineend="butt") +
    geom_hline(yintercept = 350) +
    geom_hline(yintercept = 100)

# Graph of death risk from HIV
good_stuff %>%
  filter(hash %in% ppl) %>%
  ggplot(aes(time_/365, 0.57*exp(-0.0025*CD4), color=interaction(CD4 < 350, ART, HIV), group=hash)) +
  geom_line(lineend="butt") #+
  # geom_hline(yintercept = 350) +
  # geom_hline(yintercept = 100)

# Distributions for time to CD4<350,100
time.to.AIDS <- good_stuff %>%
  group_by(hash) %>%
  summarize(
    time.to.350 = first(time_[CD4 < 350]/365),
    time.to.100 = first(time_[CD4 < 100]/365)
  )

time.to.AIDS %>%
  ggplot(aes(time.to.350)) +
  geom_histogram(binwidth = 1) +
  lims(x=c(0,20))

# This is a plot of HIV-positive people who have died during the simulation of either
# natural causes, or HIV. It shows, by age group (age at infection), how long these
# individuals have lived with their infection
cat$ds %>%
  filter(HIV == "true") %>%
  mutate(year.of.death = cut(time/365, seq(0, 50, 1), include.lowest=TRUE, labels=FALSE) + 1990,
         years.of.infection = (time-HIV_date)/365,
         age.group = cut(age-HIV_date/365, c(15, 25, 35, 45, 1e6), include.lowest = TRUE)) %>%
  group_by(year.of.death, cause, age.group) %>%
  summarize(
    years.of.infection = mean(years.of.infection),
    count=n()
  ) %>%
  ggplot(aes(year.of.death, years.of.infection, color=cause, size=count)) +
  geom_point() +
  facet_wrap(~age.group)

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

annotateForHIV <- function(ds) {
  filter(ds, HIV=="true") %>%
    mutate(
      year.of.death = cut(time/365, seq(0, 50, 1), include.lowest=TRUE, labels=FALSE) + 1990,
      years.of.infection = (time-HIV_date)/365,
      age.group = cut(age-HIV_date/365, c(15, 25, 35, 45, 1e6), include.lowest=TRUE)
    )
}

annotateForHIV(cat$ds) %>%
  group_by(age.group) %>%
  summarize(mean.years = mean(years.of.infection))
