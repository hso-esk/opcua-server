#! /bin/sh

set BUILD_TYPE=Release
#set BUILD_TYPE=Debug

# create the build directory
rm -rf asneg/build
mkdir -p asneg/build

# create makefiles from CMake
cd asneg/build

#build
cmake -DCMAKE_TOOLCHAIN_FILE=../../x86-amd64-linux-toolchain.cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../src
make --jobs=3
