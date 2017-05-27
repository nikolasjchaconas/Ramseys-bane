#!/bin/bash

cd ~/Ramseys-bane/distributed_sys_flip
git stash
git pull origin master
mpicc thread.c clique-count.c greedyGraphPermute.c distributed_flip.c client_protocol/client.c matrix.c getcpuclockspeed.c -pthread -lm

sbatch jobscriptfile

#squeue -u clique
#