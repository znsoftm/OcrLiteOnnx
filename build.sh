#!/usr/bin/env bash

##### build for MacOS or Linux
mkdir -p build
pushd build
cmake ..
make
popd
