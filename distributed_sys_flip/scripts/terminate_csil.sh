#!/bin/bash

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
		ssh -o "StrictHostKeyChecking no" nikolas_chaconas@linux$j.engr.ucsb.edu 'bash -s' < './terminate.sh' &
		echo "Shutdown sent to linux$j.engr.ucsb.edu"
	fi

	if [ "$i" -lt 36 ]
	then
		ssh -o "StrictHostKeyChecking no" nikolas_chaconas@cstl-$j.cs.ucsb.edu 'bash -s' < './terminate.sh' &
		echo "Shutdown sent to cstl-$j.cs.ucsb.edu"
	fi

	ssh -o "StrictHostKeyChecking no" nikolas_chaconas@csil-$j.cs.ucsb.edu 'bash -s' < './terminate.sh' &
	echo "Shutdown sent to csil-$j.cs.ucsb.edu"
done