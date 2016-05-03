#!/bin/bash

# This is a script to build the modules and run the test suite in the base
# Docker container.

set -x
set -e

cd /usr/src/ITKOpenSlideIO-build

cmake \
  -G Ninja \
  -DITK_DIR:PATH=/usr/src/ITK-build \
  -DBUILD_TESTING:BOOL=ON \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DBUILDNAME:STRING=External-OpenSlideIO \
    /usr/src/ITKOpenSlideIO
ctest -VV -D Experimental

mkdir -p /usr/src/ITKOpenSlideIO-example-build
cd /usr/src/ITKOpenSlideIO-example-build
cmake \
  -G Ninja \
  -DITK_DIR:PATH=/usr/src/ITK-build \
  -DCMAKE_BUILD_TYPE:STRING=Release \
    /usr/src/ITKOpenSlideIO/examples/
ninja
