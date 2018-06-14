#! /bin/sh

#create the directory for the log files if not existent
mkdir -p asneg/build-x86-amd64-debug/var/log/OpcUaStack

#copy the according configuration files
cp -rf cfg/etc asneg/build-x86-amd64-debug

#run the OPC UA server
cd asneg/build-x86-amd64-debug
#./OpcUaServer OpcUaStack etc/OpcUaStack/OpcUaServer.xml
./OpcUaServer4 etc/OpcUaStack/OpcUaServer.xml
