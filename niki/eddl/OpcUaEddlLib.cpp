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
 * This file was developed by [Kofi Atta Nsiah], [Hahn-Schickard-Gesellschaft
 * für angewandte Forschung e.V.]
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


#include "OpcUaEddlLib.h"
#include "EddlOpcUaMapping.h"
#include "OpcUaStackCore/Base/Log.h"
#include "OpcUaStackCore/Base/ConfigXml.h"
#include "OpcUaStackCore/Utility/Environment.h"
#include "OpcUaStackServer/ServiceSetApplication/ApplicationService.h"
#include "OpcUaStackServer/AddressSpaceModel/BaseNodeClass.h"
#include "OpcUaStackServer/AddressSpaceModel/ObjectNodeClass.h"
#include "OpcUaStackServer/AddressSpaceModel/VariableNodeClass.h"

#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include <cstring>


namespace OpcUaEddl
{

/**
 * OpcUaEddlLib()
 */
OpcUaEddlLib::OpcUaEddlLib(void)
  : OpcUaStackServer::ApplicationIf()
  , eddlfileName_("")
  , variableStartNodeId(0)
  , eddlParser_()
  , namespaceIndex_(0)
  , readSensorValueCallback_(boost::bind(&OpcUaEddlLib::readValue, this, _1))
  , writeSensorValueCallback_(boost::bind(&OpcUaEddlLib::writeValue, this, _1))
{
  Log(Debug, "OpcUaEddlLib::OpcUaEddlLib");
}

/**
 * ~OpcUaEddlLib()
 */
OpcUaEddlLib::~OpcUaEddlLib(void)
{
  Log(Debug, "OpcUaEddlLib::~OpcUaEddlLib");
}

/**
 * loadConfig()
 */
bool OpcUaEddlLib::loadConfig(void)
{
  Config config;
  ConfigXml configXml;

  /* read configuration file */
  config.alias("@CONF_DIR@", Environment::confDir());

  Log(Debug, "Loading application config")
      .parameter("ConfigFileName", applicationInfo()->configFileName());

  boost::filesystem::path configFileName(applicationInfo()->configFileName());
  if (configXml.parse(configFileName.string(), &config) == false) {
    Log(Error, "Error loading configuration")
      .parameter("ConfigFileName", configFileName.string())
      .parameter("Reason", configXml.errorMessage());
    return false;
  }

  /* read configuration parameter */
  boost::optional<std::string> eddlFileName = config.getValue("EddlModel.EddlPath.<xmlattr>.File");
  if (!eddlFileName) {
    Log(Error, "eddl path do not exist in configuration file")
      .parameter("Variable", "EddlModel.EddlPath.<xmlattr>.File")
      .parameter("ConfigFileName", applicationInfo()->configFileName());
    return false;
  } else {
    eddlfileName_ = *eddlFileName;
  }

  OpcUaEddlLib::objectNodeCreateInfo root_info;
  if (!config.getConfigParameter("EddlModel.ObjectNode.<xmlattr>.Id"
      , objectNodeInfo_.objectNodeId))
  {
    Log(Error, "Id does not exist in configuration file")
      .parameter("Variable", "EddlModel.ObjectNode.<xmlattr>.Id")
      .parameter("ConfigFileName", applicationInfo()->configFileName());
    return false;
  }
  if (!config.getConfigParameter("EddlModel.ObjectNode.<xmlattr>.BrowseName"
      , objectNodeInfo_.objectBrowseName)) {
    Log(Error, "BrowseName does not exist in configuration file")
      .parameter("Variable", "EddlModel.ObjectNode.<xmlattr>.BrowseName")
      .parameter("ConfigFileName", applicationInfo()->configFileName());
    return false;
  }
  if (!config.getConfigParameter("EddlModel.ObjectNode.<xmlattr>.Description"
      , objectNodeInfo_.objectDescription)) {
    Log(Error, "Description does not exist in configuration file")
      .parameter("Variable", "EddlModel.ObjectNode.<xmlattr>.Description")
      .parameter("ConfigFileName", applicationInfo()->configFileName());
    return false;
  }
  if (!config.getConfigParameter("EddlModel.ObjectNode.<xmlattr>.DisplayName"
      , objectNodeInfo_.objectDisplayName)) {
    Log(Error, "DisplayName does not exist in configuration file")
      .parameter("Variable", "EddlModel.ObjectNode.<xmlattr>.DisplayName")
      .parameter("ConfigFileName", applicationInfo()->configFileName());
    return false;
  }

  /* set the node id of the base Object in the address space of OPC UA server */
  objectNodeInfo_.parentNodeId = OpcUaId_ObjectsFolder;

  /* Read start nodeId of the Variable Node from the configuration file */
  if (!config.getConfigParameter("EddlModel.VariableNodes.<xmlattr>.FirstId", variableStartNodeId)) {
    Log(Error, "FirstId does not exist in configuration file")
      .parameter("Variable", "EddlModel.VariableNodes.<xmlattr>.FirstId")
      .parameter("ConfigFileName", applicationInfo()->configFileName());
    return false;
  }

  return true;
}

/**
 * startup()
 */
bool OpcUaEddlLib::startup(void)
{
  Log(Debug, "OpcUaEddlLib::startup");

  if (!loadConfig()) {
    return false;
  }

  /* parse EDDL file */
  if (!eddlParser_.parseEDDLfile(eddlfileName_, data_)) {
    return false;
  }

  /* process EDDL */
  std::vector<variableNodeCreateInfo> infos(processEddl(objectNodeInfo_.objectNodeId));

  /* create nodes from info parsed from eddl */
  if (!createNodes(objectNodeInfo_, infos)) {
    return false;
  }

  /* register read and write callbacks */
  if (!registerCallbacks()) {
    return false;
  }

  return true;
}

/**
 * processEDDL()
 */
std::vector<OpcUaEddlLib::variableNodeCreateInfo>
OpcUaEddlLib::processEddl(uint32_t parentNodeId)
{
  mapEddlToOpcUa v(parentNodeId, variableStartNodeId);
  for (auto const& data_item : data_) {
    boost::apply_visitor(v, data_item);
  }
  return v.infos;
}

/**
 * createRootNode()
 */
bool OpcUaEddlLib::createObjectNode(objectNodeCreateInfo const& info)
{
  OpcUaStackServer::BaseNodeClass::SPtr objectNode;
  objectNode = constructSPtr<OpcUaStackServer::ObjectNodeClass>();

  /* set node id of object */
  OpcUaNodeId objectNodeId;
  objectNodeId.set(info.objectNodeId, namespaceIndex_);
  objectNode->setNodeId(objectNodeId);

  /* set object node attributes */
  OpcUaQualifiedName browseName(info.objectBrowseName, namespaceIndex_);
  objectNode->setBrowseName(browseName);
  OpcUaLocalizedText description("de", info.objectDescription);
  objectNode->setDescription(description);
  OpcUaLocalizedText displayName("de", info.objectDisplayName);
  objectNode->setDisplayName(displayName);

  OpcUaNodeId baseObjectId;
  baseObjectId.set(info.parentNodeId, namespaceIndex_);
  /* set node id of OPC UA address space base object */
  OpcUaStackServer::BaseNodeClass::SPtr baseObject = informationModel()->find(baseObjectId);
  if (baseObject.get() != nullptr) {
    /* set reference to address space base object */
    baseObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
               ReferenceType_Organizes, true, objectNodeId);

    objectNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
                 ReferenceType_Organizes, false, baseObjectId);

    baseObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
                ReferenceType_HasComponent, true, objectNodeId);
  } else {
    return false;
  }

  /* add object node to OPC UA server information model */
  informationModel()->insert(objectNode);

  root_node_ = objectNode;
  return true;
}

/**
 * createVariableNode()
 */
bool OpcUaEddlLib::createVariableNode(variableNodeCreateInfo const& info)
{
  /* add variable node to the information model */
  OpcUaStackServer::BaseNodeClass::SPtr variableNode;
  variableNode = constructSPtr<OpcUaStackServer::VariableNodeClass>();

  OpcUaNodeId varNodeId;
  varNodeId.set(info.variableNodeId, namespaceIndex_);
  variableNode->setNodeId(varNodeId);

  OpcUaQualifiedName varbrowseName(info.variableBrowseName, namespaceIndex_);
  variableNode->setBrowseName(varbrowseName);
  OpcUaLocalizedText vardescription("de", info.variableDescription);
  variableNode->setDescription(vardescription);
  OpcUaLocalizedText  vardisplayName("de", info.variableDisplayName);
  variableNode->setDisplayName(vardisplayName);
  OpcUaByte accessLevel(info.accessLevel);
  variableNode->setAccessLevel(accessLevel);

  OpcUaNodeId objectNodeId;
  objectNodeId.set(info.parentNodeId, namespaceIndex_);

  /* create references to object node */
  variableNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
     ReferenceType_HasComponent, false, objectNodeId);

  root_node_->referenceItemMap().add(OpcUaStackServer::ReferenceType::
     ReferenceType_HasComponent, true, varNodeId);

  variableContext ctx;
  ctx.data_ = constructSPtr<OpcUaDataValue>();
  ctx.data_->statusCode(Success);
  ctx.data_->sourceTimestamp(boost::posix_time::microsec_clock::universal_time());
  ctx.data_->serverTimestamp(boost::posix_time::microsec_clock::universal_time());

  /* create device data file for variable node */
  ctx.dataFileObject_ = boost::make_shared<DeviceDataFile>(
      //objectNodeInfo_.objectDisplayName + "-" + info.var_display_name
      info.variableDisplayName
      , info.variableDescription
      , info.type
      , (DeviceData::ACCESS_READ | DeviceData::ACCESS_WRITE | DeviceData::ACCESS_OBSERVE));

  OpcUaNodeId dataTypeNodeId;

  /* check data type of value and set value of dataFileObject accordingly */
  if (info.type == DeviceDataValue::TYPE_INTEGER) {
    ctx.data_->variant()->variant(info.value.i32);
    DeviceDataValue val(info.type);
    val.setVal(info.value.i32);

    /* set value of dataFileObject */
    ctx.dataFileObject_->setVal(&val);

    /* set dataType to Int32_t */
    dataTypeNodeId.set(OpcUaId_Int32, namespaceIndex_);
  } else if (info.type == DeviceDataValue::TYPE_FLOAT) {
    ctx.data_->variant()->variant(info.value.f);

    /* set value of dataFileObject */
    DeviceDataValue val(info.type);
    val.setVal(info.value.f);
    ctx.dataFileObject_->setVal(&val);

    /* set dataType to Float */
    dataTypeNodeId.set(OpcUaId_Float, namespaceIndex_);
  } else if (info.type == DeviceDataValue::TYPE_STRING) {
    OpcUaString::SPtr str = constructSPtr<OpcUaString>();
    str->value(info.value.cStr);
    ctx.data_->variant()->variant(str);

    /* set value of dataFileObject_ */
    DeviceDataValue val(info.type);
    val.setVal(info.value.cStr);
    ctx.dataFileObject_->setVal(&val);

    /* set dataType to string */
    dataTypeNodeId.set(OpcUaId_String, namespaceIndex_);
  }

  /* set dataType of Variable Node */
  variableNode->setDataType(dataTypeNodeId);

  /* set value of Variable node to default value */
  variableNode->setValue(*ctx.data_);

  /* add variable node to OPC UA server information model */
  informationModel()->insert(variableNode);

  /* store variable node info into variableContextMap */
  variables_.insert(std::make_pair(varNodeId, ctx));

  return true;
}

