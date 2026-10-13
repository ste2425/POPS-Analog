#!/bin/sh

# Start docker
systemctl start docker

# Clean
docker run --rm -it   -v "$PWD:/build"   -w /build   ghcr.io/pspdev/pspdev:latest   make clean

# Build
docker run --rm -it   -v "$PWD:/build"   -w /build   ghcr.io/pspdev/pspdev:latest   make

