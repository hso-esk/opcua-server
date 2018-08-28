#!/bin/bash 

INSTALLER_NAME=gateway-installer_
OPCUA_SRC_PATH1=.
OPCUA_SRC_PATH2=../../../../
PREMSISSIONS=755
OPCUA_SERVER_NAME=opcua-server-bin-beaglebone_
EXTRA_LIBS=extra_libs
if [ -z "$1" ]
	then 
		INSTALLER_VERS=v0.5 
	elif [ "$1" == "--help" ]
	then
		echo "1st argumt - gateway installer version"
		echo "2nd argument - opcua server argument"
		echo "3ed argument - boost version"
		exit 1
	else 
		INSTALLER_VERS=v"$1"
fi

if [ -z "$2" ]
	then OPCUA_SERVER_VERS=v1.3.0
	else OPCUA_SERVER_VERS=v"$2"
		sed -i "8s/1.4.0/${OPCUA_SERVER_VERS}/" "${INSTALLER_NAME}files/gateway-installer.sh"
fi

if [ ! -z "$3" ]
	then BOOST_VER=1_54_0
	else BOOST_VER=1_67_0
fi

yes | cp -rf ../gateway-installer/* ./gateway-installer_files/

cp -R ../${EXTRA_LIBS}${BOOST_VER}/* ${OPCUA_SRC_PATH1}/asneg/build/

mkdir -p ${INSTALLER_NAME}${INSTALLER_VERS} tmp/opcua/asneg/build/ tmp/opcua/cfg
cp -R ${INSTALLER_NAME}files/* ./${INSTALLER_NAME}${INSTALLER_VERS}/

cp -R $OPCUA_SRC_PATH1/asneg/build/* ./tmp/opcua/asneg/build/ 
cp -R $OPCUA_SRC_PATH2/cfg/* ./tmp/opcua/cfg 
cp -R $OPCUA_SRC_PATH2/opcua-run-arm-release.sh ./tmp/opcua/
mv ./tmp/opcua/opcua-run-arm-release.sh ./tmp/opcua/opcua-run.sh
sed -i "4s/build-arm-release/build/" "./tmp/opcua/opcua-run.sh"
sed -i "8s/build-arm-release/build/" "./tmp/opcua/opcua-run.sh"
sed -i "10s/build-arm-release/build/" "./tmp/opcua/opcua-run.sh"

cd ./tmp/ 
chmod $PREMSISSIONS ./*
tar -pcvf ${OPCUA_SERVER_NAME}${OPCUA_SERVER_VERS}.tar * 
mv ./${OPCUA_SERVER_NAME}${OPCUA_SERVER_VERS}.tar ../${INSTALLER_NAME}${INSTALLER_VERS}/data/ 

cd ../ 
rm -Rf ./tmp/ 

cd ./${INSTALLER_NAME}${INSTALLER_VERS}/
tar -pcvzf ./${INSTALLER_NAME}${INSTALLER_VERS}.tar.gz * 
mv ${INSTALLER_NAME}${INSTALLER_VERS}.tar.gz ../ 

cd ../ 
rm -Rf ./${INSTALLER_NAME}${INSTALLER_VERS} 
