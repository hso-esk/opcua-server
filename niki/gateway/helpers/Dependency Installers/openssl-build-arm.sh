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
apt-get install -y gcc-4.9-arm-linux-gnueabihf g++-4.9-arm-linux-gnueabihf 

export CC=/usr/bin/arm-linux-gnueabihf-gcc-4.9
export CXX=/usr/bin/arm-linux-gnueabihf-g++-4.9
export AR=/usr/bin/arm-linux-gnueabihf-ar
export RANLIB=/usr/bin/arm-linux-gnueabihf-ranlib

OPNSSL_VERSION=
if [ ! -e "OpenSSL_1_0_2k.tar.gz" ]; then
    wget https://github.com/openssl/openssl/archive/OpenSSL_1_0_2k.tar.gz
fi

# A clean install is needed in case of an update
if [ ! -e "../../../../../../../openssl-OpenSSL_1_0_2k" ]; then
    tar -pxzf OpenSSL_1_0_2k.tar.gz -C ../../../../../../ 
    chmod 777 -R ../../../../../../OpenSSL_1_0_2k.tar.gz
    rm OpenSSL_1_0_2k.tar.gz
fi

if [ ! -e "openssl/lib/libcrypto.a" ]; then
    cd ../../../../../../openssl-OpenSSL_1_0_2k/
    export INSTALL_DIR=$(pwd)/../openssl
    chmod 777 -R ../openssl*
    ./Configure linux-generic32 shared --prefix=$INSTALL_DIR --openssldir=$INSTALL_DIR/openssl

    make depend
    make
    make install
    chmod 777 -R ../openssl*
fi
