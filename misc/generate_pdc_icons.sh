#!/bin/bash
PREFIX="$HOME/git/CleanWeatherPebble/resources/images/weather/"
BIN="$HOME/tools/pebble-sdk/svg2pdc.py"
cd "$PREFIX"
for FILE in *.svg; do
	echo -n "Converting '$FILE' ..."
	python2.7 $BIN $FILE
	echo "done"
done
