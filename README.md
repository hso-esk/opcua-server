## Building AsNeG implementation and NIKI modules ##
This file describes the steps to successfully build and run the AsNeG OPC UA server implementation and the NIKI modules. The [opcua-server repository](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/repository) is organized as follows:

- asneg - Represents the AsNeG OPC UA implementation. 
- niki  - Contains the eddl and opcua-sensor-interface submodules. 
- etc   - Contains the OPC UA configuration file, OPC UA information models, and our own defined EDDL configuration files.
- var   - Contains the log files.

### Tools Required ###
- boost 1.54 
- gcc 4.9  
- cmake 
- openssl 

### Build the implementation ###

- Clone the opcua-server repository into your local directory. 
- Create a build directory in the ***opcua-server/asneg*** directory.
- In the build subdirectory created, execute the command: ***cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=./ ../src -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9 -DCMAKE_RANLIB=gcc-ranlib-4.9 -DCMAKE_AR=gcc-ar-4.9*** in a terminal.
- Execute ***make*** to finally build the implemenation. 

### Start and run the implementation ###

- [Download](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/repository) the **etc** and **var** folders into the **opcua-server/asneg/build** directory. The organization of files in the **etc** and **var** folders should be maintained as uploaded in the repository.   
- Edit the IP of the **EndpointUrl** of the **OpcUaServer.xml** file located in the  **opcua-server/asneg/build/etc/OpcUaStack/OpcUaStack/** directory to match the IP of the PC or target you are running the OPC UA server. 
- In the **opcua-server/asneg/build/** directory, execute the command: ***./OpcUaServer OpcUaStack etc/OpcUaStack/OpcUaStack/OpcUaServer.xml*** in a terminal to start the OPC UA server application.

### Connect OPC UA client to the running OPC UA server ###
- [Download](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/files) the UaExpert OPC UA client application developed by Unified Automation to visualize the information exposed by the running OPC UA server. 
- Install and open the UaExpert application.
- Add and connect to the OPC UA server by: **Server --> Add --> Double Click to Add Server (located under Custom Discovery)**. 
- Enter the right URL **(e.g. opc.tcp://141.79.66.196:8888)**. Edit the IP to match the IP of your PC running the OPC UA server which you have already configured in the **OpcUaServer.xml** file.
-  Navigate to the added server, expand the **'>'** sign until the transport profile **(e.g. None - None(uatcp-uasc-uabinary))** and then open to connect to OPC UA server.  
- After successfully connecting to the running OPC UA server, the OPC UA nodes in the Address Space pane on the left-side of the OPC UA client window can be read, browsed or written to.  