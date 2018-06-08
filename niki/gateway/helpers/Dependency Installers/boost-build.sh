#!/bin/sh
#  @author: Oliver Kehret
#  @author: Dovydas Girdvainis (Modifications to get boost_version and OS type)
#  @date  : 2017-02-15
#		
# Prerequisites to run the script
# On Debian/Ubuntu:
# sudo apt-get install arm-linux-gnueabihf-gcc
# cd /usr/bin/
# sudo ln -s arm-linux-gnueabihf-gcc arm-linux-gnueabihf-gcc-x
# please set VERSION and BOOST_VERSION
VERSION=1.54.0
BOOST=boost_
BASE_DIR=../../../../../

if [ -z "$1" ]
	then
	BOOST_VERSION=1_54_0
	else 
	BOOST_VERSION=$1
fi

if [ -z "$2" ]
	then
	INSTALL_DIR=$BASE_DIR/boost-arm
	ARM_FLAG=1
	else 
	INSTALL_DIR=$BASE_DIR/boost-x86
	ARM_FLAG=0
fi

if [ ! -e ${BOOST}${BOOST_VERSION}".tar.bz2" ]; then
    wget -c http://sourceforge.net/projects/boost/files/boost/$VERSION"/"$BOOST_VERSION.tar.bz2
fi

if [ ! -e ${BOOST}${BOOST_VERSION} ]; then
    tar -xjvf ${BOOST}${BOOST_VERSION}" .tar.bz2"
fi

if [ ! -e "$INSTALL_DIR" ]; then
   mkdir -p $INSTALL_DIR
fi

if [ ! -e "$INSTALL_DIR/lib/libboost_thread.so" ]; then
    cd ${BOOST}${BOOST_VERSION}"/"
    ./bootstrap.sh
    if [ $ARM_FLAG -eq 1 ]
	then
	sed -i '12s/.*/  using gcc : arm : arm-linux-gnueabihf-g++-4.9 : <compileflags>-std=c++11 ;/' project-config.jam
    	./b2 install --prefix=$INSTALL_DIR -link=shared toolset=gcc-arm
	else 
	sed -i '12s/.*/  using gcc : 4.9 : g++-4.9 : <compileflags>-std=c++11 ;/' project-config.jam
 	./b2 install --prefix=$INSTALL_DIR -link=shared toolset=gcc
     fi
fi
if [ ! -e "$INSTALL_DIR/lib/libboost_thread.so" ]; then
echo "======================================="
echo "$INSTALL_DIR/lib/libboost_thread.so not succefully build ... aborting"
echo "remove the cause of the error and start again"
exit 1
fi
