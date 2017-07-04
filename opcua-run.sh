#! /bin/sh

#create the directory for the log files if not existent
#mkdir -p asneg/build/var/log/OpcUaStack

#copy the according configuration files
cp -rf cfg/etc asneg/build

#run the OPC UA server
cd asneg/build
#./OpcUaServer OpcUaStack etc/OpcUaStack/OpcUaServer.xml
./OpcUaServer3 etc/OpcUaStack/OpcUaServer.xml
