#!/bin/bash
if [ "$#" -ne 1 ]; then
    printf "Usage: ./deploy.sh NUM_THREADS\nPlease pass number of threads as first arguments\n"
fi
cd ~/Ramseys-bane/distributed_sys_flip/scripts

nohup ./setup.sh $1 > /dev/null 2>&1 &