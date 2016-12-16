## Building AsNeG implementation and NIKI modules ##

This file describes the steps to successfully build and run the AsNeG OPC UA server implementation and the NIKI modules. The [opcua-server repository](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/repository) is organized as follows:

- asneg - represents the AsNeG OPC UA implementation. 
- niki - contains the eddl and opcua-sensor-interface submodules. 
- cfg  - contains the AsNeG OPC UA server configuration files,  EDDL configuration files and sample EDDL files. 
- opcua-build.sh   - script used to automate the build process.
- opcua-run.sh - script used to automate the start and run process of the OPC UA server. 

### 1. Tools Required ###

- boost 1.54 
- gcc 4.9  
- cmake 
- openssl 
- UAExpert

### 2. Build the implementation ###

#### 2.1 Manual ####

- Clone the opcua-server repository into your local directory. 

- Create a build directory in the `opcua-server` directory.

- In the build subdirectory created, execute the following command in a terminal: 

	``` 
	cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=./ ../src -DCMAKE_C_COMPILER=gcc-4.9 -DCMAKE_CXX_COMPILER=g++-4.9 -DCMAKE_RANLIB=gcc-ranlib-4.9 -DCMAKE_AR=gcc-ar-4.9
	```
- Execute `make` to finally build the implemenation. 

#### 2.2 Automated ####

- Clone the opcua-server repository into your local directory. 

- [Download](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/repository/revisions/develop/show) `opcua-build.sh` and copy the script to the `opcua-server` directory.

- In the `opcua-server` directory, execute the script in a terminal as shown below: 

	`./opcua-build.sh`


### 3. Start and run the implementation ###

#### 3.1 Manual ####

- [Download](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/repository/revisions/develop/show) `cfg` and `niki` and copy the folders to the `opcua-server` directory. The organization of files in `cfg` and `niki` folders should be maintained as uploaded in the repository.
 
- In the `opcua-server/asneg/build/` directory, create the log directory: `var/log/OpcUaStack`. 
   
- In the `opcua-server/asneg/build/` directory, execute the following command in a terminal to start the OPC UA server application: 

	`./OpcUaServer OpcUaStack etc/OpcUaStack/OpcUaServer.xml` 

#### 3.1 Automated ####

- [Download](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/repository/revisions/develop/show) `opcua-run.sh` and copy the script to the `opcua-server` directory. 
 
- In the `opcua-server` directory, execute the script to start and run the OPC UA server application as shown below.   

	`./opcua-run.sh` 


### Connect OPC UA client to the running OPC UA server ###
- [Download](https://redmine.ivesk.hs-offenburg.de/projects/niki4-0/files) the UaExpert OPC UA client application to visualize the information exposed by the running OPC UA server. 

- Install and open the UaExpert application.

- Add and connect to the OPC UA server by: `Server --> Add --> Double Click to Add Server (located under Custom Discovery)`. 

- Enter the URL `(e.g. opc.tcp://141.79.66.196:8888)`. Edit the IP to match the IP of your PC running the OPC UA server.

-  Navigate to the added server, expand the **'>'** sign until the transport profile `(e.g. None - None(uatcp-uasc-uabinary))` and then open to connect to OPC UA server.  

- After successful connection to the running OPC UA server, the OPC UA nodes in the Address Space pane on the left-side of the OPC UA client window can be read, browsed or written to.  