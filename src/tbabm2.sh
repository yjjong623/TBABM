#!/bin/sh           

function print_usage {
  echo "Usage:"
  echo "  TBABM -h"
  echo "    Print usage information and exit"
  echo "  TBABM [options] -r rangefile -n num_runsheets"
  echo "    Run TBABM on 'num_runsheets' from 'rangefile'"
  echo
  echo "Options:"
  echo "  -b name       Name of the run"
  echo "  -e            Email when job is finished. Implies -q"
  echo "  -g            Graph the output"
  echo "  -i init_size  Initial size of population. Default 31000"
  echo "  -q            Run as a batch job through sbatch"
  echo "  -t num        Number of trajectories per runsheet. Default 4"
  echo "  -m num        Number of threads per process. Default 1"
  echo "  -j num        Number of jobs. Default 2"
}

optstring=":hb:r:n:t:qei:gm:j:"

# Make sure at least one argument was supplied
if [[ $# -lt 1 ]]; then
  echo "Error: Too few options" 1>&2
  exit 1;
fi

RUN_NAME="DefaultRunName"
RANGEFILE=""
N_RUNSHEETS=0
N_TRAJECTORIES=4
N_THREADS=1
N_JOBS=2
N_INDIVIDUALS=31000
DO_BATCH=false
DO_EMAIL=false
DO_GRAPH=false

# Put the flag being processed into the variable $opt
while getopts $optstring opt; do
  case ${opt} in
    h )
      print_usage; exit 0
      ;;
    b )
      RUN_NAME=$OPTARG
      ;;
    r )
      RANGEFILE=$OPTARG
      ;;
    n )
      N_RUNSHEETS=$OPTARG
      ;;
    t )
      N_TRAJECTORIES=$OPTARG
      ;;
    q )
      DO_BATCH=true
      ;;
    e )
      DO_EMAIL=true
      ;;
    i )
      N_INDIVIDUALS=$OPTARG
      ;;
    g )
      DO_GRAPH=false
      ;;
    m )
      N_THREADS=$OPTARG
      ;;
    j )
      N_JOBS=$OPTARG
      ;;
    \? )
      echo "Invalid Option: -$OPTARG" 1>&2
      exit 1;
      ;;
    : )
      echo "Invalid Option: -$OPTARG requires an argument" 1>&2
      exit 1
      ;;
  esac
done

if [[ "$RANGEFILE" = "" ]]; then
  echo "Error: Must specify rangefile" 1>&2
  exit 1;
fi

if [[ "$N_RUNSHEETS" = 0 ]]; then
  echo "Error: Must specify # rangefiles" 1>&2
  exit 1;
fi

echo "RUN_NAME=$RUN_NAME"
echo "RANGEFILE=$RANGEFILE"
echo "N_RUNSHEETS=$N_RUNSHEETS"
echo "N_TRAJECTORIES=$N_TRAJECTORIES"
echo "N_INDIVIDUALS=$N_INDIVIDUALS"
echo "DO_BATCH=$DO_BATCH"
echo "DO_EMAIL=$DO_EMAIL"
echo "DO_GRAPH=$DO_GRAPH"

shift $((OPTIND -1))

# Load dependencies
if module help > /dev/null 2>&1; then
  module load foss/2018b || \
    { (>&2 echo 'Loading foss/2018b failed; exiting'); exit $E_RUNERR; };
  module load parallel || \
    { (>&2 echo 'Loading parallel failed; exiting');   exit $E_RUNERR; };
fi

which -s parallel || { (>&2 echo '"parallel" not found in $PATH; exiting'); exit $E_RUNERR; };

if ! seq -w $N_RUNSHEETS | parallel -j40 stat {}.json > /dev/null; then
  echo "Could not stat 1 or more of the runsheets; exiting";
  exit 1;
fi
srun="";

if which -s 'srun'; then
  # This specifies the options used to run srun. The "-N1 -n1" options are
  # used to allocates a single core to each task.
  srun="srun -n1 -N1 --cpus-per-task $N_THREADS --exclusive";
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
parallel="parallel --verbose -j $N_JOBS --delay 0.2 --joblog parallel.log"

# Arguments to tbabm.sh:
#   number of trajectories
#   size of threadpool
#   population size
#   name of parameter sheet
seq -w $N_RUNSHEETS | \
  $parallel $srun ./tbabm.sh $N_THREADS {} '$RANDOM' '>' {}.log '2>&1'

exit 0;
