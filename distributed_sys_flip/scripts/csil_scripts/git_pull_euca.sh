#!/bin/bash
cd ~/Ramseys-bane/distributed_sys_flip/
git fetch --all
git checkout master
git pull
make clean
make