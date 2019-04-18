#!/bin/sh

#SBATCH --time=00:10:00
#SBATCH --partition=general
#SBATCH --job-name=rf_progression
#SBATCH --output=rf_progression_output.txt
#SBATCH --ntasks=40
#SBATCH --cpus-per-task=4
#SBATCH --mem-per-cpu=750M
#SBATCH --mail-type=ALL
#SBATCH --mail-user=marcus.russi@yale.edu

rangefile_title=${1?"Usage: parallel.sbatch RANGEFILE_TITLE N_RUNS"}
n_runs=${2?"Usage: parallel.sbatch RANGEFILE_TITLE N_RUNS"}
n_jobs=${SLURM_NTASKS-2}
n_threads=${SLURM_CPUS_PER_TASK-2}

echo $rangefile_title $n_runs $n_jobs $n_threads

# Load the default version of GNU parallel.
module load foss/2018b
module load parallel

# When running a large number of tasks simultaneously, it may be
# necessary to increase the user process limit.
# ulimit -u 10000

# This specifies the options used to run srun. The "-N1 -n1" options are
# used to allocates a single core to each task.
srun="srun -n1 -N1 --cpus-per-task $n_threads --exclusive"

# This specifies the options used to run GNU parallel:
#
#   --delay of 0.2 prevents overloading the controlling node.
#
#   -j is the number of tasks run simultaneously.
#
#   The combination of --joblog and --resume create a task log that
#   can be used to monitor progress.
#
parallel="parallel --verbose --delay 0.2 -j $n_jobs --joblog $rangefile_title.log --resume"

mkdir -p $rangefile_title\_output

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
$parallel $srun ./tbabm.sh $n_threads {} $rangefile_title '>' $rangefile_title\_output/{}.txt \
    ::: $(seq -w $n_runs)
