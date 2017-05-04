#!/bin/bash


ssh -o "StrictHostKeyChecking no" nikolas_chaconas@csil-01.cs.ucsb.edu 'bash -s' < '../csil_scripts/git_pull.sh' &

sleep 10
for i in {1..48}
do
	if [ "$i" -lt 10 ]
	then
		j="0$i"
	else
		j="$i"
	fi
	if [ "$i" -lt 29 ]
	then
		ssh -o "StrictHostKeyChecking no" nikolas_chaconas@linux$j.engr.ucsb.edu 'bash -s' < '../csil_scripts/deploy.sh' &
		echo "Sent to linux$j.engr.ucsb.edu"
	fi
	ssh -o "StrictHostKeyChecking no" nikolas_chaconas@csil-$j.cs.ucsb.edu 'bash -s' < '../csil_scripts/deploy.sh' &
	echo "Sent to csil-$j.cs.ucsb.edu"
done