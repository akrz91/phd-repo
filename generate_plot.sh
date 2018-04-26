#!/bin/bash
#FILE="filename=\'"
#FILE
gnuplot -e "filename='$1'" ./visualise_data.plg