/**
 * createNodes()
 */
bool OpcUaEddlLib::createNodes(objectNodeCreateInfo const& obj
    , std::vector<variableNodeCreateInfo> const& infos)
{
  Log(Debug, "OpcUaEddlLib::createNodes");

  if (!createObjectNode(obj)) {
    return false;
  }

  /* create all variable nodes from info stored in vector infos */
  for (auto const& info : infos) {
    Log(Debug, "Creating node")
        .parameter("id", info.variableNodeId)
        .parameter("name", info.variableBrowseName);

    if (!createVariableNode(info)) {
      return false;
    }
  }
  return true;
}

/**
 * shutdown()
 */
bool OpcUaEddlLib::shutdown(void)
{
  Log(Debug, "OpcUaEddlLib::shutdown");
  return true;
}

/**
 * registerCallbacks()
 */
bool OpcUaEddlLib::registerCallbacks(void)
{
  Log(Debug, "OpcUaEddlLib::registerCallbacks");

  ServiceTransactionRegisterForwardNode::SPtr trx
         = constructSPtr<ServiceTransactionRegisterForwardNode>();
  RegisterForwardNodeRequest::SPtr  req = trx->request();
  RegisterForwardNodeResponse::SPtr res = trx->response();

  req->forwardNodeSync()->readService().setCallback(readSensorValueCallback_);
  req->forwardNodeSync()->writeService().setCallback(writeSensorValueCallback_);
  req->nodesToRegister()->resize(variables_.size());

  uint32_t pos = 0;
  variableContextMap::iterator it;
  for (it = variables_.begin(); it != variables_.end(); it++) {
    OpcUaNodeId::SPtr nodeId = constructSPtr<OpcUaNodeId>();
    *nodeId = it->first;

    req->nodesToRegister()->set(pos, nodeId);
    pos++;
  }

  service().sendSync(trx);
  if (trx->statusCode() != Success) {
    Log(Error, "Response error.");
    return false;
  }

  for (pos = 0; pos < res->statusCodeArray()->size(); pos++) {
    OpcUaStatusCode statusCode;
    res->statusCodeArray()->get(pos, statusCode);
    if (statusCode != Success) {
      Log(Error, "Register value error");
      return false;
    }
  }

  return true;
}

