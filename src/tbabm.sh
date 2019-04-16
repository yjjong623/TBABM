#!/bin/sh

# $1 is arg1:{1} from GNU parallel.
#
# $PARALLEL_SEQ is a special variable from GNU parallel. It gives the
# number of the job in the sequence.
#
# Here we print the sleep time, host name, and the date and time.
echo param $1 seq:$PARALLEL_SEQ host:$(hostname) date:$(date)

container_exec="singularity exec ../../../containers_latest.sif"

n_trajectories=4
n_threads=$1
n_people=31500

$container_exec ./TBABM -t $n_trajectories \
                        -m $n_threads \
                        -n $n_people \
                        -p rangefile_medium_$2 \
                        -o rangefile_medium_results_$2/
