#! /bin/sh
#  @author: Dovydas Girdvainis 
#  @date  : 2018-01-07

INSTALLER_ASSEMBLY_DIRECTORY=./
SOURCE_BUILD_DIR=../../../../asneg/build-arm-release
PACKTED_BUILD_DIR=asneg/build

if [ ! -z "$1" ]
	then
	if [ $1='arm' ]
		then
		BUILD_OS=arm
		else
		BUILD_OS=x86
	fi
	else
	BUILD_OS=arm
fi

if [ ! -z "$2" ]
	then 
	if [ $2='1_67_0' ]
		then
		BOOST_VER='1_67_0'
		else 
		BOOST_VER='1_54_0'
	fi
	else 
	BOOST_VER='1_54_0' 
fi

if [ ! -d ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs ]
	then
	mkdir ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs	
	cp -r ../../../../../../openssl/lib/* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp -r ../../../../../../odbc-$BUILD_OS/lib/libodbc.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/ 
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_atomic.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_chrono.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_date* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_filesystem.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_prg_exec_monitor.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_regex.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_serialization.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_system.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_test_exec_monitor.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_thread.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../../boost-${BUILD_OS}_${BOOST_VER}/lib/libboost_wserialization.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
fi

if [ -d $INSTALLER_ASSEMBLY_DIRECTORY/asneg ]; then 
	rm -Rf $INSTALLER_ASSEMBLY_DIRECTORY/asneg 
fi

mkdir -p $INSTALLER_ASSEMBLY_DIRECTORY/$PACKTED_BUILD_DIR

cp -R ./$SOURCE_BUILD_DIR/* $INSTALLER_ASSEMBLY_DIRECTORY/$PACKTED_BUILD_DIR

cd $INSTALLER_ASSEMBLY_DIRECTORY

rm -R ./$PACKTED_BUILD_DIR/BuildConfig.h ./$PACKTED_BUILD_DIR/C* ./$PACKTED_BUILD_DIR/cmake*

cp -R ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/* ./$PACKTED_BUILD_DIR
