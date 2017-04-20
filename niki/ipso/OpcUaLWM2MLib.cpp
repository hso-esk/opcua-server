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

//uint32_t OpcUaLWM2MLib::ID_ = 6800;

/**
 * OpcUaLWM2MLib()
 */
OpcUaLWM2MLib::OpcUaLWM2MLib(void)
  : OpcUaStackServer::ApplicationIf()
  , ipsofileName_("")
  , namespaceIndex_(0)
  , opcUalwm2mObs(*this)
  , readSensorValueCallback_(boost::bind(&OpcUaLWM2MLib::readSensorValue, this, _1))
  , writeSensorValueCallback_(boost::bind(&OpcUaLWM2MLib::writeSensorValue, this, _1))
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
  lwm2mServer_->registerObserver(&opcUalwm2mObs);

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
  lwm2mServer_->deregisterObserver(&opcUalwm2mObs);

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
    return;
  }

  if (!it->second.dataObject)  {
	applicationReadContext->statusCode_ = BadInternalError;
	return;
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
    return;
  }

  if (!it->second.dataObject)  {
	applicationWriteContext->statusCode_ = BadInternalError;
	return;
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
    if (!objectDictionary.insert(objectDictionary_t::value_type(
    		desc.objectDesc.id, &desc)).second) {
      Log(Error, "Object dictionary creation from IPSO descriptions failed");
      return false;
    }
  }
  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createObjectNode()
 */
