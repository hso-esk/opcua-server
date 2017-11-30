/*
 * --- License -------------------------------------------------------------- *
 */

/*
 * Copyright 2017 NIKI 4.0 project team
 *
 * NIKI 4.0 was financed by the Baden-Württemberg Stiftung gGmbH (www.bwstiftung.de).
 * Project partners are FZI Forschungszentrum Informatik am Karlsruher
 * Institut für Technologie (www.fzi.de), Hahn-Schickard-Gesellschaft
 * für angewandte Forschung e.V. (www.hahn-schickard.de) and
 * Hochschule Offenburg (www.hs-offenburg.de).
 * This file was developed by the Institute of reliable Embedded Systems
 * and Communication Electronics
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/


#ifndef OPCUAEDDL_OPCUAEDDLLIB_H_
#define OPCUAEDDL_OPCUAEDDLLIB_H_


#include "EddlParser.h"
#include "OpcUaStackCore/Application/ApplicationReadContext.h"
#include "OpcUaStackCore/Application/ApplicationWriteContext.h"
#include "OpcUaStackServer/Application/ApplicationIf.h"
#include "OpcUaStackServer/AddressSpaceModel/BaseNodeClass.h"
#include "DeviceDataValue.h"
#include "DeviceDataFile.h"

namespace OpcUaEddl
{

class OpcUaEddlLib
  : public OpcUaStackServer::ApplicationIf
{

public:

/**
 * \brief   Default Constructor
 */
OpcUaEddlLib(void);

/**
 * \brief   Default Destructor.
 */
virtual ~OpcUaEddlLib(void);

/**
 * \brief   Struct declaration for Variable context.
 */
struct variableContext
{
  OpcUaDataValue::SPtr data_;
  boost::shared_ptr<DeviceDataFile> dataFileObject_;
};

/**
 * \brief   Struct declaration for object Node.
 *
 *     Contains information to create object Node.
 */
struct objectNodeCreateInfo
{
  /* nodeId of object Node */
  uint32_t objectNodeId;

  /* browse Name of object Node */
  std::string objectBrowseName;

  /* description of object Node */
  std::string objectDescription;

  /* display Name of object Node */
  std::string objectDisplayName;

  /* nodeId of base object Node in information model */
  uint32_t parentNodeId;
};

/**
 * \brief   Struct declaration for variable Node.
 *
 *     Contains information to create variable Node.
 */
struct variableNodeCreateInfo
{
  /* nodeId of variable Node */
  uint32_t variableNodeId;

  /* browse Name of variable Node */
  std::string variableBrowseName;

  /* description of variable Node */
  std::string variableDescription;

  /* display Name of variable Node */
  std::string variableDisplayName;

  /* access level of variable Node */
  OpcUaByte accessLevel;

  /* nodeId of parent object Node */
  uint32_t parentNodeId;

  /* type of data value of variable Node */
  DeviceDataValue::e_type type;

  /* data value of variable Node */
  DeviceDataValue::u_val value;
};

/**
 * \brief   Starts up the OpcUaEddl library.
 */
virtual bool startup(void);

/**
 * \brief   Shuts down the OpcUaEddl library.
 */
virtual bool shutdown(void);


private:

/**
 * \brief   Load configuration file of OpcUaEddl library.
 */
bool loadConfig(void);

/**
 * \brief   Create nodes to the information model.
 */
bool createNodes(objectNodeCreateInfo const& root
    , std::vector<variableNodeCreateInfo> const& infos);

/**
 * \brief   Create Object Node to the information model.
 */
bool createObjectNode(objectNodeCreateInfo const& info);

/**
 * \brief   Create Variable Node to the information model.
 */
bool createVariableNode(variableNodeCreateInfo const& info);

/**
 * \brief   Process EDDL data.
 *
 * \param   parent_node_id    Nodeid of parent object node.
 * \return  std::vector<node_create_info>   Vector of variable Node information.
 */
std::vector<variableNodeCreateInfo> processEddl(uint32_t parentNodeId);

/**
 * \brief   ReadValue updates the data value of the OPC UA server applicationRead context.
 *
 *     This function is mapped to the read callback function and is triggered if
 *     OPC UA client makes a read request of OPC UA node in the information model.
 */
void readValue(ApplicationReadContext* applicationReadContext);

/**
 * \brief   WriteValue updates the data value field of OPC UA server application context.
 *
 *          This function is mapped to the write callback function and is triggered if
 *          OPC UA client updates the value of OPC UA node in the information model.
 */
void writeValue(ApplicationWriteContext* applicationWriteContext);

/**
 * \brief   RegisterCallbacks registers read and write callback functions.
 *
 */
bool registerCallbacks(void);

private:
    std::string eddlfileName_;
    objectNodeCreateInfo objectNodeInfo_;
    uint32_t variableStartNodeId;

    typedef std::map<OpcUaNodeId, variableContext> variableContextMap;
    variableContextMap variables_;

    /* instantiate eddlParser and storage to store parsed EDDL */
    EddlParser eddlParser_;
    EddlParser::eddlParsedData data_;

    OpcUaUInt16 namespaceIndex_;
    OpcUaStackServer::BaseNodeClass::SPtr root_node_;

    /* read and write callback functions */
    Callback readSensorValueCallback_;
    Callback writeSensorValueCallback_;
};

} /* namespace OpcUaEddl */


#endif /* OPCUAEDDL_OPCUAEDDLLIB_H_ */
