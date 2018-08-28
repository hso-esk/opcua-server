#!/bin/sh 
#  @author: Dovydas Girdvainis 
#  @date  : 2018-05-12 
ROOT_DIR=$(pwd)

if ([ ! -z "$1" ] && [ ! -z "$2" ])
	then 
	if [ $1 = 'full' ] 
		then 
		apt-get install -y libssl-dev libicu-dev libicu55 mysql-client mysql-server openssl
		if [ $2 = '1_54_0' ]
			then
			./boost-build.sh 1_54_0 x86
			cd $ROOT_DIR
			./boost-build.sh 1_54_0 arm
		elif [ $2 = '1_67_0' ]
			then 
			./boost-build.sh 1_67_0 x86
			cd $ROOT_DIR
			./boost-build.sh 1_67_0 arm 
		else
			echo "Unsupported boost version detected. Supported versions: 1.54, 1.67" 
		fi
		./odbc-build.sh x86
		cd $ROOT_DIR
		./odbc-build.sh arm 
		cd $ROOT_DIR
		./openssl-build-arm.sh	
		cd $ROOT_DIR

		elif [ $1 = 'arm' ]
			then
			if [ $2 = '1_54_0' ]
				then
				./boost-build.sh 1_54_0 arm
				cd $ROOT_DIR
			elif [ $2 = '1_67_0' ]
				then 
				./boost-build.sh 1_67_0 arm
				cd $ROOT_DIR 
			else
				echo "Unsupported boost version detected. Supported versions: 1_54_0, 1_67_0" 
			fi
			./odbc-build.sh arm 
			cd $ROOT_DIR
			./openssl-build-arm.sh
			cd $ROOT_DIR
		
		elif [ $1 = 'x86' ]
			then 
			apt-get install -y libssl-dev libicu-dev libicu55  mysql-client mysql-server openssel
			if [ $2 = '1_54_0' ]
				then
				./boost-build.sh 1_54_0 x86
				cd $ROOT_DIR
			elif [ $2 = '1_67_0' ]
				then 
				./boost-build.sh 1_67_0 x86
				cd $ROOT_DIR
			else
				echo "Unsupported boost version detected. Supported versions: 1.54, 1.67" 
			fi
			./odbc-build.sh x86
			cd $ROOT_DIR

		else 
			echo "Unrecognized paramater detected. Accpeted values : full, arm or x86"
	fi 
	else 
		echo "No paramaterers provided. Please provide instalation type [ full || arm || x86 ] and boost version [ 1_54_0 || 1_67_0 ] "
fi
