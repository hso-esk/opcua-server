#! /bin/sh

# create the build directory
rm -rf asneg/build
mkdir -p asneg/build

# create makefiles from CMake
cd asneg/build
#cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=./ ../src -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9
cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_INSTALL_PREFIX=./ ../src -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9
make
