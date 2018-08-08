# OPC UA Server
The OPC UA Server as it is published here is based on the Open-Source implementation from ANSNeG [http://asneg.de/]. It usese the core repositories [https://81.169.197.52:8443/repositories/;jsessionid=1nnyhqp3g8p1pyndp1qgy8k3t] with slight modifications. Furthermore additional modules have been created e.g. to enable an abstractsensor interface and an adapter to connect to LWM2M devices. Therefore it uses the following submodules:
 - asneg (the core of the OPC UA Server)
 - asneg-db (database interaction for historical data)
 - wakaama (as the LWM2M server)
 - opcua-sensor-interface (abstract sensor interface)
 - opcua-lwm2m-server (interconnection between the LWM2M server and the OPC UA server)
 This allows LWM2M enabled devices to register at the OPC UA server and to share resources via OPC UA.

This work was originated from the NIKI 4.0 project. NIKI 4.0 was financed by the Baden-Württemberg Stiftung gGmbH (www.bwstiftung.de).  Project partners are FZI Forschungszentrum  Informatik am Karlsruher Institut für Technologie (www.fzi.de), Hahn-Schickard-Gesellschaft für angewandte Forschung e.V. (www.hahn-schickard.de) and Hochschule Offenburg (www.hs-offenburg.de).

## How to build and run the OPC UA server implementation ##

This README file describes the steps to build and run the  OPC UA server implementation. It also describes the client applications to connect to the OPC UA server. The [opcua-server repository](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/repository) is organized as follows:

- asneg --> represents the AsNeG OPC UA implementation.
- asneg-dbserver --> contains AsNeG database server implementation.
- cfg -> contains the configuration files to run the server implementation.
- opcua-build-x86-amd64.sh --> script to build the implementation on x86 platform.
- opcua-build-arm.sh --> script to build the implementation on arm platform.
- opcua-run.sh --> script to run the implementation.
- niki --> contains the NIKI asneg server, EDDL, IPSO, LWM2M, Interface to Sensor device implementations and Environment setup and gateway installation helpers.

### 1. Tools Required ###

- boost 1.54
- odbc driver 
- gcc 4.9
- cmake 
- openssl 
- UAExpert
- MySQL
- ssl encription

### 2. Environment setup 

A Visual Studio Code Setup has been provided in /.vscode to help set up the Visual Studio Code IDE envirnoment, thought it is not required.

Automation tools can be used to set up the environemt for the compilatiuon of the project. To use the automated environemt setup run: 
`/[OPCUA_ROOT]/niki/gateway/helpers/Dependency_Installers/dependency_installer.sh full 1_54_0` 

This script sets up the boost libraries, odbc driver, openssl and MySQL setup. Thus section 3 can be skipped by running this installer script.

### 3. MySQL database installation and configuration ###

```
sudo apt-get install mysql-server mysql-client
sudo apt-get install unixodbc-dev
sudo apt-get install libmyodbc
```

Edit */etc/odbc.ini* as below. The Data source name (nikiDataSource), database user name (UserName) and database user password (nikiPassword) should match the configuration in *cfg/etc/OpcUaStack/Nodes/dbConfig.xml*.

```
[nikiDataSource]
Description = MySQL connection to database
Driver      = MySQL
Server      = localhost
User        = nikiUserName
Password    = nikiPassword
Port        = 3306
Socket      = /var/run/mysqld/mysqld.sock
ReadOnly    = No
``` 

Edit */etc/odbcinst.ini* as below. Edit the MySQL odbc driver path (Driver = " ") and unix ODBC Driver (Setup = ""). 

For armhf platform, these lines should look like this: 

```
Driver="/usr/lib/arm-linux-gnueabihf/odbc/libmyodbc.so" 
Setup="/usr/lib/arm-linux-gnueabihf/odbc/libodbcmyS.so"
```

For an x86 system, these lines should look like this: 

```
Driver="/usr/lib/x86_64-linux-gnu/odbc/libmyodbc.so" 
Setup="/usr/lib/x86_64-linux-gnu/odbc/libodbcmyS.so"
```

Please take note, that for Ubuntu 16.04 and later odbc driver should be installed manually, by downloading it from [dev.mysql.com](https://dev.mysql.com/get/Downloads/Connector-ODBC/8.0/mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit.tar.gz). Once downloaded untar this archive, copy the entire content of ```lib/``` sub-directory to ```/usr/lib/x86_64-linux-gnu/odbc/```, also please take note, that the library file names from the odbc connector archive will be slighly different, from ``libmyodbc.so`` to ``libmyodbc8w.so``

Finally, the file should look like this: 

```
[MySQL]
Description=ODBC for MySQL
Driver=/usr/lib/x86_64-linux-gnu/odbc/libmyodbc.so
Setup=/usr/lib/x86_64-linux-gnu/odbc/libodbcmyS.so
FileUsage=1
UsageCount=2
```

Verify that the settings for the driver have been set correctlly, run `` odbcinst -q -d``. It should return the ouput as shown below.

```[MySQL]```

To verify the source settings for the odbc driver, run `` odbcinst -q -s `` 

```[nikiDataSource]```

Log in to MySQL with root privileges to create niki database user name and password and the according  privileges by executing the following in the command line.

```
mysql -u root -p
CREATE USER 'nikiUserName'@'localhost' IDENTIFIED BY 'nikiPassword';
GRANT ALL PRIVILEGES ON * . * TO 'nikiUserName'@'localhost';
FLUSH PRIVILEGES;
```

### 4. Build process (Automated) ###

1. Clone the *opcua-server repository* and change to the *opcua-server* directory.
2. Initialize sub direcotries with ``git submodule update --init --recursive``
3. Apply asneg patch with ``git apply asnegPatch.txt``
4. Change to the *niki/opcua-lwm2m-server* directory.
5. Apply wakaama patch with `` git apply wakaama.patch``
6. Change back to the opcua-server directory.
7. Build a release with ``./opcua-build-x86-amd64-realease.sh`` or ``./opcua-build-arm-release.sh`` (depending on the target platform)
8. Run the release with ``./opcua-run-x86-amd64-realease.sh`` or ``./opcua-run-arm-realease.sh``

Building for arm might require some changes to arm-linux-toolchain.cmake file, if the project set-up differs*

### 5. Build process (Manual) ###

1. First 6 steps of the build process described in section 4 are the same
2. Change to *opcua-server/asneg* directory
3. Create a build directory and change to it
4. Run a build command ``cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_INSTALL_PREFIX=./ ../src -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9``
5. Change to back to the root directory.
6. Copy configuration files to the build directory with``cp -r cfg/etc asneg/build``
7. Create a logging directory with ``mkdir -p asneg/build/var/log/OpcUaStack``
8. Change to the *opcua-server/asneg/build* directory.
9. Start the server with ``./OpcUaServer OpcUaStack etc/OpcUaStack/OpcUaServer.xml`` 

### 6. Client applications ###

 * [Clone](https://github.com/eclipse/wakaama) the Wakaama implementation, build and run the **Test Client example** to connect to the running OPC UA server.
 * [Download](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/files) the UaExpert OPC UA client application (Windows application) and install.
 * Start UaExpert and connect to OPC UA server by: `Server --> Add --> Double Click to Add Server (located under Custom Discovery)`.
 * Enter the URL `(e.g. opc.tcp://141.79.66.196:8888)`. Edit the IP to match the IP of your PC running the OPC UA server.
 * Expand **'>'** sign on the new OPC UA server until the transport profile `(e.g. None - None(uatcp-uasc-uabinary))` and then open to connect to OPC UA server.
 * Browse, read and write the OPC UA nodes in the Address Space pane on the left window pane of the UaExpert.
