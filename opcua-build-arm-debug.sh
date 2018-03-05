#! /bin/sh

#BUILD_TYPE=Release
BUILDTYPE=Debug


# create the build directory
rm -rf asneg/build-arm-debug
mkdir -p asneg/build-arm-debug

# create makefiles from CMake
cd asneg/build-arm-debug

#build
cmake -DCMAKE_TOOLCHAIN_FILE=../../arm-linux-toolchain.cmake -DCMAKE_BUILD_TYPE=$BUILDTYPE ../src
make --jobs=5
