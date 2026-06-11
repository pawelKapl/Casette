# Cross-compile for Raspberry Pi 64-bit - compilation on RPi is a nightmare, trust me on this one

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_SYSROOT "") # since we are cross compiling on PC we need all the libs pulled from RPi, set path here
set(CMAKE_XCOMPILATION_TOOLCHAIN "") # cross-compilation toolchain path

set(CMAKE_C_COMPILER   "${CMAKE_XCOMPILATION_TOOLCHAIN}/bin/aarch64-none-linux-gnu-gcc.exe")
set(CMAKE_CXX_COMPILER "${CMAKE_XCOMPILATION_TOOLCHAIN}/bin/aarch64-none-linux-gnu-g++.exe")

set(ENV{PKG_CONFIG_DIR} "")
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})
set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu/pkgconfig;${CMAKE_SYSROOT}/usr/share/pkgconfig")
set(PKG_CONFIG_PATH "${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu/pkgconfig")
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})