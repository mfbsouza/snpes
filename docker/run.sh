#!/bin/bash

TAG=cpputest-docker

HOST_WORKDIR=${PWD}
WORKDIR=/home
COMMAND=${1:-/bin/bash}

docker run \
  --rm \
  --name cpputest-docker \
  --volume "${HOST_WORKDIR}":"${WORKDIR}" \
  --workdir "${WORKDIR}" \
  -it $TAG \
  $COMMAND
