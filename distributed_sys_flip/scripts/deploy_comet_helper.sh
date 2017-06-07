#!/bin/bash

cd ~/Ramseys-bane/distributed_sys_flip
git fetch --all
git reset --hard origin/master
mpicc thread.c clique-count.c greedyGraphPermute.c distributed_flip.c client_protocol/client.c matrix.c getcpuclockspeed.c cliquer.c graph.c reorder.c heuristicsearch.c -pthread -lm

sbatch jobscriptfile

#squeue -u clique
#