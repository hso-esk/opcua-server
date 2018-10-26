#! /bin/bash -x

## Internal variables

MIN_ARGS=6
BUILD_TYPES=("release","debug")
ARCHS=("arm","x86-amd64")

PROC_COUNT="$(cat /proc/cpuinfo | awk '/^processor/{print $3}' | wc -l)"
PROC_COUNT=$((PROC_COUNT + 1))

## Internal functions 

usage () {

	echo "-t | --type	- specifies the build type (Release | Debug)"
	echo "-a | --arch	- specifies the architechture for which the build will be done (arm | x86-amd64)"
	echo "-b | --boost	- specifies the path to boost libraries"
	echo "-s | --openssl	- specifies the path to openssl libraries (only for arm builds)"
	echo "-d | --odbc	- specifies the path to the odbc librarie (only for arm builds)"
	echo "-h | --help	- prints this message"

}

if [ $# -lt $MIN_ARGS ]; 
then 
	usage
	exit 1
fi

while [ -n "$1" ];
do 
	case $1 in 
		-t | --type) 
			shift 
			if [[ "${BUILD_TYPES[*]}" =~ "${1,,}" ]];
			then
				BUILD_TYPE=$1
			else
				usage
				exit 1
			fi
			;;
		-a | --arch)
			shift
			if [[ "${ARCHS[*]}" =~ "${1,,}" ]]; 
			then
				if [ "$1" = "arm" ];
				then 
					ARCH="arm"
					TOOLCHAIN="arm-linux-toolchain.cmake"
					ARM_FLAG=1
				else
					ARCH="x86-amd64"
					TOOLCHAIN="x86-amd64-linux-toolchain.cmake"
					ARM_FLAG=0
				fi
			else
				usage
				exit 1
			fi
			;;
		-b | --boost) 
			shift 
			if [ -n "$1" ];
			then 
				BOOST_ROOT=$1
			else
				usage
				exit 1
			fi
			;;
		-s | --openssl)
			shift 
			if [ -n "$1" ];
			then
				OPENSSL_ROOT=$1
			else
				usage
				exit 1
			fi 
			;;
		-d | --odbc)
			shift
			if [ -n "$1" ];
			then 
				ODBC_ROOT=$1
			else
				usage
				exit 1
			fi
			;;
		-h | --help)
			usage 
			exit 0
			;;
	esac
	shift
done

# create the build directory
if [ -d "asneg/build-${ARCH,,}-${BUILD_TYPE,,}" ];
then
	rm -rf asneg/build-${ARCH,,}-${BUILD_TYPE,,}
	mkdir -p asneg/build-${ARCH,,}-${BUILD_TYPE,,}
else 
	mkdir -p asneg/build-${ARCH,,}-${BUILD_TYPE,,}
fi

cd asneg/build-${ARCH,,}-${BUILD_TYPE,,}

#build

if [ $ARM_FLAG -eq 0 ];
then
	OPENSSL_ROOT="/usr/local/ssl"
else
	sed -i "s34/*/SET(ENV{LDFLAGS} "\$ENV{LDFLAGS} -L$ODBC_ROOT")" $(pwd)/../../$TOOLCHAIN
fi

cmake -DOPENSSL_ROOT_DIR=$OPENSSL_ROOT -DOPENSSL_LIBRARIES=${OPENSSL_ROOT}/lib -DBOOST_ROOT=$BOOST_ROOT -DCMAKE_TOOLCHAIN_FILE=../../$TOOLCHAIN -DCMAKE_BUILD_TYPE=${BUILD_TYPE^^} ../src 
make --jobs=$PROC_COUNT