bool OpcUaLWM2MLib::createObjectNode(IPSOParser::ipsoObjectDescription const& objectInfo)
{
  Log(Debug, "OpcUaLWM2MLib::createObjectNode");

  /* set node id of object */
    OpcUaNodeId objectNodeId;
    objectNodeId.set(objectInfo.id, namespaceIndex_);


  OpcUaStackServer::BaseNodeClass::SPtr objectNode = informationModel()->find(objectNodeId);
  if (objectNode.get() == nullptr) {

  OpcUaStackServer::BaseNodeClass::SPtr objectNode;
  objectNode = OpcUaStackServer::ObjectNodeClass::construct();
  objectNode->setNodeId(objectNodeId);

  /* set object node attributes */
  OpcUaQualifiedName browseName(objectInfo.name, namespaceIndex_);
  objectNode->setBrowseName(browseName);
  OpcUaLocalizedText description("de", objectInfo.desc);
  objectNode->setDescription(description);
  OpcUaLocalizedText displayName("de", objectInfo.name);
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
  deviceObjectNodeId.set(device->getID(), namespaceIndex_);
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

   //rootNode_ = objectNode;
   return true;
}
/*---------------------------------------------------------------------------*/

namespace
{
/**
 * offset ()
 */
static uint32_t offset()
{
	static uint32_t ID = 6800;
	return ID++;
}

} /* anonymous namespace */

/*---------------------------------------------------------------------------*/
/**
 * createVariableNode()
 */
bool OpcUaLWM2MLib::createVariableNode (IPSOParser::ipsoResourceDescription& varInfo)
{
	Log(Debug, "OpcUaLWM2MLib::createVariableNode");

	OpcUaStackServer::BaseNodeClass::SPtr variableNode;
	variableNode = OpcUaStackServer::VariableNodeClass::construct();

	/* set OPC UA variable node id */
	OpcUaNodeId varNodeId;
	uint32_t resourceId = varInfo.resourceId + offset();
	varNodeId.set(resourceId, namespaceIndex_);
	variableNode->setNodeId(varNodeId);

	/* set variable node attributes */
	OpcUaQualifiedName varbrowseName(varInfo.name, namespaceIndex_);
	variableNode->setBrowseName(varbrowseName);

	OpcUaLocalizedText vardescription("de", varInfo.desc);
	variableNode->setDescription(vardescription);

	std::string varDisplayName = varInfo.name + std::string("<") +
	    boost::lexical_cast<std::string>(varInfo.objectInstanceId) + std::string(">");
	OpcUaLocalizedText  vardisplayName("de", varDisplayName);
	variableNode->setDisplayName(vardisplayName);

	//OpcUaByte accessLevel(0x05);
	OpcUaByte accessLevel(0x4F);
	variableNode->setAccessLevel(accessLevel);

	//OpcUaByte userAccessLevel(0x05);
	OpcUaByte userAccessLevel(0x4F);
	variableNode->setUserAccessLevel(userAccessLevel);

	OpcUaDouble minimumSamplingInterval = 1;
	variableNode->setMinimumSamplingInterval(minimumSamplingInterval);

	/* Set true to allow server to collect historical data */
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

	/* set OPC UA object node id */
	OpcUaNodeId objectNodeId;
	//objectNodeId.set(varInfo.resource->getParent()->getObjId(), namespaceIndex_);
	objectNodeId.set(varInfo.objectId, namespaceIndex_);

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

	if (varInfo.operation == "R") {

	  ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
			  varInfo.deviceName
			, varInfo.desc
			, varInfo.type
			, (DeviceData::ACCESS_READ | DeviceData::ACCESS_OBSERVE)
			, varInfo.resource);

	} else if (varInfo.operation == "W") {

	  ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
			varInfo.deviceName
			, varInfo.desc
			, varInfo.type
			, (DeviceData::ACCESS_READ | DeviceData::ACCESS_WRITE)
			, varInfo.resource);

	} else if (varInfo.operation == "E") {

	  ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
			  varInfo.deviceName
			, varInfo.desc
			, varInfo.type
			, DeviceData::ACCESS_NONE
			, varInfo.resource);

	} else if  (varInfo.operation == "R/W") {

	  ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
			varInfo.deviceName
			, varInfo.desc
			, varInfo.type
			, (DeviceData::ACCESS_READ | DeviceData::ACCESS_WRITE | DeviceData::ACCESS_OBSERVE)
			, varInfo.resource);

	}

	OpcUaNodeId dataTypeNodeId;
	if (ctx.dataObject) {

	  if (varInfo.type == DeviceDataValue::TYPE_INTEGER) {

        ctx.data->variant()->variant(varInfo.value.i32);

		/* set dataType to Int32_t */
		dataTypeNodeId.set(OpcUaId_Int32, namespaceIndex_);

	   } else if (varInfo.type == DeviceDataValue::TYPE_FLOAT) {
		 ctx.data->variant()->variant(varInfo.value.f);

		 /* set dataType to Float */
		 dataTypeNodeId.set(OpcUaId_Float, namespaceIndex_);


	   } else if (varInfo.type == DeviceDataValue::TYPE_STRING) {

		 OpcUaString::SPtr str = constructSPtr<OpcUaString>();
		 str->value(varInfo.value.cStr);
		 ctx.data->variant()->variant(str);

		 /* set dataType to string */
		 dataTypeNodeId.set(OpcUaId_String, namespaceIndex_);
	   }

	   /* set dataType of Variable Node */
	   variableNode->setDataType(dataTypeNodeId);

	   /* set value of Variable node to default value */
	   variableNode->setValue(*ctx.data);

	   /* add variable node to OPC UA server information model */
	   informationModel()->insert(variableNode);

	   /* store variable node info into variableContextMap */
	   variables_.insert(std::make_pair(varNodeId, ctx));

	   /* register callback for read and write access permissions */
	   if (varInfo.operation == "R" || varInfo.operation == "W" || varInfo.operation == "R/W") {
	     if (!registerCallbacks(resourceId)) {
		   Log(Error, "Register callback failed");
	    }
      }
    }
   return true;
}
/*---------------------------------------------------------------------------*/
/*
* onDeviceRegister()
*/
int8_t OpcUaLWM2MLib::onDeviceRegister(const LWM2MDevice* p_dev)
{
  Log(Debug, "OpcUaLWM2MLib::onDeviceRegister");

  /* get iterator to the device map structure */
  std::map<std::string, LWM2MDevice*>::iterator deviceIterator;
  deviceIterator = p_dev->getServer()->deviceStart();

  /* get device ID */
  deviceId_ = p_dev->getID();

  /* create device object */
  if (!createDeviceObjectNode(p_dev))
  {
    Log (Error, "Creation of device object failed");
  }

  std::vector<LWM2MObject*>::iterator objectIterator;
  for (objectIterator = deviceIterator->second->objectStart();
	   objectIterator != deviceIterator->second->objectEnd();
	   ++objectIterator)
  {

    /* check corresponding object ids in dictionary */
	if (hasObjectId((*objectIterator)->getObjId(), objectDictionary)) {
	   Log(Debug, "LWM2M Object ID exist in dictionary");

	   /* store matching Object Ids */
	   objectIdVec_.push_back((*objectIterator)->getObjId());

	   /* add object with their instances to vector */
	   objectVec_.push_back((*objectIterator));

	   std::cout << "ObjectIdVec size is: " << objectIdVec_.size() << std::endl;
	   std::cout << "ObjectVec_ size is: " << objectVec_.size() << std::endl;

	   /* create OPC UA object node */
	   for (auto& entry : objectDictionary)
	   {
	     if (std::find(objectIdVec_.begin(), objectIdVec_.end(), entry.second->objectDesc.id) != objectIdVec_.end()) {
           if (!createObjectNode(entry.second->objectDesc)) {
               Log(Debug, "Object node creation failed");
           }
	     }
	   }
	 } else {
	   continue;
	 }
  }

  /* create LWM2M resources */
  for (auto& dictItem : objectDictionary)
  {
    if (std::find(objectIdVec_.begin(), objectIdVec_.end(), dictItem.second->objectDesc.id) != objectIdVec_.end()) {
      if(!createLWM2MResources(objectVec_, dictItem.second->resourceDesc, resourceMap_))
      {
        Log(Debug, "Creation of resources failed");
        return -1;
      }
    }
   }

  /* create OPC UA variables from resource information */
  for (auto& resource : resourceMap_)
  {
	  if (!createVariableNode(resource.second))
	    {
	      Log(Debug, "Creation of variable node B failed");
	      return -1;
	    }
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

  /* get iterator to the device map structure */
  std::map<std::string, LWM2MDevice*>::iterator deviceIterator;
  deviceIterator = p_dev->getServer()->deviceStart();

  std::vector< LWM2MResource* >::iterator resourceIterator;
  std::vector<LWM2MObject*>::iterator objectIterator;
  variableContextMap::iterator iter;

  /* iterate over the objects of this device */
  for (objectIterator = deviceIterator->second->objectStart();
	   objectIterator != deviceIterator->second->objectEnd();
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
/*
* hasObjectId()
*/
bool OpcUaLWM2MLib::hasObjectId(int16_t id, objectDictionary_t dictionary)
{
  Log(Debug, "OpcUaLWM2MLib::hasObjectId");

  bool ret = false;
  /* iterate object dictionary and loop up for Object id matches */
  for (auto& dict : dictionary)
  {
    if (id == dict.second->objectDesc.id){
      ret = true;
    } else {
     continue;
    }
  }
  return ret;
}

/*---------------------------------------------------------------------------*/
/*
* createLWM2MResourcesB()
*/
bool OpcUaLWM2MLib::createLWM2MResources(objectVec_t& objVec
		, std::vector<IPSOParser::ipsoResourceDescription>& ipsoResource
		, resourceMap_t& resourceMap)
{
  Log(Debug, "OpcUaLWM2MLib::createLWM2MResources");

  for (auto& objitem : objVec)
  {
    for (auto& resItem : ipsoResource)
    {

      /* create new resource per instance */
      IPSOParser::ipsoResourceDescription res = {};
      res.deviceName = objitem->getParent()->getName();
      res.objectId =  objitem->getObjId();
      res.objectInstanceId =  objitem->getInstId();

      res.name = resItem.name;
      res.desc = resItem.desc;
      res.type = resItem.type;
      res.resourceId = resItem.resourceId;
      res.value = resItem.value;

      /* create new resource tuple */
      resourceId_t resourceId(res.deviceName
            , res.objectId
            , res.objectInstanceId
            , res.resourceId);

      if (resItem.operation == "R") {

        /* create resource for READ access */
        LWM2MResource* resource =
          new  LWM2MResource(resItem.resourceId, true, false, false);

        /* add resource to parent object */
        objitem->addResource(resource);

        /* copy resource attributes */
        res.resource = resource;
        res.operation = resItem.operation;

        /* add resource info to map */
        resourceMap.insert(resourceMap_t::value_type(resourceId, res));

      } else if (resItem.operation == "W") {

        /* create resource for WRITE access */
        LWM2MResource* resource =
          new LWM2MResource(resItem.resourceId, false, true, false);

        /* add resource to object */
        objitem->addResource(resource);

        /* store created resource */
        res.resource = resource;
        res.operation = resItem.operation;

        /* add resource info to map */
        resourceMap.insert(resourceMap_t::value_type(resourceId, res));

      } else if (resItem.operation == "E") {

        /* create resource for EXEC access */
        LWM2MResource* resource =
          new LWM2MResource(resItem.resourceId, false, false, true);

        /* add resource to parent object */
        objitem->addResource(resource);

        /* copy resource attributes */
        res.resource = resource;
        res.operation = resItem.operation;

        /* add resource info to map */
        resourceMap.insert(resourceMap_t::value_type(resourceId, res));

      } else if  (resItem.operation == "R/W") {

      /* create resource for READ and WRITE access */
      LWM2MResource* resource =
          new LWM2MResource(resItem.resourceId, true, true, false);

      /* add resource to parent object */
      objitem->addResource(resource);

      /* copy resource attributes */
      res.resource = resource;
      res.operation = resItem.operation;

      /* add resource info to map */
      resourceMap.insert(resourceMap_t::value_type(resourceId, res));
      }
    }
  }

  return true;
}

} /* namespace OpcUalwm2m */

  extern "C" DLLEXPORT void init(OpcUaStackServer::ApplicationIf** applicationIf) {
  *applicationIf = new OpcUaLWM2M::OpcUaLWM2MLib();
}

