#!/bin/sh

# $1 is arg1:{1} from GNU parallel.
#
# $PARALLEL_SEQ is a special variable from GNU parallel. It gives the
# number of the job in the sequence.
#
# Here we print the sleep time, host name, and the date and time.
echo param $1 seq:$PARALLEL_SEQ host:$(hostname) date:$(date)

container_exec="singularity exec ../../../containers_latest.sif"

$container_exec ./TBABM -t $1 -m $2 -n $3 -p params_$4 -o results_$4/

