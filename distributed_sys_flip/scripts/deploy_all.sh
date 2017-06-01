#!/bin/bash

./deploy_euca.sh



echo "Sending to 12 core node on euca!"
ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.246 'bash -s 12' < './deploy.sh' &
printf "Check it out here: \nssh -i ~/.ssh/coordinator_key.pem root@128.111.84.246\n\n"

echo "Sending to 12 core cornell client!"
ssh -o "StrictHostKeyChecking no" -i ~/.ssh/ramseys-bane-cornell.pem ubuntu@128.84.8.90 'bash -s 12' < './deploy.sh' &
printf "Check it out here: \nssh -i ~/.ssh/ramseys-bane-cornell.pem ubuntu@128.84.8.90\n\n"

echo "Sending to 2 core cornell client"
ssh -o "StrictHostKeyChecking no" -i ~/.ssh/ramseys-bane-cornell.pem ubuntu@128.84.8.46 'bash -s 2' < './deploy.sh' &
printf "Check it out here: \nssh -i ~/.ssh/ramseys-bane-cornell.pem ubuntu@128.84.8.46\n\n"

echo "Sending to 8 core Chameleon node"
ssh -o "StrictHostKeyChecking no" cc@129.114.33.232 -i ~/.ssh/cloud.key 'bash -s 8' < './deploy.sh' &
printf "Check it out here: \nssh cc@129.114.33.232 -i ~/.ssh/cloud.key\n\n"

echo "Sending to condor node"

echo "scp condor_configuration.txt"
scp condor_configuration1.txt cs293b-3@ucsbsubmit.chtc.wisc.edu:Ramseys-bane/distributed_sys_flip
scp condor_configuration2.txt cs293b-3@ucsbsubmit.chtc.wisc.edu:Ramseys-bane/distributed_sys_flip
scp condor_configuration3.txt cs293b-3@ucsbsubmit.chtc.wisc.edu:Ramseys-bane/distributed_sys_flip
scp condor_configuration4.txt cs293b-3@ucsbsubmit.chtc.wisc.edu:Ramseys-bane/distributed_sys_flip
scp condor_configuration5.txt cs293b-3@ucsbsubmit.chtc.wisc.edu:Ramseys-bane/distributed_sys_flip

ssh -o "StrictHostKeyChecking no" cs293b-3@ucsbsubmit.chtc.wisc.edu 'bash -s' < './deploy_condor.sh' &
printf "Check it out here: \nssh cs293b-3@ucsbsubmit.chtc.wisc.edu\n\n"

# scp jobscriptfile clique@comet.sdsc.edu:Ramseys-bane/distributed_sys_flip
# ssh -o "StrictHostKeyChecking no" clique@comet.sdsc.edu 'bash -s' < './deploy_comet.sh' &

printf "\nSend to a total of 17 Euca nodes + 12 core Euca + 12 core Cornell + 2 Core Cornell + 8 core Chameleon + 250condor + 250*8core + 250*16core + 250*24core for a total of 12301 nodes!\n"