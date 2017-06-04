#!/bin/bash
cd ~/Ramseys-bane/distributed_sys_flip
git fetch --all
git reset --hard origin/master
make clean
make