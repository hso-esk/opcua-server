## How to build and run the OPC UA server implementation ##

This README file describes the steps to build and run the  OPC UA server implementation. It also describes the client applications to connect to the OPC UA server. The [opcua-server repository](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/repository) is organized as follows:

- asneg --> represents the AsNeG OPC UA implementation.
- asneg-dbserver --> contains AsNeG database server implementation.
- cfg -> contains the configuration files to run the server implementation.
- opcua-build-x86-amd64.sh --> script to build the implementation on x86 platform.
- opcua-build-arm.sh --> script to build the implementation on arm platform.
- opcua-run.sh --> script to run the implementation.
- niki --> contains the NIKI asneg server, EDDL, IPSO, LWM2M and Interface to Sensor device implementations.

### 1. Tools Required ###

- boost 1.54 
- gcc 4.9
- cmake 
- openssl 
- UAExpert
- MySQL

### 2. boost compilation ### 

- **Get gcc 4.9 with:** 

 ``sudo apt-get install gcc-4.9 g++4.9 cpp-4.9``

- **Create symbolic links for gcc with:**
```
 cd /usr/bin
 rm gcc g++ cpp
 ln -s gcc-4.9 gcc
 ln -s g++-4.9 g++
 ln -s cpp-4.9 cpp
```

- **Check if gcc is working with:**

 ``gcc -v``

- **For cross compilation tools install:**

``sudo apt install gcc-4.9-arm-linux-gnueabihf``

**and**
 
``sudo apt install g++-4.9-arm-linux-gnueabihf``

