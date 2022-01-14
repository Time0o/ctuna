#!/bin/sh

docker build -f test/Dockerfile -t ctuna .

docker run --cap-add NET_ADMIN ctuna
