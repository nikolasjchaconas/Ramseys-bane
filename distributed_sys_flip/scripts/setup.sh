#!/bin/bash
cd ~/Ramseys-bane/distributed_sys_flip
git fetch --all
git reset --hard origin/master
make clean
make
make run threads=$1
sleep 5m
PROCESS=$(pgrep clique)

while [[ 1 == 1 ]]; do
	if [[ `pgrep clique` == "" ]]; then
		echo rerunning
		make run threads=$1
	else
		echo sleeping
		sleep 5m
	fi
done