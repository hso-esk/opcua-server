/*
 * OpcUaIPSOLib.cpp
 *
 *  Created on: 24 Feb 2017
 *      Author: osboxes
 */

#include "OpcUaLWM2MLib.h"
#include "OpcUaStackCore/Base/Log.h"
#include "OpcUaStackCore/Utility/Environment.h"
#include "OpcUaStackServer/ServiceSetApplication/ApplicationService.h"
#include "OpcUaStackServer/AddressSpaceModel/BaseNodeClass.h"
#include "OpcUaStackServer/AddressSpaceModel/ObjectNodeClass.h"
#include "OpcUaStackServer/AddressSpaceModel/VariableNodeClass.h"
#include "OpcUaStackCore/Base/ConfigXml.h"
#include "LWM2MDevice.h"
#include "LWM2MResource.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include <algorithm>
#include <string>

namespace OpcUaLWM2M
{

/**
 * OpcUaLWM2MLib()
 */
OpcUaLWM2MLib::OpcUaLWM2MLib(void)
  : OpcUaStackServer::ApplicationIf()
  , ipsofileName_("")
  , namespaceIndex_(0)
  , opcUalwm2mObs_(*this)
  , readSensorValueCallback_(boost::bind(&OpcUaLWM2MLib::readSensorValue, this, _1))
  , writeSensorValueCallback_(boost::bind(&OpcUaLWM2MLib::writeSensorValue, this, _1))
  , methodCallback_(boost::bind(&OpcUaLWM2MLib::callSensorMethod, this, _1))
{
  Log(Debug, "OpcUaLWM2MLib::OpcUaLWM2MLib");
}

/*---------------------------------------------------------------------------*/
/**
 * ~OpcUaLWM2MLib()
 */
OpcUaLWM2MLib::~OpcUaLWM2MLib(void)
{
  Log(Debug, "OpcUaLWM2MLib::~OpcUaLWM2MLib");
}

/*---------------------------------------------------------------------------*/
/**
 * startup()
 */
bool OpcUaLWM2MLib::startup(void)
{
  Log(Debug, "OpcUaLWM2MLib::startup");

  /* load config file */
  if (!loadConfig()) {
    Log(Error, "Could not load config file");
    return false;
  }

  /* parse the IPSO file */
  for (auto& ipsofileName : ipsofileNameVec_)
  {
    if (!ipsoParser_.parseIPSOfile(ipsofileName, data_))
    {
	    Log(Error, "Parsing of IPSO file failed");
	    return false;
    }
  }

  /* create object dictionary */
  if (!createObjectDictionary(data_))
  {
    Log(Error, "Creation of Object dictionary failed");
    return false;
  }

  /* create an instance of LWM2M server */
  lwm2mServer_ = boost::make_shared<LWM2MServer>();

  /* start the the LWM2M server */
  lwm2mServer_->startServer();

  Log (Debug, "LWM2M server started");

  /* register the OPC UA server observer object */
  lwm2mServer_->registerObserver(&opcUalwm2mObs_);

  /* Wait for LWM2M client connections */
  std::cout << "Waiting for incoming connections" << std::endl;

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * shutdown()
 */
bool OpcUaLWM2MLib::shutdown()
{
  Log(Debug, "OpcUaLWM2MLib::shutdown");

  /* deregister OpcUa LWM2M server observer */
  lwm2mServer_->deregisterObserver(&opcUalwm2mObs_);

  /* stop the LWM2M server */
  lwm2mServer_->stopServer();

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * loadConfig()
 */
bool OpcUaLWM2MLib::loadConfig(void)
{
  Log(Debug, "OpcUaLWM2MLib::loadConfig");

  Config config;
  ConfigXml configXml;

  /* read configuration file */
  config.alias("@BIN_DIR@", Environment::binDir());
  config.alias("@CONF_DIR@", Environment::confDir());
  config.alias("@LOG_DIR@", Environment::logDir());
  config.alias("@INSTALL_DIR@", Environment::installDir());

  Log(Debug, "Loading application config")
    .parameter("ConfigFileName", applicationInfo()->configFileName());

  boost::filesystem::path configFileName(applicationInfo()->configFileName());
  if (configXml.parse(configFileName.string(), &config) == false) {
    Log(Error, "Error loading configuration")
      .parameter("ConfigFileName", configFileName.string())
      .parameter("Reason", configXml.errorMessage());
    return false;
  }
  std::vector<Config> ipsoConfigVec;
  config.getChilds("IPSOModel.IPSOPath", ipsoConfigVec);

  if (ipsoConfigVec.size() == 0) {
    Log(Error, "IPSO XML file does not exist");
    return false;
  }

  for (auto& ipsoConfig : ipsoConfigVec)
  {
    /* read configuration parameter */
    boost::optional<std::string> ipsoFileName = ipsoConfig.getValue("<xmlattr>.File");
    if (!ipsoFileName) {
      Log(Error, "eddl path do not exist in configuration file")
        .parameter("Variable", "IPSOModel.IPSOPath.<xmlattr>.File")
        .parameter("ConfigFileName", applicationInfo()->configFileName());
      return false;
    } else {
      ipsofileName_ = *ipsoFileName;
      ipsofileNameVec_.push_back(ipsofileName_);
    }
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * readSensorValue()
 */
void OpcUaLWM2MLib::readSensorValue (ApplicationReadContext* applicationReadContext)
{
  Log(Debug, "OpcUaLWM2MLib::readSensorValue")
    .parameter("id", applicationReadContext->nodeId_);

  /* get nodeId of OPC UA Node client makes a read request to */
  variableContextMap::iterator it;
  it = variables_.find(applicationReadContext->nodeId_);
  if (it == variables_.end()) {
    applicationReadContext->statusCode_ = BadInternalError;
  }

  if (!it->second.dataObject)  {
    applicationReadContext->statusCode_ = BadInternalError;
  }

  /* get updated value */
  const DeviceDataValue* p_val = NULL;
  p_val = it->second.dataObject->getVal();

  /* update value of OPC UA variable node */
  if (p_val->getType() == DeviceDataValue::TYPE_INTEGER) {
    int32_t readSensorVal = p_val->getVal().i32;
    it->second.data->variant()->variant(readSensorVal);

  } else if (p_val->getType() == DeviceDataValue::TYPE_FLOAT) {
    float readSensorVal = p_val->getVal().f;
    it->second.data->variant()->variant(readSensorVal);

  } else if (p_val->getType() == DeviceDataValue::TYPE_STRING) {
    std::string readSensorVal(p_val->getVal().cStr);
    OpcUaString::SPtr str = constructSPtr<OpcUaString>();
    str->value(readSensorVal);
    it->second.data->variant()->variant(str);
  }

  /* copy updated value to application read context */
  applicationReadContext->statusCode_ = Success;
  it->second.data->copyTo(applicationReadContext->dataValue_);
}

/*---------------------------------------------------------------------------*/
/**
 * writeSensorValue()
 */
void OpcUaLWM2MLib::writeSensorValue (ApplicationWriteContext* applicationWriteContext)
{
  Log(Debug, "OpcUaLWM2MLib::writeSensorValue")
    .parameter("id", applicationWriteContext->nodeId_);

  /* get nodeId of OPC UA Node client makes a write request to */
  variableContextMap::iterator it;
  it = variables_.find(applicationWriteContext->nodeId_);
  if (it == variables_.end()) {
    applicationWriteContext->statusCode_ = BadInternalError;
  }

  if (!it->second.dataObject)  {
    applicationWriteContext->statusCode_ = BadInternalError;
  }
  applicationWriteContext->statusCode_ = Success;

  /* copy updated value from application write context */
  applicationWriteContext->dataValue_.copyTo(*it->second.data);

  /* check and write updated value to corresponding OPC UA variable node */
  OpcUaVariant::SPtr value = it->second.data->variant();
  if (value->variantType() == OpcUaStackCore::OpcUaBuildInType_OpcUaInt32) {
    DeviceDataValue val(DeviceDataValue::TYPE_INTEGER);
    val.setVal(value->get<int32_t>());
    it->second.dataObject->setVal(&val);

  } else if (value->variantType() == OpcUaStackCore::OpcUaBuildInType_OpcUaFloat) {
    DeviceDataValue val(DeviceDataValue::TYPE_FLOAT);
    val.setVal(value->get<float>());
    it->second.dataObject->setVal(&val);

  } else if (value->variantType() == OpcUaStackCore::OpcUaBuildInType_OpcUaString) {
    DeviceDataValue val(DeviceDataValue::TYPE_STRING);
    OpcUaVariantSPtr tmp = value->get<OpcUaVariantSPtr>();
    OpcUaString::SPtr str = boost::dynamic_pointer_cast<OpcUaString>(tmp.objectSPtr_);
    val.setVal(str->value());
    it->second.dataObject->setVal(&val);
  }
}
/*---------------------------------------------------------------------------*/
/**
 * registerResourceCallback()
 */
bool OpcUaLWM2MLib::registerCallbacks(OpcUaUInt32 id)
{
  Log(Debug, "OpcUaLWM2MLib::registerCallbacks");

  OpcUaNodeId::SPtr nodeId = constructSPtr<OpcUaNodeId>();
  nodeId->set(id, namespaceIndex_);

  ServiceTransactionRegisterForward::SPtr trx = ServiceTransactionRegisterForward::construct();
  RegisterForwardRequest::SPtr req = trx->request();
  RegisterForwardResponse::SPtr res = trx->response();

  req->forwardInfoSync()->setReadCallback(readSensorValueCallback_);
  req->forwardInfoSync()->setWriteCallback(writeSensorValueCallback_);
  req->nodesToRegister()->resize(1);
  req->nodesToRegister()->set(0, nodeId);

  service().sendSync(trx);
  if (trx->statusCode() != Success) {
    Log(Error, "Status code response error");
    return false;
  }

  OpcUaStatusCode statusCode;
  res->statusCodeArray()->get(0, statusCode);
  if (statusCode != Success) {
    Log(Error, "Status Code Array response error");
    return false;
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * UnregisterResourceCallback()
 */
bool OpcUaLWM2MLib::unregisterCallbacks()
{
  Log(Debug, "OpcUaLWM2MLib::unregisterCallbacks");

  ServiceTransactionRegisterForward::SPtr trx = ServiceTransactionRegisterForward::construct();
  RegisterForwardRequest::SPtr req = trx->request();
  RegisterForwardResponse::SPtr res = trx->response();

  req->forwardInfoSync()->unsetReadCallback();
  req->forwardInfoSync()->unsetWriteCallback();
  req->nodesToRegister()->clear();

  service().sendSync(trx);
  if (trx->statusCode() != Success) {
    Log(Error, "Status code response error");
    return false;
  }

  OpcUaStatusCode statusCode;
  res->statusCodeArray()->get(0, statusCode);
  if (statusCode != Success) {
    Log(Error, "Status Code Array response error");
    return false;
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createObjectDictionary()
 */
bool OpcUaLWM2MLib::createObjectDictionary(IPSOParser::ipsoDescriptionVec& ipsoDesc)
{
  Log(Debug, "OpcUaLWM2MLib::createObjectDictionary");

  for (auto& desc : ipsoDesc) {
    if (!objectDictionary_.insert(objectDictionary_t::value_type(
        desc.objectDesc.id, &desc)).second) {
      Log(Error, "Object dictionary creation from IPSO descriptions failed");
      return false;
    }
  }
  return true;
}
/*---------------------------------------------------------------------------*/
namespace
{
/**
 * offset used internally to avoid ID conflicts
 */
/* offset for OPC UA object nodes */
static uint32_t offset()
{
  static uint32_t ID = 40000;
  return ID++;
}

/* offset for OPC UA variable nodes */
static uint32_t offset2()
{
  static uint32_t ID = 60000;
  return ID++;
}

/* offset for OPC UA device nodes */
static uint32_t offset3()
{
  static uint32_t ID = 20000;
  return ID++;
}

} /* anonymous namespace */

/*---------------------------------------------------------------------------*/
/**
 * createObjectNode()
 */
bool OpcUaLWM2MLib::createObjectNode(objectMap_t& objectMap)

{
  Log(Debug, "OpcUaLWM2MLib::createObjectNode");

  for (auto& objectInfo : objectMap)
  {
    /* set id of OPC UA object node */
    OpcUaNodeId objectNodeId;
    objectNodeId.set(objectInfo.first, namespaceIndex_);

    OpcUaStackServer::BaseNodeClass::SPtr objectNode = informationModel()->find(objectNodeId);
    if (objectNode.get() == nullptr) {

      OpcUaStackServer::BaseNodeClass::SPtr objectNode;
      objectNode = OpcUaStackServer::ObjectNodeClass::construct();
      objectNode->setNodeId(objectNodeId);

      /* set object node attributes */
      OpcUaQualifiedName browseName(objectInfo.second.name, namespaceIndex_);
      objectNode->setBrowseName(browseName);

      OpcUaLocalizedText description("de", objectInfo.second.desc);
      objectNode->setDescription(description);

      std::string objectDisplayName = objectInfo.second.name
            + std::string("<")
            + boost::lexical_cast<std::string>(objectInfo.second.instanceId)
            + std::string(">");

      OpcUaLocalizedText displayName("de", objectDisplayName);
      objectNode->setDisplayName(displayName);

      /* set event notifier attribute of object Node */
      OpcUaByte executable = 1;
      objectNode->setEventNotifier(executable);

      /* set access rights */
      OpcUaUInt32  writemask= 0x00;
      objectNode->setWriteMask(writemask);
      objectNode->setUserWriteMask(writemask);

      /* set OPC UA base object id */
      OpcUaNodeId parentObjectId;
      parentObjectId.set(deviceId_, namespaceIndex_);

      /* check existence of parent object */
      OpcUaStackServer::BaseNodeClass::SPtr baseObject = informationModel()->find(parentObjectId);
      if (baseObject.get() != nullptr) {

        /* set reference to parent object */
        baseObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
              ReferenceType_Organizes, true, objectNodeId);

        objectNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
              ReferenceType_Organizes, false, parentObjectId);

        baseObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
              ReferenceType_HasComponent, true, objectNodeId);

        baseObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
              ReferenceType_HasComponent, false, parentObjectId);
      } else {
        return false;
      }

      /* add object node to OPC UA server information model */
      informationModel()->insert(objectNode);
    }
  }
  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createObjectNode()
 */
bool OpcUaLWM2MLib::createDeviceObjectNode(const LWM2MDevice* device)
{

  Log(Debug, "OpcUaLWM2MLib::createDeviceObjectNode");

  OpcUaStackServer::BaseNodeClass::SPtr deviceobjectNode;
  deviceobjectNode = OpcUaStackServer::ObjectNodeClass::construct();

  /* set node id of object */
  OpcUaNodeId deviceObjectNodeId;
  deviceId_ = offset3();
  deviceObjectNodeId.set(deviceId_, namespaceIndex_);
  deviceobjectNode->setNodeId(deviceObjectNodeId);

  /* set object node attributes */
  OpcUaQualifiedName browseName(device->getName(), namespaceIndex_);
  deviceobjectNode->setBrowseName(browseName);
  OpcUaLocalizedText description("de", device->getName());
  deviceobjectNode->setDescription(description);
  OpcUaLocalizedText displayName("de", device->getName());
  deviceobjectNode->setDisplayName(displayName);

  /* set event notifier attribute of object Node */
  OpcUaByte executable = 0;
  deviceobjectNode->setEventNotifier(executable);

  /* set OPC UA base object id */
  OpcUaNodeId baseObjectId;
  baseObjectId.set(OpcUaId_ObjectsFolder, namespaceIndex_);

  /* set access permissions */
  OpcUaUInt32  writemask= 0x00;
  deviceobjectNode->setWriteMask(writemask);
  deviceobjectNode->setUserWriteMask(writemask);

  /* set node id of OPC UA address space base object */
  OpcUaStackServer::BaseNodeClass::SPtr baseObject = informationModel()->find(baseObjectId);

  if (baseObject.get() != nullptr) {

    /* set reference to address space base object */
    baseObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
	        ReferenceType_Organizes, true, deviceObjectNodeId);

    deviceobjectNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
          ReferenceType_Organizes, false, baseObjectId);

    OpcUaNodeId typeNodeId;
    typeNodeId.set(OpcUaId_FolderType);
    deviceobjectNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
          ReferenceType_HasTypeDefinition, true, typeNodeId);

    baseObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
          ReferenceType_HasComponent, true, deviceObjectNodeId);
  } else {
    return false;
  }

   /* add object node to OPC UA server information model */
   informationModel()->insert(deviceobjectNode);

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createVariableNode()
 */
bool OpcUaLWM2MLib::createVariableNode (resourceMap_t& resourceMap)
{
  Log(Debug, "OpcUaLWM2MLib::createVariableNode");

  /* iterate through resource map and create OPC UA variable nodes */
  for (auto& varInfo : resourceMap)
  {
    OpcUaStackServer::BaseNodeClass::SPtr variableNode;
    variableNode = OpcUaStackServer::VariableNodeClass::construct();

    /* set OPC UA variable node id */
    OpcUaNodeId varNodeId;
    uint32_t resourceId =  offset2();

    varNodeId.set(resourceId, namespaceIndex_);
    variableNode->setNodeId(varNodeId);

    /* set variable node attributes */
    OpcUaQualifiedName varbrowseName(varInfo.second.name, namespaceIndex_);
    variableNode->setBrowseName(varbrowseName);

    OpcUaLocalizedText vardescription("de", varInfo.second.desc);
    variableNode->setDescription(vardescription);

    OpcUaLocalizedText  vardisplayName("de", varInfo.second.name);
    variableNode->setDisplayName(vardisplayName);

    OpcUaByte accessLevel(0x4F);
    variableNode->setAccessLevel(accessLevel);

    OpcUaByte userAccessLevel(0x4F);
    variableNode->setUserAccessLevel(userAccessLevel);

    OpcUaDouble minimumSamplingInterval = 1;
    variableNode->setMinimumSamplingInterval(minimumSamplingInterval);

    /* Set attribute to allow server to collect historical data */
    OpcUaBoolean historizing = true;
    variableNode->setHistorizing(historizing);

    /* set access permissions */
    OpcUaUInt32  writemask= 0x01;
    variableNode->setWriteMask(writemask);
    variableNode->setUserWriteMask(writemask);

    /* set value attribute to scalar or array */
    OpcUaInt32 valueRank = -2;
    variableNode->setValueRank(valueRank);

    /* set Array Dimensions */
    OpcUaUInt32Array arrayDimension;
    arrayDimension.resize(0);
    variableNode->setArrayDimensions(arrayDimension);

    /* set node id of parent object */
    OpcUaNodeId objectNodeId;
    uint32_t parentObjectId = varInfo.second.objectId;
    objectNodeId.set(parentObjectId, namespaceIndex_);

    OpcUaStackServer::BaseNodeClass::SPtr parentObject = informationModel()->find(objectNodeId);
    if (parentObject.get() != nullptr) {

      /* create references to object node */
      variableNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
            ReferenceType_HasComponent, false, objectNodeId);

      parentObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
            ReferenceType_HasComponent, true, varNodeId);
    }

    variableContext ctx;
    ctx.data = constructSPtr<OpcUaDataValue>();
    ctx.data->statusCode(Success);
    ctx.data->sourceTimestamp(boost::posix_time::microsec_clock::universal_time());
    ctx.data->serverTimestamp(boost::posix_time::microsec_clock::universal_time());

    if (varInfo.second.operation == "R") {

      ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
            varInfo.second.resource->getParent()->getParent()->getName()
          , varInfo.second.desc
          , varInfo.second.type
          , (DeviceData::ACCESS_READ | DeviceData::ACCESS_OBSERVE)
          , varInfo.second.resource);

    } else if (varInfo.second.operation == "W") {

      ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
            varInfo.second.resource->getParent()->getParent()->getName()
          , varInfo.second.desc
          , varInfo.second.type
          , (DeviceData::ACCESS_READ | DeviceData::ACCESS_WRITE)
          , varInfo.second.resource);

    } else if (varInfo.second.operation == "E") {

      ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
            varInfo.second.resource->getParent()->getParent()->getName()
          , varInfo.second.desc
          , varInfo.second.type
          , DeviceData::ACCESS_NONE
          , varInfo.second.resource);

    } else if  (varInfo.second.operation == "RW") {

      ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
            varInfo.second.resource->getParent()->getParent()->getName()
          , varInfo.second.desc
          , varInfo.second.type
          , (DeviceData::ACCESS_READ | DeviceData::ACCESS_WRITE | DeviceData::ACCESS_OBSERVE)
          , varInfo.second.resource);
    }

    OpcUaNodeId dataTypeNodeId;
    if (ctx.dataObject) {

      if (varInfo.second.type == DeviceDataValue::TYPE_INTEGER) {
        ctx.data->variant()->variant(varInfo.second.value.i32);

        /* set dataType to Int32_t */
        dataTypeNodeId.set(OpcUaId_Int32, namespaceIndex_);

      } else if (varInfo.second.type == DeviceDataValue::TYPE_FLOAT) {
        ctx.data->variant()->variant(varInfo.second.value.f);

        /* set dataType to Float */
        dataTypeNodeId.set(OpcUaId_Float, namespaceIndex_);

/*---------------------------------------------------------------------------*/
/**
 * createMethodNode()
 */
bool OpcUaLWM2MLib::createMethodNode(methodMap_t& methodMap)
{
  Log (Debug, "OpcUaLWM2MLib::createMethodNode");
      } else if (varInfo.second.type == DeviceDataValue::TYPE_STRING) {

  /* create Method node */
  /* iterate through resource map and create OPC UA variable nodes */
  for (auto& methodInfo : methodMap)
  {

    OpcUaStackServer::BaseNodeClass::SPtr methodNode;
    methodNode = OpcUaStackServer::MethodNodeClass::construct();

    /* set OPC UA method node id */
    OpcUaNodeId methodNodeId;
    uint32_t methodId =  methodInfo.first;

    methodNodeId.set(methodId, namespaceIndex_);
    methodNode->setNodeId(methodNodeId);

    /* set method node attributes */
    OpcUaQualifiedName methodBrowseName(methodInfo.second.name, namespaceIndex_);
    methodNode->setBrowseName(methodBrowseName);

    OpcUaLocalizedText methodDescription("de", methodInfo.second.desc);
    methodNode->setDescription(methodDescription);

    OpcUaLocalizedText  methodDisplayName("de", methodInfo.second.name);
    methodNode->setDisplayName(methodDisplayName);

    OpcUaBoolean isExecutable = true;
    methodNode->setExecutable(isExecutable);

    OpcUaBoolean userExecutable = true;
    methodNode->setUserExecutable(userExecutable);

    /* set access permissions */
    OpcUaUInt32  writemask= 0x01;
    methodNode->setWriteMask(writemask);
    methodNode->setUserWriteMask(writemask);

    /* set node id of parent object */
    OpcUaNodeId objectNodeId;
    uint32_t parentObjectId = methodInfo.second.objectId;
    objectNodeId.set(parentObjectId, namespaceIndex_);

    OpcUaStackServer::BaseNodeClass::SPtr parentObject = informationModel()->find(objectNodeId);
    if (parentObject.get() != nullptr) {

      /* create references to object node */
      methodNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
            ReferenceType_HasComponent, false, objectNodeId);

      parentObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
            ReferenceType_HasComponent, true, methodNodeId);
    }

    /* create LWM2M device data for OPC UA variables */
     opcUaNodeContext methodCtx;
     methodCtx = createDeviceDataLWM2M(methodInfo.second, methodNode);

     /* store variable node info into variableContextMap */
     methods_.insert(std::make_pair(methodNodeId, methodCtx));

    /* add the method node to the information model */
    informationModel()->insert(methodNode);
  }

  /* clear the method Map */
  methodMap.clear();
}

