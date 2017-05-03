#! /bin/sh

BUILD_TYPE=Release
#BUILDTYPE=Debug


# create the build directory
rm -rf asneg/build
mkdir -p asneg/build

# create makefiles from CMake
cd asneg/build

#build
cmake -DCMAKE_TOOLCHAIN_FILE=../../arm-linux-toolchain.cmake -DCMAKE_BUILD_TYPE=$BUILDTYPE ../src
make --jobs=3
