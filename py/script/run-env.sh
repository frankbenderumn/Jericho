#!/bin/bash

PORT=$1

if [ -z "$1" ]
then
    PORT=8085
fi

ROOTDIR=`git rev-parse --show-toplevel`
echo "${ROOTDIR}"
source ${ROOTDIR}/config/settings

docker run --rm -p 127.0.0.1:$PORT:8085 -v "${ROOTDIR}:/home/user/repo" -it ${DOCKER_IMAGE_PREFIX}/env
