#! /bin/sh

BUILD_TYPE=Release
#BUILDTYPE=Debug


# create the build directory
rm -rf asneg/build-arm-release
mkdir -p asneg/build-arm-release

# create makefiles from CMake
cd asneg/build-arm-release

#build
cmake -DCMAKE_TOOLCHAIN_FILE=../../arm-linux-toolchain.cmake -DCMAKE_BUILD_TYPE=$BUILDTYPE ../src
make --jobs=5
