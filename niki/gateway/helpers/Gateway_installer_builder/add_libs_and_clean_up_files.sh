#! /bin/sh
#  @author: Dovydas Girdvainis 
#  @date  : 2018-01-07

INSTALLER_ASSEMBLY_DIRECTORY=./
SOURCE_BUILD_DIR=../../../../asneg/build-arm-release
PACKTED_BUILD_DIR=asneg/build

if [ -z "$1" ]
	then
	BUILD_OS=arm
	else
	BUILD_OS=x86
fi

if [ ! -d ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs ]
	then
	mkdir ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs	
	cp ../../../../../openssl/lib/* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../odbc-$BUILD_OS/libodbc.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/ 
	cp ../../../../../boost-$BUILD_OS/libboost_atomic.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../boost-$BUILD_OS/libboost_chrono.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../boost-$BUILD_OS/libboost_date.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../boost-$BUILD_OS/libboost_filesystme.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../boost-$BUILD_OS/libboost_prg_exec_monitor.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../boost-$BUILD_OS/libboost_regex.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../boost-$BUILD_OS/libboost_serialization.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../boost-$BUILD_OS/libboost_system.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../boost-$BUILD_OS/libboost_test_exec_monitor.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../boost-$BUILD_OS/libboost_thread.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
	cp ../../../../../boost-$BUILD_OS/libboost_wserialization.* ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/
fi

if [ -d $INSTALLER_ASSEMBLY_DIRECTORY/asneg ]; then 
	rm -Rf $INSTALLER_ASSEMBLY_DIRECTORY/asneg 
fi

mkdir -p $INSTALLER_ASSEMBLY_DIRECTORY/$PACKTED_BUILD_DIR

cp -R ./$SOURCE_BUILD_DIR/* $INSTALLER_ASSEMBLY_DIRECTORY/$PACKTED_BUILD_DIR

cd $INSTALLER_ASSEMBLY_DIRECTORY

rm -R ./$PACKTED_BUILD_DIR/BuildConfig.h ./$PACKTED_BUILD_DIR/C* ./$PACKTED_BUILD_DIR/cmake*

cp -R ${INSTALLER_ASSEMBLY_DIRECTORY}extra_libs/* ./$PACKTED_BUILD_DIR
