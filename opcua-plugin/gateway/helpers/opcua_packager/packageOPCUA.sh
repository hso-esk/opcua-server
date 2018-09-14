#!/bin/sh
#  @author: Dovydas Girdvainis 
#  @date  : 2018-09-10

## Paths
PACKER_DIR=$(pwd) 
BINARIES_ROOT_DIR=../../../../asneg/ 
PACKAGE_DIR=$PACKER_DIR/opcua/
DEPENDENCY_BASE_DIR=../../../../../../

## Private variable declarations
MIN_ARG_COUNT=2
PACKAGE_TYPE="none" 
BINARIES_DIR="none" 
BOOST_VER="none"
ARCH="none"
PACKAGE_VERSION="none"

## Color codes
RED='\033[1;31m'
GREEN='\033[1;32m'
BLUE='\033[1;34m'
PURPLE='\033[1;35m'
NC='\033[0m'

## Internal functions 

getBoostVer () {

if [ $# -eq 3 ];
then 
	case $3 in 
		1_54_0)
			BOOST_VER="1_54_0"
			;;
		1_67_0)
			BOOST_VER="1_67_0"
			;;
		*)
			BOOST_VER="1_54_0"
			echo "$RED Unsupported BOOST version! Setting the default $BLUE boost-$BOOST_VER $RED version $NC"
			echo "$PURPLE SUpported versions: $BLUE [1_54_0 | 1_67_0] $NC"
			;;
	esac
else 
	BOOST_VER="1_54_0"
	echo "$PURPLE No boost version has been provided! Using the default $BLUE boost-$BOOST_VER $NC"
fi
}

