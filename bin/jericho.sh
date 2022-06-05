#!/bin/bash
ROOTDIR=`git rev-parse --show-toplevel`
source $ROOTDIR/config/settings
export CLUSTER

PORT=$1

if [-z "$1"]
then
    PORT=8080    
fi

./build/bin/prizm $PORT 10 ./frontend