/*---------------------------------------------------------------------------*/
/*
* createDeviceDataLWM2M()
*/
OpcUaLWM2MLib::opcUaNodeContext OpcUaLWM2MLib::createDeviceDataLWM2M
  (IPSOParser::ipsoResourceDescription opcUaNodeInfo , OpcUaStackServer::BaseNodeClass::SPtr opcUaNode)
{
  Log (Debug, "OpcUaLWM2MLib::createDeviceDataLWM2M");

  opcUaNodeContext ctx;
  ctx.data = constructSPtr<OpcUaDataValue>();
  ctx.data->statusCode(Success);
  ctx.data->sourceTimestamp(boost::posix_time::microsec_clock::universal_time());
  ctx.data->serverTimestamp(boost::posix_time::microsec_clock::universal_time());

  if (opcUaNodeInfo.operation == "R") {

    ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
          opcUaNodeInfo.resource->getParent()->getParent()->getName()
        , opcUaNodeInfo.desc
        , opcUaNodeInfo.type
        , (DeviceData::ACCESS_READ | DeviceData::ACCESS_OBSERVE)
        , opcUaNodeInfo.resource);

  } else if (opcUaNodeInfo.operation == "W") {

    ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
          opcUaNodeInfo.resource->getParent()->getParent()->getName()
        , opcUaNodeInfo.desc
        , opcUaNodeInfo.type
        , (DeviceData::ACCESS_READ | DeviceData::ACCESS_WRITE)
        , opcUaNodeInfo.resource);

  } else if (opcUaNodeInfo.operation == "E") {

    ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
          opcUaNodeInfo.resource->getParent()->getParent()->getName()
        , opcUaNodeInfo.desc
        , opcUaNodeInfo.type
        , DeviceData::ACCESS_NONE
        , opcUaNodeInfo.resource);

  } else if  (opcUaNodeInfo.operation == "RW") {

    ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
          opcUaNodeInfo.resource->getParent()->getParent()->getName()
        , opcUaNodeInfo.desc
        , opcUaNodeInfo.type
        , (DeviceData::ACCESS_READ | DeviceData::ACCESS_WRITE | DeviceData::ACCESS_OBSERVE)
        , opcUaNodeInfo.resource);
  }

  OpcUaNodeId dataTypeNodeId;
  if (ctx.dataObject) {

    if (opcUaNodeInfo.type == DeviceDataValue::TYPE_INTEGER) {
      ctx.data->variant()->variant(opcUaNodeInfo.value.i32);

      /* set dataType to Int32_t */
      dataTypeNodeId.set(OpcUaId_Int32, namespaceIndex_);

    } else if (opcUaNodeInfo.type == DeviceDataValue::TYPE_FLOAT) {
      ctx.data->variant()->variant(opcUaNodeInfo.value.f);

      /* set dataType to Float */
      dataTypeNodeId.set(OpcUaId_Float, namespaceIndex_);

    } else if (opcUaNodeInfo.type == DeviceDataValue::TYPE_STRING) {

      OpcUaString::SPtr str = constructSPtr<OpcUaString>();
      str->value(opcUaNodeInfo.value.cStr);
      ctx.data->variant()->variant(str);

      /* set dataType to string */
      dataTypeNodeId.set(OpcUaId_String, namespaceIndex_);
    }

    /* set dataType of Variable Node */
    opcUaNode->setDataType(dataTypeNodeId);

    /* set value of Variable node to default value */
    opcUaNode->setValue(*ctx.data);
  }

  return ctx;
  return true;
}
/*---------------------------------------------------------------------------*/
/*
* onDeviceRegister()
*/
int8_t OpcUaLWM2MLib::onDeviceRegister(const LWM2MDevice* p_dev)
{
  Log(Debug, "OpcUaLWM2MLib::onDeviceRegister");

  /* create device object */
  if (!createDeviceObjectNode(p_dev))
  {
    Log (Error, "Creation of device object failed");
  }

  LWM2MDevice* device = const_cast<LWM2MDevice*>(p_dev);

  std::vector<LWM2MObject*>::iterator objectIterator;
  for (objectIterator = device->objectStart();
      objectIterator != device->objectEnd();
      ++objectIterator)
  {
    /* check id match between LWM2M device and object dictionary objects  */
    if (!matchObjectId((*objectIterator), objectDictionary_, objectMap_)) {
      Log(Debug, "LWM2M Object ID exist in dictionary");
    }
  }

  /* create OPC UA object nodes from object map */
  if (!createObjectNode(objectMap_)) {
    Log(Debug, "Object node creation failed");
    return -1;
  }

  /* create LWM2M resources of LWM2M object instances */
  if(!createLWM2MResources(objectMap_, objectDictionary_, resourceMap_)) {
    Log(Debug, "Creation of resources failed");
    return -1;
  }

  /* create OPC UA variable nodes from resource map */
  if (!createVariableNode(resourceMap_)) {
    Log(Debug, "Creation of variable node failed");
    return -1;
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
/*
* onDeviceUnregister()
*/
int8_t OpcUaLWM2MLib::onDeviceDeregister(const LWM2MDevice* p_dev)
{
  Log(Debug, "OpcUaLWM2MLib::onDeviceUnregister");

  LWM2MDevice* device = const_cast<LWM2MDevice*>(p_dev);

  std::vector< LWM2MResource* >::iterator resourceIterator;
  std::vector<LWM2MObject*>::iterator objectIterator;

  /* iterate over the objects of this device */
  for (objectIterator = device->objectStart();
      objectIterator != device->objectEnd();
      ++objectIterator)
  {
    /* iterate over the resources of this object */
    for (resourceIterator = (*objectIterator)->resourceStart();
        resourceIterator != (*objectIterator)->resourceEnd();
        ++resourceIterator)
    {
      /* unregister all callbacks */
      unregisterCallbacks();

      OpcUaNodeId variableNodeId;
      int16_t resourceId = (*resourceIterator)->getResId();
      variableNodeId.set(resourceId, namespaceIndex_);

      /* clear variable context */
      variables_.clear();

      if (variables_.empty()) {
        OpcUaNodeId objectNodeId;
        int16_t objectId = (*resourceIterator)->getParent()->getObjId();
        objectNodeId.set(objectId, namespaceIndex_);
        informationModel()->remove(objectNodeId);
      }
    }
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
bool OpcUaLWM2MLib::matchObjectId(LWM2MObject* lwm2mObj, objectDictionary_t& dictionary
    , objectMap_t& objectMap)
{
  Log(Debug, "OpcUaLWM2MLib::matchObjectId");

  bool ret = false;
  /* iterate object dictionary and loop up for Object id matches */
  for (auto& dict : dictionary)
  {
    if (lwm2mObj->getObjId() == dict.second->objectDesc.id) {

      /* store object info of connected LWM2M device */
      IPSOParser::ipsoObjectDescription objectInfo = {};
      objectInfo.id = lwm2mObj->getObjId();
      objectInfo.instanceId = lwm2mObj->getInstId();
      objectInfo.object = lwm2mObj;
      objectInfo.name = dict.second->objectDesc.name;
      objectInfo.desc = dict.second->objectDesc.desc;
      objectInfo.type = dict.second->objectDesc.type;

      /* unique id for object instances  */
      uint32_t id = offset();

      /* store info of matching Object in map */
      objectMap.insert(objectMap_t::value_type(id, objectInfo));

      ret = true;
    } else {
     continue;
    }
  }
    return ret;
}

/*---------------------------------------------------------------------------*/
/*
* createLWM2MResources()
*/
bool OpcUaLWM2MLib::createLWM2MResources(objectMap_t& objectMap
    , objectDictionary_t& dictionary
    , resourceMap_t& resourceMap)
{
  Log(Debug, "OpcUaLWM2MLib::createLWM2MResources");

  for (auto& objectItem : objectMap)
  {
    for (auto& dictEntry : dictionary)
    {
      for (auto& resourceItem : dictEntry.second->resourceDesc)
      {
        /* create new resource per instance */
        IPSOParser::ipsoResourceDescription resourceInfo = {};

        /* copy resource attributes from object dictionary */
        resourceInfo.name = resourceItem.name;
        resourceInfo.desc = resourceItem.desc;
        resourceInfo.type = resourceItem.type;
        resourceInfo.resourceId = resourceItem.resourceId;
        resourceInfo.value = resourceItem.value;
        resourceInfo.operation = resourceItem.operation;

        /* copy node id of parent object */
        resourceInfo.objectId = objectItem.first;

        /* create new resource tuple */
        resourceId_t resourceId(objectItem.second.object->getParent()->getName()
              , objectItem.second.id
              , objectItem.second.instanceId
              , resourceItem.resourceId);

      if (objectItem.second.id == dictEntry.second->id) {
        if (resourceItem.operation == "R") {

          /* create resource for READ access */
          LWM2MResource* resource =
                new  LWM2MResource(resourceItem.resourceId, true, false, false);

          /* add resource to parent object */
          objectItem.second.object->addResource(resource);

          /* copy created resource */
          resourceInfo.resource = resource;

          /* add resource info to resource map */
          resourceMap.insert(resourceMap_t::value_type(resourceId, resourceInfo));

          } else if (resourceItem.operation == "W") {

            /* create resource for WRITE access */
            LWM2MResource* resource =
                new LWM2MResource(resourceItem.resourceId, false, true, false);

            /* add resource to parent object */
            objectItem.second.object->addResource(resource);

            /* copy created resource */
            resourceInfo.resource = resource;

            /* add resource info to map */
            resourceMap.insert(resourceMap_t::value_type(resourceId, resourceInfo));

          } else if (resourceItem.operation == "E") {

            /* create resource for EXEC access */
            LWM2MResource* resource =
                new LWM2MResource(resourceInfo.resourceId, false, false, true);

            /* add resource to parent object */
            objectItem.second.object->addResource(resource);

            /* copy resource attributes */
            resourceInfo.resource = resource;

            /* add resource info to map */
            resourceMap.insert(resourceMap_t::value_type(resourceId, resourceInfo));

          } else if  (resourceItem.operation == "RW") {

            /* create resource for READ and WRITE access */
            LWM2MResource* resource =
                new LWM2MResource(resourceItem.resourceId, true, true, false);

            /* add resource to parent object */
            objectItem.second.object->addResource(resource);

            /* copy resource attributes */
            resourceInfo.resource = resource;

            /* add resource info to map */
            resourceMap.insert(resourceMap_t::value_type(resourceId, resourceInfo));
          }
        }
      }
    }
  }

  /* clear object map */
  objectMap.clear();

  return true;
}

} /* namespace OpcUalwm2m */

  extern "C" DLLEXPORT void init(OpcUaStackServer::ApplicationIf** applicationIf) {
  *applicationIf = new OpcUaLWM2M::OpcUaLWM2MLib();
}

