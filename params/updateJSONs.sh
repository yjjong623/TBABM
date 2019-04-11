#!/bin/bash

# One day this will be the new version
# find . -name '*.csv' -type f -print0 \
	# | parallel -0 sh -c 'csvjson {.}.csv > {.}.json'

# Old version:
find . -name \*.csv -type f -print0 \
	| sed "s/.csv//g" \
	| time xargs -0 -L 1 -I \% -P 4 -t sh -c 'csvjson "%.csv" > "%.json"'
