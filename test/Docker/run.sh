#!/bin/sh

script_dir="`cd $(dirname $0); pwd`"

docker run \
  --rm \
  -v $script_dir/../..:/usr/src/ITKOpenSlideIO \
    insighttoolkit/openslideio-test \
      /usr/src/ITKOpenSlideIO/test/Docker/test.sh
