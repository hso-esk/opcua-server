#! /bin/sh

#create the directory for the log files if not existent
mkdir -p asneg/build-arm-debug/var/log/OpcUaStack

#copy the according configuration files
cp -rf cfg/etc asneg/build-arm-debug

#run the OPC UA server
cd asneg/build-arm-debug
#./OpcUaServer OpcUaStack etc/OpcUaStack/OpcUaServer.xml
./OpcUaServer4 etc/OpcUaStack/OpcUaServer.xml
