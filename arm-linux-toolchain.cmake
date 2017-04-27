# this one is important
SET(CMAKE_SYSTEM_NAME Linux)

#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

#SET(CMAKE_SYSTEM_PROCESSOR arm)
# specify the cross compiler
SET(CMAKE_C_COMPILER   /usr/bin/arm-linux-gnueabihf-gcc-4.9)
SET(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabihf-g++-4.9)

# where is the target environment
#SET(CMAKE_FIND_ROOT_PATH  ./bbb-toolchain)
# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#exlude system libraries
SET(NO_CMAKE_SYSTEM_PATH    ON)

#set boost library path
#SET(BOOST_INCLUDEDIR       "${CMAKE_SOURCE_DIR}/../../../boost/include")
#SET(BOOST_LIBRARYDIR       "${CMAKE_SOURCE_DIR}/../../../boost/lib")
SET(BOOST_ROOT              "${CMAKE_SOURCE_DIR}/../../../boost")
SET(Boost_NO_SYSTEM_PATHS   ON)

#set OpenSSL Library path
SET(OPENSSL_ROOT_DIR        "${CMAKE_SOURCE_DIR}/../../../openssl")

