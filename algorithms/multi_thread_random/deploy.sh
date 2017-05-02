#!/bin/bash


ssh nikolas_chaconas@csil-01.cs.ucsb.edu 'bash -s' < 'csil_scripts/git_pull.sh' &

sleep 5
for i in {10..48}
do
	ssh nikolas_chaconas@csil-$i.cs.ucsb.edu 'bash -s' < 'csil_scripts/setup.sh' &
	echo "Sent to CSIL-$i"
done