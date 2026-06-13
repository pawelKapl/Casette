#!/bin/bash
mkdir -p build
cd build
cmake -G "Unix Makefiles" .. -DCMAKE_TOOLCHAIN_FILE=../rpi64-toolchain.cmake
make -j32
scp Casette root@192.168.68.104:/root/Casette/build/