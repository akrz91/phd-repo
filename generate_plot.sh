#!/bin/bash

gnuplot -e "filename='$1'; plotTitle='$2'" ./visualise_data.plg
