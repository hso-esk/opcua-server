#!/bin/sh
#  @author: Oliver Kehret
#  @date  : 2017-02-15
#
# Prerequisites to run the script
# On Debian/Ubuntu:
# sudo apt-get install arm-linux-gnueabihf-gcc
# cd /usr/bin/
# sudo ln -s arm-linux-gnueabihf-gcc arm-linux-gnueabihf-gcc-x
# Get openssl archive if not existing
export CC=/usr/bin/arm-linux-gnueabihf-gcc-4.9
export CXX=/usr/bin/arm-linux-gnueabihf-g++-4.9
OPNSSL_VERSION=
if [ ! -e "OpenSSL_1_0_2k.tar.gz" ]; then
    wget https://github.com/openssl/openssl/archive/OpenSSL_1_0_2k.tar.gz
fi

# A clean install is needed in case of an update
if [ ! -e "openssl-OpenSSL_1_0_2k" ]; then
    tar -pxzf OpenSSL_1_0_2k.tar.gz
fi

if [ ! -e "openssl/lib/libcrypto.a" ]; then
    export INSTALL_DIR=../../../../../openssl
    cd openssl-OpenSSL_1_0_2k/

    ./Configure linux-generic32 -fPIC shared \
                --prefix=$INSTALL_DIR --openssldir=$INSTALL_DIR/openssl

    make depend
    make
    make install
fi
