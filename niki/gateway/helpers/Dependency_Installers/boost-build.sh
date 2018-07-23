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
BOOST=boost_
BASE_DIR=../../../../../../

add-apt-repository "deb http://dk.archive.ubuntu.com/ubuntu/ xenial main" 
add-apt-repository "deb http://dk.archive.ubuntu.com/ubuntu/ xenial universe"
apt-get update

apt-get install -y cmake libboost-all-dev build-essential

if [ ! -z "$1" ]
	then
	if [ $1 = '1_67_0' ]
		then
		BOOST_VERSION=1_67_0
		VERSION=1.67.0
		else 
		BOOST_VERSION=1_54_0
		VERSION=1.54.0
	fi
fi

if [ ! -z "$2" ]
	then
	if [ $2 = 'arm' ]
		then
		INSTALL_DIR=$BASE_DIR/boost_$BOOST_VERSION-arm
		apt-get install -y gcc-4.9-arm-linux-gnueabihf g++-4.9-arm-linux-gnueabihf 
		ARM_FLAG=1

		else 
		INSTALL_DIR=$BASE_DIR/boost_$BOOST_VERSION-x86
		apt-get install -y gcc-4.9 g++-4.9 cpp-4.9
		ARM_FLAG=0
	fi
fi

# Not the best way to do this, will mess up newer gcc versions... 
 rm /usr/bin/gcc /usr/bin/g++ /usr/bin/cpp
 ln -s /usr/bin/gcc-4.9 /usr/bin/gcc
 ln -s /usr/bin/g++-4.9 /usr/bin/g++
 ln -s /usr/bin/cpp-4.9 /usr/bin/cpp

if [ ! -e "$INSTALL_DIR" ]; then
   mkdir -p $INSTALL_DIR
fi

if [ ! -e ${INSTALL_DIR}/${BOOST}${BOOST_VERSION}".tar.bz2" ]; then
    wget -c http://sourceforge.net/projects/boost/files/boost/$VERSION"/"${BOOST}${BOOST_VERSION}.tar.bz2 --directory-prefix=$INSTALL_DIR
fi

tar -xjvf ${INSTALL_DIR}/${BOOST}${BOOST_VERSION}".tar.bz2" -C ${INSTALL_DIR}

if [ ! -e "$INSTALL_DIR/${BOOST}${BOOST_VERSION}/lib/libboost_thread.so" ]; then
    echo "starting boost setup"	
    cd $INSTALL_DIR/${BOOST}${BOOST_VERSION}"/"
    INSTALL_DIR=$(pwd)		
    echo "cd to install dir" $INSTALL_DIR 	
    ./bootstrap.sh
    if [ $ARM_FLAG -eq 1 ]${BOOST}${BOOST_VERSION}
	then
	sed -i '12s/.*/  using gcc : arm : arm-linux-gnueabihf-gcc-4.9 : <compileflags>-std=c++11 ;/' project-config.jam
	echo "Installing with b2 to" $INSTALL_DIR/boost-arm_${BOOST_VERSION}
    	./b2 install --prefix=${INSTALL_DIR}/../../boost-arm_${BOOST_VERSION} -link=shared toolset=gcc-arm 
	else 
	sed -i '12s/.*/  using gcc : 4.9 : g++-4.9 : <compileflags>-std=c++11 ;/' project-config.jam
	echo "Installing with b2 to" ${INSTALL_DIR}/boost-x86_${BOOST_VERSION}
 	./b2 install --prefix=${INSTALL_DIR}/../../boost-x86_${BOOST_VERSION} -link=shared toolset=gcc-4.9 --with-thread --with-chrono --with-date_time --with-system --with-test --with-filesystem --with-program_options --with-regex
     fi
fi

rm ${INSTALL_DIR}/../${BOOST}${BOOST_VERSION}".tar.bz2" 
chmod 777 -R ${INSTALL_DIR}/../../boost* 
