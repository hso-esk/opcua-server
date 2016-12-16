#! /bin/sh

# create the build directory
rm -rf asneg/build
mkdir -p asneg/build

# create makefiles from CMake
cd asneg/build
cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKEBUILDTYPE=Debug -D CMAKEINSTALLPREFIX=./ ../src -DCMAKECCOMPILER=gcc-4.9 -DCMAKECXXCOMPILER=g++-4.9 -DCMAKERANLIB=gcc-ranlib-4.9 -DCMAKEAR=gcc-ar-4.9
make