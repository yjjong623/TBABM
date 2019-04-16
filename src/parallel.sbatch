#!/bin/sh

#SBATCH --time=00:30:00
#SBATCH --partition=general
#SBATCH --job-name=rangefile_medium
#SBATCH --output=rangefile_medium_output.txt
#SBATCH --ntasks=50
#SBATCH --cpus-per-task=4
#SBATCH --mem-per-cpu=750M

# Load the default version of GNU parallel.
module load parallel

# When running a large number of tasks simultaneously, it may be
# necessary to increase the user process limit.
# ulimit -u 10000

# This specifies the options used to run srun. The "-N1 -n1" options are
# used to allocates a single core to each task.
srun="srun -n1 -N1 --cpus-per-task $SLURM_CPUS_PER_TASK --exclusive"

# This specifies the options used to run GNU parallel:
#
#   --delay of 0.2 prevents overloading the controlling node.
#
#   -j is the number of tasks run simultaneously.
#
#   The combination of --joblog and --resume create a task log that
#   can be used to monitor progress.
#
parallel="parallel --delay 0.2 -j $SLURM_NTASKS --joblog rangefile_medium.log --resume"

mkdir -p rangefile_medium_output

# Run a script, runtask.sh, using GNU parallel and srun. Parallel
# will run the runtask script for the numbers 1 through 128. To
# illustrate, the first job will run like this:
#
#   srun --exclusive -N1 -n1 ./runtask.sh arg1:1 > runtask.1
#
# Arguments to tbabm.sh:
#   number of trajectories
#   size of threadpool
#   population size
#   name of parameter sheet
$parallel $srun ./tbabm.sh $SLURM_CPUS_PER_TASK {} '>' rangefile_medium_output/{}.txt \
    ::: $(seq -w 187)
