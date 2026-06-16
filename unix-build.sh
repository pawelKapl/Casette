#!/bin/bash
mkdir -p build
cd build
cmake -G "Unix Makefiles" .. -DBUILD_NAMCORE=ON -DCMAKE_BUILD_TYPE="Release" -DCMAKE_TOOLCHAIN_FILE=../rpi64-toolchain.cmake
make -j32
scp Casette root@192.168.68.102:/root/Casette/build/