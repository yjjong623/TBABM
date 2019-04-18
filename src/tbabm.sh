#!/bin/sh

echo param $1 seq:$PARALLEL_SEQ host:$(hostname) date:$(date)

container_exec="singularity exec ../../../containers_latest.sif"

n_threads=$1
run_id=$2
rangefile_name=$3
n_trajectories=4
n_people=32000

$container_exec ./TBABM -t $n_trajectories \
                        -m $n_threads \
                        -n $n_people \
                        -p $rangefile_name\_$run_id \
                        -o $rangefile_name\_$run_id/
