## How to build and run the OPC UA server implementation ##

This README file describes the steps to build and run the  OPC UA server implementation. It also describes the client applications to connect to the OPC UA server. The [opcua-server repository](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/repository) is organized as follows:

- asneg - represents the AsNeG OPC UA implementation. 
- niki - contains the EDDL, IPSO, OPC UA LWM2M and OPC UA sensor interface implementations.
- cfg  - contains the configuration files to run the server implementation.
- opcua-build.sh - script to build the implementation.
- opcua-run.sh - script to run the implementation.

### 1. Tools Required ###

- boost 1.54 
- gcc 4.9
- cmake 
- openssl 
- UAExpert

### 2. Build process (Automated) ###

 * Clone the *opcua-server repository* and change to the *opcua-server* directory.
 * ``git submodule update --init --recursive``
 * Change to the *niki/opcua-lwm2m-server* directory.
 * `` git submodule update --init``
 * `` git apply wakaama.patch``
 * Change to the opcua-server directory.
* ``./opcua-build.sh``
* ``./opcua-run.sh``


### 3. Build process (Manual) ###

 * First 5 steps of Build process (Automated) same for Build process (Manual)
 * Change to *opcua-server/asneg* directory, create a build directory and change to that.
 * ``cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_INSTALL_PREFIX=./ ../src -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9``
 * Change to *opcua-server* directory.
 * ``cp -r cfg/etc asneg/build``  . This copies all configuration files required to run OPC UA server.
 * ``mkdir -p asneg/build/var/log/OpcUaStack``  .This creates directory for the log files
 * Change to the *opcua-server/asneg/build* directory.
 * ``./OpcUaServer OpcUaStack etc/OpcUaStack/OpcUaServer.xml`` to run the OPC UA server.


### 4. Client applications ###

 * [Clone](https://github.com/eclipse/wakaama) the Wakaama implementation, build and run the **Test Client example** to connect to the running OPC UA server.
 * [Download](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/files) the UaExpert OPC UA client application (Windows application) and install.
 * Start UaExpert and connect to OPC UA server by: `Server --> Add --> Double Click to Add Server (located under Custom Discovery)`.
 * Enter the URL `(e.g. opc.tcp://141.79.66.196:8888)`. Edit the IP to match the IP of your PC running the OPC UA server.
 * Expand **'>'** sign on the new OPC UA server until the transport profile `(e.g. None - None(uatcp-uasc-uabinary))` and then open to connect to OPC UA server.
 * Browse, read and write the OPC UA nodes in the Address Space pane on the left window pane of the UaExpert.
