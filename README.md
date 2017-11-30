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


### 2. MySQL database installation and configuration ###

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

### 3. Build process (Automated) ###

 * Clone the *opcua-server repository* and change to the *opcua-server* directory.
 * ``git submodule update --init --recursive``
 * Change to the *niki/opcua-lwm2m-server* directory.
 * `` git submodule update --init``
 * `` git apply wakaama.patch``
 * Change to the opcua-server directory.
*  ``./opcua-build-x86-amd64.sh`` or  ``./opcua-build-x86-amd64.sh`` (depending on the target platform)
* ``./opcua-run.sh``


### 4. Build process (Manual) ###

 * First 5 steps of Build process (Automated) same for Build process (Manual)
 * Change to *opcua-server/asneg* directory, create a build directory and change to that.
 * ``cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_INSTALL_PREFIX=./ ../src -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9``
 * Change to *opcua-server* directory.
 * ``cp -r cfg/etc asneg/build``  . This copies all configuration files required to run OPC UA server.
 * ``mkdir -p asneg/build/var/log/OpcUaStack``  .This creates directory for the log files
 * Change to the *opcua-server/asneg/build* directory.
 * ``./OpcUaServer OpcUaStack etc/OpcUaStack/OpcUaServer.xml`` to run the OPC UA server.


### 5. Client applications ###

 * [Clone](https://github.com/eclipse/wakaama) the Wakaama implementation, build and run the **Test Client example** to connect to the running OPC UA server.
 * [Download](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/files) the UaExpert OPC UA client application (Windows application) and install.
 * Start UaExpert and connect to OPC UA server by: `Server --> Add --> Double Click to Add Server (located under Custom Discovery)`.
 * Enter the URL `(e.g. opc.tcp://141.79.66.196:8888)`. Edit the IP to match the IP of your PC running the OPC UA server.
 * Expand **'>'** sign on the new OPC UA server until the transport profile `(e.g. None - None(uatcp-uasc-uabinary))` and then open to connect to OPC UA server.
 * Browse, read and write the OPC UA nodes in the Address Space pane on the left window pane of the UaExpert.
