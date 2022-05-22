#!/bin/bash

if [ -d build ]; then
    rm build -r
fi

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${HOME}/local ..
make -j4
make install
cd ..

