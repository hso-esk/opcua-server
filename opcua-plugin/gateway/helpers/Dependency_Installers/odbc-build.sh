#!/bin/sh
#  @author: Manuel Schappacher
#  @author: Dovydas Girdvainis (Added OS check)
#  @date  : 2017-12-06
#
# Prerequisites to run the script
# On Debian/Ubuntu:
# arm-linux-gnueabihf-gcc-4.9 and arm-linux-gnueabihf-g++-4.9
# must be installed
VERSION=2.3.4
ODBC_VERSION=unixODBC-2.3.4
BASE_DIR=../../../../../../
FILES_DIR=$(pwd)

if [ ! -z "$1" ]
	then 
	if [ $1 = 'arm' ]
	then
		apt-get install -y gcc-4.9-arm-linux-gnueabihf
		mkdir $BASE_DIR/odbc-arm
		cd $BASE_DIR/odbc-arm
		INSTALL_DIR=$(pwd)
		ARM_FLAG=1
	else
		apt-get install -y gcc-4.9 g++-4.9 cpp-4.9 unixodbc-dev
		mkdir $BASE_DIR/odbc-x86
		cd $BASE_DIR/odbc-x86
		INSTALL_DIR=$(pwd)
		wget -c http://launchpadlibrarian.net/141005765/libmyodbc_5.1.10-3_amd64.deb
		dpkg -i libmyodbc_5.1.10-3_amd64.deb
		apt-get install -f
		wget -c https://dev.mysql.com/get/Downloads/Connector-ODBC/8.0/mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit.tar.gz 
		tar -xvzf mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit.tar.gz 
		cp mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit/lib/libmyodbc* /usr/lib/x86_64-linux-gnu/odbc
		rm ./*tar.gz
		cp $FILES_DIR/odbc.ini $FILES_DIR/odbcinst.ini /etc/
		echo -e "\033[1;35m Please enter the SQL root pasword: \033[0m"
		read -s rootPswd

		sqlUserName=opcuaUserName
		sqlPassword=opcuaPassword

		mysql --user=root --password=$rootPswd -e "CREATE USER '$sqlUserName'@'localhost' IDENTIFIED BY '$sqlPassword';"
		mysql --user=root --password=$rootPswd -e "GRANT ALL PRIVILEGES ON * . * TO '$sqlUserName'@'localhost';"
		mysql --user=root --password=$rootPswd -e "FLUSH PRIVILEGES;"

		chmod 777 -R $INSTALL_DIR
		ARM_FLAG=0
		exit 0
	fi
fi

if [ ! -e $ODBC_VERSION".tar.gz" ]; then
    wget -c http://www.unixodbc.org/$ODBC_VERSION.tar.gz
fi

if [ ! -e $ODBC_VERSION ]; then
	gzip -d $ODBC_VERSION".tar.gz"
    tar -xf $ODBC_VERSION".tar"
fi

if [ ! -e "$INSTALL_DIR" ]; then
   mkdir -p $INSTALL_DIR
fi

if [ ! -e "$INSTALL_DIR/lib/libodbc.a" ]; then
    cd $ODBC_VERSION"/"
    
    if [ $ARM_FLAG -eq 1 ]
	then
	./configure CC=/usr/bin/arm-linux-gnueabihf-gcc-4.9 CXX=/usr/bin/arm-linux-gnueabihf-g++-4.9 CFLAGS='-fPIC' CXXFLAGS='-fPIC' --target=arm-linux --host=x86 --prefix=$INSTALL_DIR --sysconfdir=/etc
    	else
 	./configure CC=/usr/bin/gcc-4.9 CXX=/usr/bin/g++-4.9 CFLAGS='-fPIC' CXXFLAGS='-fPIC' --target=x86 --host=x86 --prefix=$INSTALL_DIR --sysconfdir=/etc
    fi
    make
    make install

    chmod 777 -R ${BASE_DIR}/odbc*
fi
if [ ! -e "$INSTALL_DIR/lib/libodbc.a" ]; then
echo "======================================="
echo "$INSTALL_DIR/lib/libodbc.a not succefully build ... aborting"
echo "remove the cause of the error and start again"
exit 1
fi
