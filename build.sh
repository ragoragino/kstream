#/bin/bash

set -ex

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

conan install .. \
      --install-folder . \
      --build=missing
      
cmake ..
cmake --build .