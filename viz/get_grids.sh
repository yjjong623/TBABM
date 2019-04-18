#!/bin/sh

basedir="~/scratch60/lab/repos/TBABM/viz"
rangefile_title=${1?"Usage: get_grids.sh RANGEFILE_TITLE"}

archive_name=$rangefile_title"_images.tar.gz"

scp mwr8@farnam.hpc.yale.edu:$basedir/$archive_name $archive_name
tar -zxvf $archive_name -C .
rm $archive_name
