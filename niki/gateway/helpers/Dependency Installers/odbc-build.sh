#!/bin/sh
#  @author: Manuel Schappacher
#  @author: Dovydas Girdvainis (Added OS check)
#  @date  : 2017-12-06
#
# Prerequisites to run the script
# On Debian/Ubuntu:
# arm-linux-gnueabihf-gcc-4.9 and arm-linux-gnueabihf-g++-4.9
# must be installed
VERSION=2.3.4
ODBC_VERSION=unixODBC-2.3.4
BASE_DIR=../../../../../
#INSTALL_DIR=/usr/lib/arm-linux-gnueabihf/odbc

if [ -z "$1" ]
	then
	INSTALL_DIR=$BASE_DIR/odbc-arm
	ARM_FLAG=1
	else
	INSTALL_DIR=$BASE_DIR/odbc-x86
	ARM_FLAG=0
fi

if [ ! -e $ODBC_VERSION".tar.gz" ]; then
    wget -c http://www.unixodbc.org/$ODBC_VERSION.tar.gz
fi

if [ ! -e $ODBC_VERSION ]; then
	gzip -d $ODBC_VERSION".tar.gz"
    tar -xf $ODBC_VERSION".tar"
fi

if [ ! -e "$INSTALL_DIR" ]; then
   mkdir -p $INSTALL_DIR
fi

if [ ! -e "$INSTALL_DIR/lib/libodbc.a" ]; then
    cd $ODBC_VERSION"/"
    
    if [ $ARM_FLAG -eq 1 ]
	then
	./configure CC=/usr/bin/arm-linux-gnueabihf-gcc-4.9 CXX=/usr/bin/arm-linux-gnueabihf-g++-4.9 CFLAGS='-fPIC' CXXFLAGS='-fPIC' --target=arm-linux --host=x86 --prefix=$INSTALL_DIR --sysconfdir=/etc
    	else
 	./configure CC=/usr/bin/gcc-4.9 CXX=/usr/bin/g++-4.9 CFLAGS='-fPIC' CXXFLAGS='-fPIC' --target=x86 --host=x86 --prefix=$INSTALL_DIR --sysconfdir=/etc
    fi
    make
    make install
fi
if [ ! -e "$INSTALL_DIR/lib/libodbc.a" ]; then
echo "======================================="
echo "$INSTALL_DIR/lib/libodbc.a not succefully build ... aborting"
echo "remove the cause of the error and start again"
exit 1
fi