*On ubuntu 16.04 you will have to downlod and install g++ package manualy, you can get it from [xenial](https://packages.ubuntu.com/xenial/amd64/g++-4.9-arm-linux-gnueabihf/download)*

- **For cross compilation to work you will also need to make a symbolic link to opnesslconf.h file with:**

``sudo ln -s /usr/include/x86_64-linux-gnu/openssl/opensslconf.h /usr/include/openssl/``

- **Get boost 1.54 from:**

[sourceforge](https://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.bz2/download) 

- **Untar it in /usr/local/ or in a directory of you choice with:**

 ``tar --bzip2 -xf /path/to/boost_1_61_0.tar.bz2``

- **Downlad and install liboost dev tools from:**

[libboost-all-dev](https://launchpad.net/ubuntu/trusty/amd64/libboost-all-dev/1.54.0.1ubuntu1)

- **Modify BOOST_ROOT/boost/cstdint.hpp from:**

 ``#if defined(BOOST_HAS_STDINT_H) && (!defined(__GLIBC__) || defined(__GLIBC_HAVE_LONG_LONG))``

**to:**

 ``#if defined(BOOST_HAS_STDINT_H)``

- **Go to boost directory and run bootstrap.sh with:**

 ``./bootstrap.sh --prefix=/usr/local/ --toolset=gcc`` 

- **Build and install boost libraries with:**

 ``./b2 install --toolset=ARCHITECHTURE_TOOLSET --target-os=linux``

*ARCHITECHTURE_TOOLSET - your chosen architechture, **gcc** for x86/amd64, **gcc-arm** for arm*

- **When crosscompileing for arm, you have to modify project-config.jam, to do so run:** 

`` sudo gedit BOOST_ROOT/project-config.jam `` 

**and change the line:** 

``using gcc`` 

**to** 

``using gcc : arm : /usr/bin/arm-linux-gnueabihf-g++-4.9 ;`` 

**and then run with:**

``./b2 install toolset=gcc-arm``

- **If boost cross compilation for arm fails due to openssl not being recognized** 
*If you do not have openssl compiled for arm architechture you will have to cross compile it as well, or else boost compilation will fail. There is a scrip to compile openssl sources, get the source tar ball from [openssl](https://www.openssl.org/source/) and Build_for_arm.sh*

### 3. MySQL database installation and configuration ###

* `` sudo apt-get install mysql-server mysql-client``.
* ``sudo apt-get install unixodbc-dev``
* ``sudo apt-get install libmyodbc``
* Edit */etc/odbc.ini* as below. The Data source name (nikiDataSource), database user name (UserName) and database user password (nikiPassword) should match the configuration in *cfg/etc/OpcUaStack/Nodes/dbConfig.xml*.

		[nikiDataSource]
		Description = MySQL connection to  database
		Driver      = MySQL
		Server      = localhost
		User        = nikiUserName
		Password    = nikiPassword
		Port        = 3306
		Socket      = /var/run/mysqld/mysqld.sock
		ReadOnly    = No


* Edit */etc/odbcinst.ini* as below. Edit the MySQL odbc driver path (Driver = " ") and unix ODBC Driver (Setup = ""). For armhf platform, *Driver="/usr/lib/arm-linux-gnueabihf/odbc/libmyodbc.so"* and *Setup="/usr/lib/arm-linux-gnueabihf/odbc/libodbcmyS.so"*

		[MySQL]
		Description=ODBC for MySQL
		Driver=/usr/lib/i386-linux-gnu/odbc/libmyodbc.so
		Setup=/usr/lib/i386-linux-gnu/odbc/libodbcmyS.so
		FileUsage=1
		UsageCount=2

* Verify that the settings works `` odbcinst -q -d``. It should return the ouput as shown below.

		[MySQL]

* Log in to MySQL with root privileges to create niki database user name and password and the according  privileges by executing the following in the command line.
* ``mysql -u root -p``
* ``CREATE USER 'nikiUserName'@'localhost' IDENTIFIED BY 'nikiPassword';``
* ``GRANT ALL PRIVILEGES ON * . * TO 'nikiUserName'@'localhost';``
* ``FLUSH PRIVILEGES;``

### 4. Build process (Automated) ###

 * Clone the *opcua-server repository* and change to the *opcua-server* directory.
 * ``git submodule update --init --recursive``
 * Change to the *niki/opcua-lwm2m-server* directory.
 * `` git submodule update --init``
 * `` git apply wakaama.patch``
 * Change to the opcua-server directory.
 *  ``./opcua-build-x86-amd64.sh`` or  ``./opcua-build-arm.sh`` (depending on the target platform)
 * ``./opcua-run.sh``
 * *Building for arm might require some changes to arm-linux-toolchain.cmake file, when the project set-up differs*

### 5. Build process (Manual) ###

 * First 5 steps of Build process (Automated) same for Build process (Manual)
 * Change to *opcua-server/asneg* directory, create a build directory and change to that.
 * ``cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_INSTALL_PREFIX=./ ../src -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9``
 * Change to *opcua-server* directory.
 * ``cp -r cfg/etc asneg/build``  . This copies all configuration files required to run OPC UA server.
 * ``mkdir -p asneg/build/var/log/OpcUaStack``  .This creates directory for the log files
 * Change to the *opcua-server/asneg/build* directory.
 * ``./OpcUaServer OpcUaStack etc/OpcUaStack/OpcUaServer.xml`` to run the OPC UA server.


### 6. Client applications ###

 * [Clone](https://github.com/eclipse/wakaama) the Wakaama implementation, build and run the **Test Client example** to connect to the running OPC UA server.
 * [Download](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/files) the UaExpert OPC UA client application (Windows application) and install.
 * Start UaExpert and connect to OPC UA server by: `Server --> Add --> Double Click to Add Server (located under Custom Discovery)`.
 * Enter the URL `(e.g. opc.tcp://141.79.66.196:8888)`. Edit the IP to match the IP of your PC running the OPC UA server.
 * Expand **'>'** sign on the new OPC UA server until the transport profile `(e.g. None - None(uatcp-uasc-uabinary))` and then open to connect to OPC UA server.
 * Browse, read and write the OPC UA nodes in the Address Space pane on the left window pane of the UaExpert.
