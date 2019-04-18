#!/bin/sh

#SBATCH --time=00:10:00
#SBATCH --partition=general
#SBATCH --job-name=grid_render
#SBATCH --output=grid_render_output.txt
#SBATCH --ntasks=40
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=500M
#SBATCH --mail-type=ALL
#SBATCH --mail-user=marcus.russi@yale.edu

rangefile_title=${1?"Usage: parallel_render_graphs.sbatch RANGEFILE_TITLE RANGEFIEL_NRUNS"}
n_runs=${2?"Usage: parallel_render_graphs.sbatch RANGEFILE_TITLE RANGEFIEL_NRUNS"}
n_jobs=${SLURM_NTASKS-2}
n_threads=${SLURM_CPUS_PER_TASK-1}

# Load the default version of GNU parallel.
module load foss/2018b
module load parallel

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
parallel="parallel --verbose --delay 0.2 -j $n_jobs --joblog "$rangefile_title"_grid_render.log --resume -X"

output_dir="../output"
results_dir_pfx=$rangefile_title"_"

image_dir="./"$rangefile_title"_images"
compressed_images_fname=$rangefile_title"_images.tar.gz"

mkdir -p $image_dir

seq -w $n_runs | $parallel $srun ./render_graphs.sh $output_dir/$results_dir_pfx{}/
seq -w $n_runs | parallel cp $output_dir/$results_dir_pfx{}/TBgrid.png $image_dir/TBgrid_{}.png
tar -zcvf $compressed_images_fname $image_dir
rm -rf $image_dir
