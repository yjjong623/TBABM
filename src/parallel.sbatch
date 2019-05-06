#!/bin/sh

#SBATCH --time=00:40:00
#SBATCH --partition=general
#SBATCH --job-name=rf_initialization_wider
#SBATCH --output=rf_initialization_wider_output.txt
#SBATCH --ntasks=40
#SBATCH --cpus-per-task=4
#SBATCH --mem-per-cpu=750M
#SBATCH --mail-type=ALL
#SBATCH --mail-user=marcus.russi@yale.edu
E_RUNERR=65

# Load dependencies
if which -s module; then
	module load foss/2018b || { (>&2 echo 'Loading foss/2018b failed; exiting'); exit $E_RUNERR; };
	module load parallel   || { (>&2 echo 'Loading parallel failed; exiting');   exit $E_RUNERR; };
else
	which -s parallel || { (>&2 echo '"parallel" not found in $PATH; exiting'); exit $E_RUNERR; };
fi

usage_string="Usage: parallel.sbatch RANGEFILE_TITLE N_RUNS"

rangefile_title=${1?$usage_string}

n_runs=${2?$usage_string}
n_jobs=${SLURM_NTASKS-`getconf _NPROCESSORS_ONLN`}
n_threads=${SLURM_CPUS_PER_TASK-2}

echo "Running $rangefile_title for $n_runs runs using $n_jobs jobs on $n_threads threads/process"

if ! seq -w $n_runs | parallel stat {}.json > /dev/null; then
	echo "Could not stat 1 or more of the runsheets; exiting";
	exit 1;
fi

if which -s srun; then
	# This specifies the options used to run srun. The "-N1 -n1" options are
	# used to allocates a single core to each task.
	srun="srun -n1 -N1 --cpus-per-task $n_threads --exclusive";
else
	srun="";
fi

# This specifies the options used to run GNU parallel:
#
#   --delay of 0.2 prevents overloading the controlling node.
#
#   -j is the number of tasks run simultaneously.
#
#   The combination of --joblog and --resume create a task log that
#   can be used to monitor progress.
#
parallel="parallel --verbose -j $n_jobs --delay 0.2 --joblog parallel.log"

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
seq -w $n_runs | $parallel $srun ./tbabm.sh $n_threads {} '$RANDOM' '>' {}.log '2>&1'