/**
 * readValue()
 */
void OpcUaEddlLib::readValue(ApplicationReadContext* applicationReadContext)
{
  Log(Debug, "OpcUaEddlLib::readValue")
    .parameter("id", applicationReadContext->nodeId_);

  /* get nodeId of OPC UA Node client makes a read request to */
  variableContextMap::iterator it;
  it = variables_.find(applicationReadContext->nodeId_);
  if (it == variables_.end()) {
    applicationReadContext->statusCode_ = BadInternalError;
    return;
  }

  /* get updated value from data file */
  const DeviceDataValue* p_val = NULL;
  p_val = it->second.dataFileObject_->getVal();

  /* set data field of the Variable context with updated value from file */
  if (p_val->getType() == DeviceDataValue::TYPE_INTEGER) {
    int32_t readSensorVal = p_val->getVal().i32;
    it->second.data_->variant()->variant(readSensorVal);
  } else if (p_val->getType() == DeviceDataValue::TYPE_FLOAT) {
    float readSensorVal = p_val->getVal().f;
    it->second.data_->variant()->variant(readSensorVal);
  } else if (p_val->getType() == DeviceDataValue::TYPE_STRING) {
    std::string readSensorVal(p_val->getVal().cStr);
    OpcUaString::SPtr str = constructSPtr<OpcUaString>();
    str->value(readSensorVal);
    it->second.data_->variant()->variant(str);
  }

  /* update the OPC UA application read context with current value */
  applicationReadContext->statusCode_ = Success;
  it->second.data_->copyTo(applicationReadContext->dataValue_);
}

/**
 * writeValue()
 */
void OpcUaEddlLib::writeValue(ApplicationWriteContext* applicationWriteContext)
{
  Log(Debug, "OpcUaEddlLib::writeValue")
    .parameter("id", applicationWriteContext->nodeId_);

  /* get nodeId of OPC UA Node client makes a write request to */
  variableContextMap::iterator it;
  it = variables_.find(applicationWriteContext->nodeId_);
  if (it == variables_.end()) {
    applicationWriteContext->statusCode_ = BadInternalError;
    return;
  }
  applicationWriteContext->statusCode_ = Success;

  /* copy the data value written by the client from OPC UA server application
  application write context to the Variable context data field */
  applicationWriteContext->dataValue_.copyTo(*it->second.data_);

  /* check value type and update the data file object of the Variable Node */
  OpcUaVariant::SPtr value = it->second.data_->variant();
  if (value->variantType() == OpcUaStackCore::OpcUaBuildInType_OpcUaInt32) {
    DeviceDataValue val(DeviceDataValue::TYPE_INTEGER);
    val.setVal(value->get<int32_t>());
    it->second.dataFileObject_->setVal(&val);
  } else if (value->variantType() == OpcUaStackCore::OpcUaBuildInType_OpcUaFloat) {
    DeviceDataValue val(DeviceDataValue::TYPE_FLOAT);
    val.setVal(value->get<float>());
    it->second.dataFileObject_->setVal(&val);
  } else if (value->variantType() == OpcUaStackCore::OpcUaBuildInType_OpcUaString) {
    DeviceDataValue val(DeviceDataValue::TYPE_STRING);
    OpcUaVariantSPtr tmp = value->get<OpcUaVariantSPtr>();
    OpcUaString::SPtr str = boost::dynamic_pointer_cast<OpcUaString>(tmp.objectSPtr_);
    val.setVal(str->value());
    it->second.dataFileObject_->setVal(&val);
  }
}

} /* namespace OpcUaEddl */

extern "C" DLLEXPORT void  init(OpcUaStackServer::ApplicationIf** applicationIf) {
    *applicationIf = new OpcUaEddl::OpcUaEddlLib();
}

