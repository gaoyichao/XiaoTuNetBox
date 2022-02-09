#!/bin/bash

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/home/gyc/local ..
make -j10
make install
cd ..

