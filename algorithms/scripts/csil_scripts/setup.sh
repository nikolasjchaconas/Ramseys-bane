#!/bin/bash
cd ~/Ramseys-bane/algorithms/

make run
sleep 5m
PROCESS=$(pgrep clique)

while [[ 1 == 1 ]]; do
	if [[ `pgrep clique` == "" ]]; then
		echo rerunning
		make run
	else
		echo sleeping
		sleep 5m
	fi
done