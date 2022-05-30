#!/bin/bash

VAL=$1

if [ -z $1 ]
then
    VAL="username=username&password=password"
fi
# echo -e "VALS are $VAL"
export ENV_VAR="hello environment"
./login $VAL