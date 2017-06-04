#!/bin/bash
cd ~/Ramseys-bane/distributed_sys_flip
git reset --hard origin/master
git pull
make clean
make