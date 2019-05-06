#!/bin/sh

# $1: Number of threads per process
# $2: Run ID, padded
# $3: Seed, integer

n_threads=$1
run_id=$2
seed=$3

n_trajectories=2
n_people=1000

parameter_fname=$run_id.json
output_dir=$run_id

E_RUNERR=65

echo "Run $run_id" seq:$PARALLEL_SEQ host:$(hostname) date:$(date)

# Determine which TBABM binary will be used. Any TBABM binary in the working
# directory is chosen first. Then, if Lmod is available, an attempt is made 
# to load TBABM from there. Finally, TBABM is searched for in $PATH.
if stat TBABM > /dev/null 2>&1; then
	TBABM="./TBABM";
	echo "Binary found [dir]: ./TBABM";

elif which -s module; then
	module load TBABM || { (>&2 echo 'Loading TBABM from Lmod failed; exiting');\
						        exit $E_RUNERR; };

	which TBABM || { (>&2 echo 'No TBABM found in $PATH after successful Lmod load; exiting');\
					 	  exit $E_RUNERR; };

	TBABM=`which TBABM`;
	echo "Binary found [Lmod]: $(which TBABM)";

else
	which -s TBABM || { (>&2 echo 'No Lmod, TBABM in $PATH, or TBABM in .; exiting');\
							 exit $E_RUNERR; };

	TBABM=`which TBABM`;
	echo "Binary found: [path]: $(which TBABM)";
fi

# Make sure that the RunSheet actually exists
stat $parameter_fname > /dev/null || \
	{ (>&2 echo "Runsheet '$parameter_fname' does not exist; exiting");
		   exit 1; }

# Attempt to create a directory for output and exit if fails
mkdir $output_dir || { (>&2 echo "Creation of '$output_dir' failed; exiting"); exit 1; }

# Run the model
$TBABM -t $n_trajectories \
       -m $n_threads \
       -n $n_people \
       -p $parameter_fname \
       -o $output_dir/\
       -s $seed
