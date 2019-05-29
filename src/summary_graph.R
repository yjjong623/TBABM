# Creates a summary graph of each TimeSeries included in the statistical
# summary of a batch of runs. Each panel corresponds to some quantity
# being tracked by a TimeSeries, and one panel contains every trajectory's
# data for that quantity.
# 
# Expects: a 'results.csv' file where the Column 1 is trajectory-id ('0001', 
# '0523', etc), and Column 2 is a 'type' ('tbTreatmentExperienced', etc).
# Also expects that the corresponding .csv file for each 'type' and
# 'trajectory-id' is stored inside a folder of the same name as 'trajectory-id',
# and the name of this file is simply 'type'.csv
# 
# Possible improvements:
# - Speeding up bottleneck of appending all the timeseries together. This
#   could be done multithreaded, but it could probably also be done in
#   the shell
# - CLI arguments of the form TS:type PTS:type IPTS:type, etc, so that
#   there is no need to have a 'results.csv' file prepared already
# - Fitting it into a larger system that does tons of graphing after a model
#   run (probably involving an R package)

library(tidyverse)

timeseries_spec = cols(period=col_integer(), 
				       trajectory=col_double(),
				       value=col_integer())

summary_spec = cols(trajectory=col_character(),
					type=col_character(),
					min=col_double(),
					max=col_double(),
					mean=col_double(),
					median=col_double())

summary_data <- read_csv('results.csv', col_types=summary_spec)

ObtainTimeSeries <- function(summary_row) {
	# First two columns are trajectory ID (not seed), and type
	trajectory_id <- summary_row[1]
	type <- summary_row[2]

	fname <- paste0(trajectory_id, '/', type, '.csv')
	ts <- read_csv(fname, col_types=timeseries_spec)

	mutate(ts, trajectory_id=trajectory_id, type=type)
}

TBData <- apply(summary_data, 1, ObtainTimeSeries) %>% bind_rows()

ggplot(TBData, aes(period, value, group=trajectory_id)) + 
	geom_line(alpha=0.07) + 
	facet_wrap(~type, scale='free_y')

ggsave('summary.png', device=png(), width=24, height=13.5, units='in', dpi=160)
ggsave('summary_printable.png', device=png(), width=10.2, height=7.9, units='in', dpi=160)
