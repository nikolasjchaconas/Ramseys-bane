#!/bin/bash
cd ~/Ramseys-bane/algorithms/
git fetch --all
git checkout master
git pull
make clean
make