packageArmBinaries () {

## Check if openssl libs for arm exist
if [ -d "${DEPENDENCY_BASE_DIR}/openssl/lib" ]; 
then 
	cp -rf ${DEPENDENCY_BASE_DIR}/openssl/lib/* $PACKAGE_DIR/bin
else 
	echo "$RED NO OPENSSL for arm has been found! Try running the Dependency_Installer! $NC"
	exit 1
fi 

## Check if odbc-arm libs exist
if [ -d "${DEPENDENCY_BASE_DIR}/odbc-$ARCH/lib" ]; 
then 
	cp ${DEPENDENCY_BASE_DIR}/odbc-$ARCH/lib/libodbc.* $PACKAGE_DIR/bin
else 
	echo "$RED NO ODBC for arm has been found! Try running the Dependency_Installer! $NC"
	exit 1
fi

## Add remote debuging startup 
touch ${PACKAGE_DIR}/startGDBServer.sh
cat << EOF > ${PACKAGE_DIR}/startGDBServer.sh 
#!/bin/bash
#  @author      : Dovydas Girdvainis 
#  @date        : 2018-09-10 
#  @description : Start a remote debuging server on the host

cd ./bin 
gdbserver :9091 OpcUaServer4 etc/OpcUaStack/OpcUaServer.xml
        
EOF

}

packageBinaries () {
	BINARIES_DIR=$(pwd) 
	echo "$GREEN Packaging the OPC UA server binaries from $PURPLE $BINARIES_DIR $GREEN for $PACKAGE_TYPE... $NC"

	cd $PACKER_DIR
 
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
	
	## Copy the binary files
	cp -R ${BINARIES_DIR}* $PACKAGE_DIR
	
	## Rename the directory
	mv $PACKAGE_DIR/build-* $PACKAGE_DIR/bin

	## Remove Make files and build config
	rm -rf $PACKAGE_DIR/bin/C* $PACKAGE_DIR/bin/c* $PACKAGE_DIR/bin/BuildConfig.h $PACKAGE_DIR/bin/Makefile $PACKAGE_DIR/bin/OpcUaProjectBuilderConfig.h

	## Add the boost dependencies 
	if [ -d "${DEPENDENCY_BASE_DIR}boost-${ARCH}_${BOOST_VER}/lib" ] 
	then 
		cp ${DEPENDENCY_BASE_DIR}boost-${ARCH}_${BOOST_VER}/lib/* $PACKAGE_DIR/bin
	else 
		echo "$RED Boost libraries not found at ${DEPENDENCY_BASE_DIR}boost-${ARCH}_${BOOST_VER}/lib! Try the Dependency_Installer! $NC"
		exit 1
	fi
	
	if [ "$ARCH" = "arm" ];
	then
		packageArmBinaries
	fi

	## Copy the startup script
	cp ${BINARIES_ROOT_DIR}../opcua-run-${ARCH}-${PACKAGE_TYPE}.sh $PACKAGE_DIR/

	## Edit the startup script
	sed -i "4s/asneg\/build-${ARCH}-${PACKAGE_TYPE}/.\/bin/" "${PACKAGE_DIR}opcua-run-${ARCH}-${PACKAGE_TYPE}.sh"
	sed -i "7s/asneg\/build-${ARCH}-${PACKAGE_TYPE}/.\/bin/" "${PACKAGE_DIR}opcua-run-${ARCH}-${PACKAGE_TYPE}.sh"
	sed -i "10s/asneg\/build-${ARCH}-${PACKAGE_TYPE}/.\/bin/" "${PACKAGE_DIR}opcua-run-${ARCH}-${PACKAGE_TYPE}.sh"

	## Rename the startup script
	mv ${PACKAGE_DIR}opcua-run-${ARCH}-${PACKAGE_TYPE}.sh ${PACKAGE_DIR}opcua-run.sh

	## Copy the standard xml files 
	cp -rf ${BINARIES_ROOT_DIR}../cfg ${PACKAGE_DIR}

	## Copy the project specific xmls 
	cp -rf ${BINARIES_ROOT_DIR}../Amelis-XMLS ${PACKAGE_DIR}
	cp -rf ${BINARIES_ROOT_DIR}../Parsifal-XMLS ${PACKAGE_DIR}
	
	## Create project specific launchers 
	cp ${PACKAGE_DIR}opcua-run.sh ${PACKAGE_DIR}opcua-run_Ameli.sh
	cp ${PACKAGE_DIR}opcua-run.sh ${PACKAGE_DIR}opcua-run_Parsifal.sh

	## Edit project specific launcher 
	sed -i "7s/cfg/Amelis-XMLS\/cfg/" "${PACKAGE_DIR}opcua-run_Ameli.sh"
	sed -i "7s/cfg/Parsifal-XMLS\/cfg/" "${PACKAGE_DIR}opcua-run_Parsifal.sh"

	## Tar the files 
	tar -cvf opcua-server_v${PACKAGE_VERSION}_${ARCH}_${PACKAGE_TYPE}.tar opcua

	## Remove the package directory 
	rm -rf ${PACKAGE_DIR}

	echo "$GREEN Packaging complete! $NC"
	exit 0
}

doPackaging () {
	if [ -d "${BINARIES_ROOT_DIR}$BINARIES_DIR" ]; 
	then
		cd ${BINARIES_ROOT_DIR}${BINARIES_DIR}
		packageBinaries
	else 
		echo "$RED Directory $BINARIES_DIR in $BINARIES_ROOT_DIR does not exist! $NC"
	fi
}

if [ $# -lt $MIN_ARG_COUNT ]
	then 
		echo "$RED Please provide the package type: $BLUE[--DEBUG_x86 | --RELEASE_x86 | --DEBUG_arm | --RELEASE_arm] and the package version, for examle $BLUE [0.1] $NC"
		exit 1
	else 
		PACKAGE_VERSION=$2
fi

case $1 in 
	--DEBUG_x86) 
		ARCH="x86-amd64"
		PACKAGE_TYPE="debug"
		BINARIES_DIR="build-x86-amd64-debug"
		getBoostVer
		doPackaging
		;;
	--RELEASE_x86) 
		ARCH="x86-amd64"
		PACKAGE_TYPE="release"
		BINARIES_DIR="build-x86-amd64-release"
		getBoostVer
		doPackaging
		;;
	--DEBUG_arm) 
		ARCH="arm"
		PACKAGE_TYPE="debug"
		BINARIES_DIR="build-arm-debug"
		getBoostVer
		doPackaging
		;;
	--RELEASE_arm)
		ARCH="arm"
		PACKAGE_TYPE="release"
		BINARIES_DIR="build-arm-release"
		getBoostVer
		doPackaging
		;;
	*)
		echo "$RED Unrecognized packaging type, supported types are:  $BLUE [--DEBUG_x86 | --RELEASE_x86 | --DEBUG_arm | --RELEASE_arm] $NC"
		;;
esac
