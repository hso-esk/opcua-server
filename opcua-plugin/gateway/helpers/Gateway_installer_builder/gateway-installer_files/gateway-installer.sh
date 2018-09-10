#!/bin/bash

## Installer variables

rootDir="$(pwd)"

dataDir=data

permisions=755

sixLBRTag=6lbr-1.5.0

opcuaVersion=1.4.0

version_file="/etc/debian_version"

supported_versions=("8.7")

counter=3;

## Colours codes for the installer

RED='\033[1;31m'

GREEN='\033[1;32m'

BLUE='\033[1;34m'

PURPLE='\033[1;35m'

NC='\033[0m'

## Check the OS version

read version < $version_file

if [[ "${supported_versions[*]}" =~ "$version" ]]; then

	echo -e " $GREEN This version of Debian OS is supported. $NC"

	continue_flag="yes"

else

	echo -e "$RED This Linux version is not supported. Continue? $NC"

	read continue_flag

fi

## Start the instalation

if [[ "$continue_flag" == y* ]]; then

	## Get the time from NTP server

	apt-get update

	## Sync the system time with NTP server

	hwclock --systohc

	mkdir gateway-install

	cd gateway-install

	## Install basic packages

	echo -e "$BLUE Step 1 out of 9. Installing the necessary packages. $NC"

	apt-get install -y xml-twig-tools binutils build-essential autoconf automake git unixodbc-dev libmyodbc mysql-server mysql-client

	echo -e "$GREEN Step 1 was succesfully completed. $NC"

	## Configure ODBC

	echo -e "$BLUE Step 2 out of 9. Configuring the ODBC service. $NC"

	mv $rootDir/$dataDir/odbc.ini /etc/

	mv $rootDir/$dataDir/odbcinst.ini /etc/

	echo -e "$GREEN Step 2 was succesfully completed. $NC"

	## Enable UARTs of the Beagle Bone

	echo -e "$BLUE Step 3 out of 9. Configuring the UART peripheral. $NC"

	echo cape_enable=bone_capemgr.enable_partno=BB-UART1,BB-UART2,BB-UART3,BB-UART4,BB-UART5 >> /boot/uEnv.txt

	echo -e "$GREEN Step 3 was succesfully completed. $NC"

	## Install base packages for the 6LBR application

	echo -e "$BLUE Step 4 out of 9. Installing 6LBR service. $NC"

	apt-get install -y libncurses5-dev bridge-utils

	## Clone 6LBR sources

	git clone https://github.com/cetic/6lbr.git

	cd 6lbr

	git checkout $sixLBRTag

	git submodule update --init --recursive

	## Build the 6lbr application

	cd examples/6lbr

	make all

	make plugins

	make tools

	## Install 6LBR

	make install

	make plugins-install

	## Enable 6LBR Autostart

	update-rc.d 6lbr defaults

	echo -e "$GREEN Step 4 succesfully completed. $NC"

	## Configure 6lbr and network interfaces

	echo -e "$BLUE Step 5 out of 9. Configuring the network interfaces file. $NC"

	cp /etc/network/interfaces /etc/network/interfaces.old

	cp $rootDir/$dataDir/interfaces.tmp.dhcp /etc/network/interfaces

	cp $rootDir/$dataDir/6lbr.conf.tmp /etc/6lbr/6lbr.conf

	cp $rootDir/$dataDir/60dhcp.tmp /etc/6lbr/ifup.d/60dhcp

	cp $rootDir/$dataDir/nvm.dat /etc/6lbr/nvm.dat

	echo -e "$GREEN Step 5 succesfully completed. $NC"

	## Copy file and enable it

	echo -e "$BLUE Step 6 out of 9. Configuring the cc1310 startup procedure. $NC"

	cp $rootDir/$dataDir/cc1310-startup /etc/init.d/

	chown root:root /etc/init.d/cc1310-startup

	chmod $permisions /etc/init.d/cc1310-startup

	update-rc.d cc1310-startup defaults

	echo -e "$GREEN Step 6 succesfull completed. $NC"

	## Install Leshan Server

	echo -e "$BLUE Step 7 out of 9. Installign Leshan demo server. $NC"

	apt-get install -y default-jre default-jdk maven

	wget https://hudson.eclipse.org/leshan/job/leshan/lastSuccessfulBuild/artifact/leshan-server-demo.jar

	wget https://hudson.eclipse.org/leshan/job/leshan/lastSuccessfulBuild/artifact/leshan-bsserver-demo.jar

	mkdir -p /opt/leshan

	cp leshan-server-demo.jar leshan-bsserver-demo.jar /opt/leshan

	## Configure Leshan Server

	mkdir -p /etc/leshan

	cp $rootDir/$dataDir/leshan /etc/init.d/leshan

	cp $rootDir/$dataDir/leshan-bootstrap /etc/init.d/leshan-bootstrap

	cp $rootDir/$dataDir/bootstrap.json /etc/leshan/

	chmod $permisions /etc/init.d/leshan

	chmod $permisions /etc/init.d/leshan-bootstrap

	#update-rc.d leshan defaults

	#update-rc.d leshan-bootstrap defaults

	echo -e "$GREEN Step 7 succesfully completed. $NC"

	## Install the OPC UA server

	echo -e "$BLUE Step 8 out of 9. Installing OPC UA server. $NC"

	## If start up script does not exit or if it does, but is size 0 and if counter is greater than 0

	while [ ! -e /opt/opcua/opcua-run.sh ] || [ ! -s /opt/opcua/opcua-run.sh ] && [ $counter -gt 0 ]

	do	

		tar -pxvf $rootDir/$dataDir/*.tar -C /opt/ 

		counter=$(expr $counter - 1)

	done 

	if [ $counter -eq 0 ]

	then 

		echo -e "$RED Step 8 failed. Extraction of OPC UA server files has failed."

	fi

	chown root:root /opt/opcua

	chmod -R $permisions /opt/opcua

	## Install the OPC UA server startup files and crontab

	cp $rootDir/$dataDir/asneg /etc/init.d/

	chown root:root /etc/init.d/asneg

	chmod $permisions /etc/init.d/asneg

	cp $rootDir/$dataDir/asneg-wd /etc/init.d/

	chown root:root /etc/init.d/asneg-wd

	chmod $permisions /etc/init.d/asneg-wd

	cp $rootDir/$dataDir/crontab /etc/crontab

	chown root:root /etc/crontab

	chmod $permisions /etc/crontab

	#update-rc.d asneg defaults

	echo -e "$GREEN Step 8 succesfully completed. $NC"

	## Set up SQL server

	sleep 1

	if [ -e /opt/opcua/cfg/etc/OpcUaStack/Nodes/dbConfig.xml ]

	then

		echo -e "$BLUE Step 9 out of 9. Setting up the SQL database. $NC"

		sqlUserName="$(xml_grep 'UserName' /opt/opcua/cfg/etc/OpcUaStack/Nodes/dbConfig.xml --text_only)"

		sqlPassword="$(xml_grep 'Password' /opt/opcua/cfg/etc/OpcUaStack/Nodes/dbConfig.xml --text_only)"

		while : ; do 

			echo -e "$PURPLE Please enter the SQL root pasword: $NC"

			read -s rootPswd

			mysql --user=root --password=$rootPswd -e "CREATE USER '$sqlUserName'@'localhost' IDENTIFIED BY '$sqlPassword';"

			if [ "$?" -eq 0 ]

			then 

				break;

			else 

				echo -e "$RED Step 9 failled. Failed creating the user. Wrong mySQL root password. $NC"

			fi

		done

		mysql --user=root --password=$rootPswd -e "GRANT ALL PRIVILEGES ON * . * TO '$sqlUserName'@'localhost';"

		mysql --user=root --password=$rootPswd -e "FLUSH PRIVILEGES;"

		echo -e "$GREEN Step 9 succesfully completed. $NC"

	else 

		echo -e "$Red Step 9 failled. Server configuration files do not exist."

	fi

	sleep 1

if [[ $(pwd) != "/home/debian" ]]; then

	## Remove the install directory

	echo -e " $GREEN Removing instalation folder... $NC"

	cd /home/debian

	rm -rf ./${rootDir##*/}

fi

	echo -e "$PURPLE Please reboot your system now. $NC"

else

	echo -e "$RED Please install one of these supported Debian OS versions: ${supported_versions[*]} $NC"

fi

