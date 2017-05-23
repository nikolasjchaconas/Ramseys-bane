#!/bin/bash
if [ "$#" -ne 1 ]; then
    printf "Usage: ./deploy.sh NUM_THREADS\nPlease pass number of threads as first arguments\n"
fi
nohup ./setup.sh $1 > /dev/null 2>&1 &