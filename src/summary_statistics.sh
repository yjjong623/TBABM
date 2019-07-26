#!/usr/bin/env bash

# This script calculates summary statistics on TimeSeries, where the TimeSeries
# involved are specified in the 'files' variable.
# 
# Each file is assumed to have a row that we don't want - this is the 50th
# period that is always left blank in PrevalenceTimeSeries.
# 
# Cleaning/reformatting/joining of data is done, and then the data is fed to
# 'datamash' where min, max, mean, and median are calculated
# 
# Expects: the first argument to be the trajectory-id (0001, 0534, etc.) of
# some trajectory that has finished running, and for that trajectory-id to
# correpond to the name of a directory where the results from that trajectory
# are stored. Also expects the runs to be 50 years long
# 
# Possible improvements:
# - Specification of which files/types to compute statistics on
# - More statistics
# - Statistics on only a portion of a time series, for example, year 10-onwards

files='tbCompletedTreatment.csv|tbInTreatment.csv|tbExperienced.csv|tbInfectious.csv|tbLatent.csv|tbSusceptible.csv|tbTxExperiencedAdults.csv|tbTxExperiencedInfectiousAdults.csv|tbTxNaiveAdults.csv|tbTxNaiveInfectiousAdults.csv'

cd $1 || (>&2 echo 'Failed to open directory' $1)

awk 'NR > 1 && $1 < 50' populationSize.csv | sort -t , -k 1 > ps_clean.csv

ls -1 |
  grep -oE $files |
  xargs awk -F, -v OFS=, 'FNR > 1 && FNR < 52 { print substr(FILENAME, 1, index(FILENAME, ".csv")-1), $0 }' |
  sort -t , -k 2 |
  join -t , -1 2 -2 1 -o '1.1,1.2,1.3,1.4,2.3' - ps_clean.csv |
  awk -F, -v OFS=, '{ print $1, $2, $3, 100*$4/($5+0.0001) }' |
  (echo 'Type,Period,Trajectory,Value'; cat) |
  datamash -s -t, --round 4 --header-in --group Type min Value max Value mean Value median Value |
  sed 's/^/'$1',/g'

rm -f ps_clean.csv

exit $?;

# Go to the directory, exit if this fails, and list the contents of the
# directory, 1 per line
cd $1 && ls -1 | 
  # Discard the files we aren't going to do statistics on. -o means print only
  # the matching part of the line, and -E means use extended regular expressions
  grep -oE $files | 
  # Discard the header and prepend each successive line with the filename. This
  # effectively adds a first column to every TimeSeries containing the filename
  # of that TimeSeries.
  # 
  # -I means replace instances of % with a field
  # -F means that the input field separator is a command (csv)
  # -v sets the Output Field Separator to a comman (csv)
  # NR is an 'awk' variable that essentially means line number
  # $0 is the line (including all of its fields, i.e. columns)
  xargs -I % awk -F , -v OFS=, 'NR>1{ print "%", $0 }' % |
  # Get rid of the periods 0-9 and 50
  awk -F , '10 <= $2 && $2 < 50 { print $0 }' |
  # Remove the file extension from the filename. This converts the filename to
  # the 'type' name, i.e. 'tbExperienced.csv' => 'tbExperienced'
  sed 's/.csv//g' |
  # These are the new headers; add them first to the stream and then cat the
  # rest of the stdin
  (echo 'Type,Period,Trajectory,Value'; cat) |
  # Compute the summary statistics. Assume the stream is sorted on 'Type' (it
  # should be).
  # -t means fields are separated by a comma.
  # --round means round numbers to 4 decimal places
  # --header-in means that the stream will contain one header row,
  #   but the output of the stream will not.
  # --group groups the data on the 'Type' variable
  # The rest of the line specifies the output records: min of Value, max of
  # Value, etc
  datamash -t , --round 4 --header-in --group Type min Value max Value mean Value median Value |
  # To the beginning of every line, append the trajectory-id followed by a
  # comma. This way, output from one trajectory is distinguishable from
  # that of other trajectories when this script is run on every trajectory 
  # in a batch.
  sed 's/^/'$1',/g'
  # column -t -s ,
  # ^ The above line can be used to pretty print the output


# ===> The code below can calculate prevalence using two TimeSeries <===



# FILE_NUMERATOR=`ls -1 $1/*tbInfectious.csv`
# FILE_DENOMINATOR=`ls -1 $1/*populationSize.csv`

# echo $FILE_NUMERATOR $FILE_DENOMINATOR

# Remove the 50th period of data (incidence data only covers 0-49). Preserve
# the header
# AWK_HANDLE_PREVALENCE_DATA='(NR-1) % 51 != 0 || NR == 1'

# Print the first line. Then, calculate the rate as the quotient of the 
# third and fourth column. Then, print all the columns.
# AWK_CALCULATE_RATE="'"'NR==1{print}; NR>1{inc=100*$3/($4+0.01); print $1,$2,$3,$4,inc}'"'"

# Combine the two files, which are comma delimited
# paste -d , $FILE_NUMERATOR $FILE_DENOMINATOR |\
    # datamash -t , --round 4 --header-in min percentage max percentage mean percentage





    # Remove 50th periods
    # awk '(NR-1) % 51 != 0 || NR == 1' |\
    # Get columns 1,2,3,6. 1,2,3 is period, seed, valueL, 6 is valueR
    # cut -d , -f 1,2,3,6 |\
    # Tell awk to read and write comma-delimited data. Perform rate calculation.
    # awk -F, -v OFS=, 'NR==1{print "period,trajectory,valueL,valueR,percentage"};\
    #                 NR>11{inc=$3/($4+0.001); print $1, $2, $3, $4, inc}' |\
