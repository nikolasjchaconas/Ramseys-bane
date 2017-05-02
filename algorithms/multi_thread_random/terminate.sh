#!/bin/bash

for i in {10..40}
do
	ssh nikolas_chaconas@csil-$i.cs.ucsb.edu 'bash -s' < 'csil_scripts/terminate.sh' &
done