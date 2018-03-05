#! /bin/sh

BUILD_TYPE=Release
#BUILDTYPE=Debug

# create the build directory
rm -rf asneg/build-x86-amd64-release
mkdir -p asneg/build-x86-amd64-release

# create makefiles from CMake
cd asneg/build-x86-amd64-release

#build
cmake -DCMAKE_TOOLCHAIN_FILE=../../x86-amd64-linux-toolchain.cmake -DCMAKE_BUILD_TYPE=$BUILDTYPE ../src
make --jobs=5
