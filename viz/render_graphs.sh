#!/bin/sh

# $1 is arg1:{1} from GNU parallel.
#
# $PARALLEL_SEQ is a special variable from GNU parallel. It gives the
# number of the job in the sequence.
#
# Here we print the sleep time, host name, and the date and time.
echo param $1 seq:$PARALLEL_SEQ host:$(hostname) date:$(date)

module load R

grid_exec="Rscript R/grid_gen.R"

$grid_exec $@
