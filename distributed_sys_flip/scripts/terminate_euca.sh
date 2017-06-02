#!/bin/bash

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.222 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.222"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.192 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.192"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.218 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.218"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.203 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.203"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.189 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.189"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.244 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.244"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.191 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.191"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.172 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.172"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.251 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.251"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem  root@128.111.84.219 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.219"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.187 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.187"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.170 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.170"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.253 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.253"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.230 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.230"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.224 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.224"

ssh -o "StrictHostKeyChecking no" -i ~/.ssh/coordinator_key.pem root@128.111.84.193 'bash -s' < './terminate.sh' &
echo "Sent to root@128.111.84.193"