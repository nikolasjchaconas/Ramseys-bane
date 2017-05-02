#!/bin/bash
cd ~/Ramseys-bane/algorithms/multi_thread_random/

make run
sleep 5
PROCESS=$(pgrep clique)

while [[ 1 == 1 ]]; do
	if [[ `pgrep clique` == "" ]]; then
		echo rerunning
		make run
	else
		echo sleeping
		sleep 5
	fi
done