#!/bin/sh

#SBATCH --time=00:30:00
#SBATCH --partition=general
#SBATCH --job-name=parallel_test
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=2
#SBATCH --cpus-per-task=4

# Load the default version of GNU parallel.
module load parallel

# When running a large number of tasks simultaneously, it may be
# necessary to increase the user process limit.
# ulimit -u 10000

# This specifies the options used to run srun. The "-N1 -n1" options are
# used to allocates a single core to each task.
srun="srun --exclusive -N1 -n1 --cpus-per-task $SLURM_CPUS_PER_TASK"

# This specifies the options used to run GNU parallel:
#
#   --delay of 0.2 prevents overloading the controlling node.
#
#   -j is the number of tasks run simultaneously.
#
#   The combination of --joblog and --resume create a task log that
#   can be used to monitor progress.
#
parallel="parallel --link --delay 0.2 -j $SLURM_NTASKS --joblog runtask.log --resume"

mkdir bulk_output

# Run a script, runtask.sh, using GNU parallel and srun. Parallel
# will run the runtask script for the numbers 1 through 128. To
# illustrate, the first job will run like this:
#
#   srun --exclusive -N1 -n1 ./runtask.sh arg1:1 > runtask.1
#
# Arguments to tbabm.sh:
# 	number of trajectories
# 	size of threadpool
# 	population size
# 	name of parameter sheet
$parallel $srun ./tbabm.sh {1} $SLURM_CPUS_PER_TASK {2} {3} \
	'>' bulk_output/tbabm_{3}.txt \
	::: 4 \
	::: 1000 \
	::: {1..4}


# Note that if your program does not take any input, use the -n0 option to
# call the parallel command: 
#
#   $parallel -n0 "$srun ./run_noinput_task.sh > output.{1}" ::: {1..128}
