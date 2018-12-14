data <- read_csv("~/Desktop/thembisa_incidence_data.csv")

# Remove NA rows
data <- data %>% filter(!is.na(`M15-24`))

# Transpose
data <- gather(data, "sex-age", "incidence", 2:9)

mapping <- list(
  "0" = "0-14",
  "5" = "0-14",
  "10" = "0-14",
  "15" = "15-24",
  "20" = "15-24",
  "25" = "25-49",
  "30" = "25-49",
  "35" = "25-49",
  "40" = "25-49",
  "45" = "25-49",
  "50" = "50-100"
)

translator <- function(year, sex, age) {
  age_group <- mapping[as.character(age)]
  pmap_dbl(list(year,sex,age_group), 
           ~filter(data, Year %in% ..1, `sex-age` %in% paste0(..2, ..3))$incidence)
}

demographics <- tibble(
  year = 1985:2025 %>% map(~rep.int(., 11)) %>% flatten_dbl() %>% rep(2),
  sex = c(rep("M", 41*11), rep("W", 41*11)),
  age = seq(0, 50, 5) %>% rep(82)
)

reformatted <- demographics %>% mutate(
  distribution = "Bernoulli",
  `parameter-description` = "p",
  `parameter-1` = translator(year, sex, age),
  `parameter-2` = "",
  `parameter-3` = "",
  `parameter-4` = "",
)

reformatted %>%
  ggplot(aes(year, `parameter-1`, color=interaction(sex, age), group=interaction(sex, age))) + geom_line() + facet_wrap(~sex)
