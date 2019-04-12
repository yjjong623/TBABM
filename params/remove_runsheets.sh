#!/bin/sh

parallel -I \% find . -name "\%_\*.csv" -exec rm -v {} "\;" ::: $@
parallel -I \% find . -name "\%_\*.json" -exec rm -v {} "\;" ::: $@
