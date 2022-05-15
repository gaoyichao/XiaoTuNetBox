#!/bin/bash

if [ -d build ]; then
    rm build -r
fi

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/home/ict/local ..
make -j4
make install
cd ..

