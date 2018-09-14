#!/bin/sh
#  @author: Dovydas Girdvainis 
#  @date  : 2018-09-10 

## Paths
PACKER_DIR=$(pwd) 
PACKAGE_DIR=${PACKER_DIR}/gateway-installer
BINARIES_ROOT_DIR=../../../../asneg/ 
GATEWAY_INSTALLER_DIR=${PACKER_DIR}/gateway-installer_files/

## Private variable declaration 
MIN_ARG_COUNT=2
PACKAGE_TYPE="none" 
PACKAGE_VERSION="none"
RELEASE_TYPE="none"

## Color codes
RED='\033[1;31m'
GREEN='\033[1;32m'
BLUE='\033[1;34m'
PURPLE='\033[1;35m'
NC='\033[0m'

doPackaging () {

## Package opcua server
${PACKER_DIR}/../opcua_packager/packageOPCUA.sh $PACKAGE_TYPE $PACKAGE_VERSION
ret_code=$? 

if [ $ret_code -ne 0 ]; 
then 
	echo "$RED Failed executind OPC UA packager. $NC"
	exit 1
fi

## Check if directory exists
if [ ! -d "$PACKAGE_DIR" ]; 
then 
	## IF NOT create the directory
	mkdir $PACKAGE_DIR
else 
	## IF YES delete the old one and create a new 
	rm -rf $PACKAGE_DIR 
	mkdir $PACKAGE_DIR
fi

## Copy the gateway installer files to packaging directory 
cp -rf ${GATEWAY_INSTALLER_DIR}* $PACKAGE_DIR

## Move the tar file to data directory
mv ${PACKER_DIR}/*.tar ${PACKAGE_DIR}/data

## Edit the installer file 
sed -i "8s/1.4.0/${PACKAGE_VERSION}/" "${GATEWAY_INSTALLER_DIR}/gateway-installer.sh"

cd $PACKAGE_DIR
tar -cvzf ../gateway-installer-v${PACKAGE_VERSION}_${RELEASE_TYPE}.tar.gz *

## Remove the package directory
rm -rf $PACKAGE_DIR

}

if [ $# -lt $MIN_ARG_COUNT ]
	then 
		echo "$RED Please provide the package type: $BLUE[ --DEBUG_arm | --RELEASE_arm ] and the package version, for examle $BLUE [0.1] $NC"
		exit
	else 
		PACKAGE_VERSION=$2
fi

case $1 in 
	--DEBUG_arm)
		PACKAGE_TYPE="--DEBUG_arm"
		RELEASE_TYPE="debug"
		doPackaging
		;; 
	--RELEASE_arm)
		PACKAGE_TYPE="--RELEASE_arm"
		RELEASE_TYPE="release"
		doPackaging
		;;
esac

