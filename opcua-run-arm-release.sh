#! /bin/sh

#create the directory for the log files if not existent
mkdir -p asneg/build-arm-release/var/log/OpcUaStack

#copy the according configuration files
cp -rf cfg/etc asneg/build-arm-release

#run the OPC UA server
cd asneg/build-arm-release
#./OpcUaServer OpcUaStack etc/OpcUaStack/OpcUaServer.xml
./OpcUaServer3 etc/OpcUaStack/OpcUaServer.xml
