#!/bin/bash
PREFIX="$HOME/git/CleanWeatherPebble/resources/images/weather/"
for FILE in "$PREFIX"*.svg; do
	echo -n "Converting '$FILE' ..."
	convert "$FILE" -resize 50x50 "${FILE%.*}.png"
	echo "done"
done
