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
DEPENDENCY_BASE_DIR=../../../../../../

## Private variable declarations
MIN_ARG_COUNT=2
BOOST_VER="none"
ARCH="none"
ARCH_NAME="none"

## Color codes
RED='\033[1;31m'
GREEN='\033[1;32m'
BLUE='\033[1;34m'
PURPLE='\033[1;35m'
NC='\033[0m'

## Internal functions 
handleArch() {
    if [ ! -z "$ARCH" ]
	then
		case $ARCH in 
			arm)
			ARCH_NAME="arm"
			;;
			x86) 
			ARCH_NAME="x86-amd64"
			;;
		esac
	fi
}

createInstallDir () {
	if [ ! -z "$ARCH_NAME" ]
		then
                INSTALL_DIR=${DEPENDENCY_BASE_DIR}boost_${BOOST_VER}-${ARCH_NAME}
		case $ARCH_NAME in 
			arm)			
			apt-get install -y gcc-4.9-arm-linux-gnueabihf g++-4.9-arm-linux-gnueabihf 
			ARM_FLAG=1
			;;
			x86-amd64) 
			apt-get install -y gcc-4.9 g++-4.9 cpp-4.9
			ARM_FLAG=0
			;;
			*)		
			echo "$PURPLE Supported Architecture version: $BLUE [arm | x_86-amd64] $NC"
		esac	
	fi
	if [ ! -e "$INSTALL_DIR" ]; then
		mkdir -p $INSTALL_DIR
	fi
}

createPackaging() {
	if [ ! -e ${INSTALL_DIR}/${BOOST}${BOOST_VER}".tar.bz2" ]; then
		wget -c http://sourceforge.net/projects/boost/files/boost/$VERSION"/"${BOOST}${BOOST_VER}.tar.bz2 --directory-prefix=$INSTALL_DIR
	fi

	tar -xjvf ${INSTALL_DIR}/${BOOST}${BOOST_VER}".tar.bz2" -C ${INSTALL_DIR}

	if [ ! -e "$INSTALL_DIR/${BOOST}${BOOST_VER}/lib/libboost_thread.so" ]; then
		echo "starting boost setup"	
		cd $INSTALL_DIR/${BOOST}${BOOST_VER}"/"
		INSTALL_DIR=$(pwd)		
		echo "cd to install dir" $INSTALL_DIR 	
		./bootstrap.sh
		if [ $ARM_FLAG -eq 1 ]${BOOST}${BOOST_VER}
		then
		sed -i '12s/.*/  using gcc : arm : arm-linux-gnueabihf-gcc-4.9 : <compileflags>-std=c++11 ;/' project-config.jam
		echo "Installing with b2 to" ${INSTALL_DIR}/boost-${ARCH_NAME}_${BOOST_VER}
			./b2 install --prefix=${INSTALL_DIR}/../../boost-${ARCH_NAME}_${BOOST_VER} -link=shared toolset=gcc-arm --with-atomic --with-thread --with-chrono --with-date_time --with-system --with-test --with-filesystem --with-program_options --with-regex
		else 
		sed -i '12s/.*/  using gcc : 4.9 : g++-4.9 : <compileflags>-std=c++11 ;/' project-config.jam
		echo "Installing with b2 to" ${INSTALL_DIR}/boost-${ARCH_NAME}_${BOOST_VER}
		./b2 install --prefix=${INSTALL_DIR}/../../boost-${ARCH_NAME}_${BOOST_VER} -link=shared toolset=gcc-4.9 --with-atomic --with-thread --with-chrono --with-date_time --with-system --with-test --with-filesystem --with-program_options --with-regex
		 fi
	fi
}

chmodInstallDir () {
	rm ${INSTALL_DIR}/../${BOOST}${BOOST_VER}".tar.bz2" 
	chmod 777 -R ${INSTALL_DIR}/../../boost* 
}

if [ $# -lt $MIN_ARG_COUNT ]
	then 
		echo "$RED Please provide the boost version: $BLUE[ 1_54_0 || 1_67_0 ] and the installation type $BLUE [arm || x86]"
		exit 1
fi

add-apt-repository "deb http://dk.archive.ubuntu.com/ubuntu/ xenial main" 
add-apt-repository "deb http://dk.archive.ubuntu.com/ubuntu/ xenial universe"
apt-get update

apt-get install -y cmake libboost-all-dev build-essential

# Not the best way to do this, will mess up newer gcc versions... 
rm /usr/bin/gcc /usr/bin/g++ /usr/bin/cpp
ln -s /usr/bin/gcc-4.9 /usr/bin/gcc
ln -s /usr/bin/g++-4.9 /usr/bin/g++
ln -s /usr/bin/cpp-4.9 /usr/bin/cpp

case $1 in 
	1_54_0)
		BOOST_VER="1_54_0"
		VERSION=1.54.0
		ARCH="$2"
		handleArch
		createInstallDir
		createPackaging
		chmodInstallDir
		;;
	1_67_0) 
		BOOST_VER="1_67_0"
		VERSION=1.67.0
		ARCH="$2"
		handleArch
		createInstallDir
		createPackaging
		chmodInstallDir
		;;
	*)
		BOOST_VER="1_54_0"
		echo "$RED Unsupported BOOST version! Setting the default $BLUE boost-$BOOST_VER $RED version $NC"
		echo "$PURPLE SUpported versions: $BLUE [1_54_0 | 1_67_0] $NC"
		;;
esac
