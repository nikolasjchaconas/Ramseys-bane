#!/bin/bash

scp jobscriptfile clique@comet.sdsc.edu:Ramseys-bane/distributed_sys_flip
ssh -o "StrictHostKeyChecking no" clique@comet.sdsc.edu 'bash -s' < './deploy_comet_helper.sh' &