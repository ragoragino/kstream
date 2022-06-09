#/bin/bash

set -ex

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

export CXX=/usr/bin/g++-10

conan install .. \
      -pr=profile \
      --install-folder . \
      --build=missing
      
cmake ..
cmake --build .