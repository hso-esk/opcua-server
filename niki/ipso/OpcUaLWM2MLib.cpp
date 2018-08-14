/*
 * --- License -------------------------------------------------------------- *
 */

/*
 * Copyright 2017 NIKI 4.0 project team
 *
 * NIKI 4.0 was financed by the Baden-Württemberg Stiftung gGmbH (www.bwstiftung.de).
 * Project partners are FZI Forschungszentrum Informatik am Karlsruher
 * Institut für TechnoLoger::logie (www.fzi.de), Hahn-Schickard-Gesellschaft
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


#include "OpcUaLWM2MLib.h"
#include "Logger.h"
#include "OpcUaStackCore/Utility/Environment.h"
#include "OpcUaStackServer/ServiceSetApplication/ApplicationService.h"
#include "OpcUaStackServer/AddressSpaceModel/BaseNodeClass.h"
#include "OpcUaStackServer/AddressSpaceModel/ObjectNodeClass.h"
#include "OpcUaStackServer/AddressSpaceModel/VariableNodeClass.h"
#include "OpcUaStackServer/AddressSpaceModel/MethodNodeClass.h"
#include "OpcUaStackCore/Base/ConfigXml.h"
#include "OpcUaStackCore/Base/ConfigXmlManager.h"
#include "LWM2MDevice.h"
#include "LWM2MResource.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/format.hpp>
#include <boost/functional/hash.hpp>
#include <iostream>
#include <algorithm>
#include <string>
#include <csignal>

void signalHandler(int signum)
{
  std::cout << "Application will close..., received external interrupt"
            << std::endl;

  exit (signum);
}

namespace OpcUaLWM2M
{

/** Sleeptime in us within the Thread */
#define OPCUALWM2MLIB_THREAD_TOT_US       5000
#define ID_MODULO_DIVISOR            100000000

/**
 * OpcUaLWM2MLib()
 */
OpcUaLWM2MLib::OpcUaLWM2MLib(void)
  : OpcUaStackServer::ApplicationIf()
  , threadRun(false)
  , ipsofileName_("")
  , namespaceIndex_(0)
  , deviceId_(0)
  , resourceMap_()
  , objectMap_()
  , resourceMaps_()
  , objectMaps_()
  , dbServer_()
  , readSensorValueCallback_(boost::bind(&OpcUaLWM2MLib::readSensorValue, this, _1))
  , readHistorySensorValueCb_(boost::bind(&OpcUaLWM2MLib::readHistorySensorValue, this, _1))
  , writeSensorValueCallback_(boost::bind(&OpcUaLWM2MLib::writeSensorValue, this, _1))
  , execSensorMethodCallback_(boost::bind(&OpcUaLWM2MLib::execSensorMethod, this, _1))
  , mp_op( NULL )
{

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

  /* initialize the mutex */
  pthread_mutex_init( &m_mutex, &attr );

  Logger::log(Debug, "OpcUaLWM2MLib::OpcUaLWM2MLib");
}

/*---------------------------------------------------------------------------*/
/**
 * ~OpcUaLWM2MLib()
 */
OpcUaLWM2MLib::~OpcUaLWM2MLib(void)
{
	Logger::log(Debug, "OpcUaLWM2MLib::~OpcUaLWM2MLib");
}

/*---------------------------------------------------------------------------*/
bool OpcUaLWM2MLib::isObserved = false;

/*---------------------------------------------------------------------------*/
/**
 * notify()
 */
int8_t OpcUaLWM2MLib::notify( s_lwm2m_serverobserver_event_param_t param,
    const e_lwm2m_serverobserver_event_t ev)
{
  Logger::log(Trace, "OpcUaLWM2MLib::notify Notification for event <> received", ev);

  s_devEvent_t event = {param, ev};
  pthread_mutex_lock(&m_mutex);

  std::deque<s_devEvent_t>::iterator it = evQueue.begin();
  while(it != evQueue.end())
  {
    if( (it->event == ev) &&
        (strcmp(it->param.devName, param.devName) == 0) )

      Logger::log(Trace,"OpcUaLWM2MLib::notify Device <> <>", param.devName, ev);

      break;
    it++;
  }

  if( it == evQueue.end() )
    evQueue.push_back( event );
  pthread_mutex_unlock(&m_mutex);

  return 0;
}


/*---------------------------------------------------------------------------*/
/**
 * notify()
 */
int8_t OpcUaLWM2MLib::notify( const DeviceDataValue* p_val,
    const DeviceData* p_data, void* p_param )
{
   const DeviceDataLWM2M* p_lwm2mData = static_cast<const DeviceDataLWM2M*>(p_data);

  LWM2MResource* p_res = p_lwm2mData->getResource();

  Logger::log(Trace, "OpcUaLWM2MLib::notify Notification from received.");

  for (auto& item : variables_)
  {
    if( item.second.resInfo.resource == p_res )
    {
      OpcUaDataValue::SPtr value;
     item.second.data = createDataValue(p_val);
     OpcUaNodeId nodeId = item.first;

     /* update value of observed node */
     OpcUaStackServer::BaseNodeClass::SPtr observedNode;
     observedNode = informationModel()->find(nodeId);
     if( observedNode != NULL )
     {
       observedNode->setValue(*item.second.data);
       /* store sensor value in database */
       dbServer_.writeDataToDatabase(dbModelConfig_.databaseConfig().databaseTableName()
                 , nodeId, *item.second.data);
     }
    }
  }
  return 0;
}


/*---------------------------------------------------------------------------*/
/**
 * startup()
 */
bool OpcUaLWM2MLib::startup(void)
{
  Logger::log(Debug, "OpcUaLWM2MLib::startup");

  signal(SIGINT, signalHandler);

  /* load config file */
  if (!loadServerConfig()) {
    Logger::log(Error, "Could not load config file");
    return false;
  }

  /* parse the IPSO file */
  for (auto& ipsofileName : ipsofileNameVec_)
  {
    if (!ipsoParser_.parseIPSOfile(ipsofileName, data_))
    {
    	Logger::log(Error, "Parsing of IPSO file failed");
	return false;
    }
  }

  /* create object dictionary */
  if (!createObjectDictionary(data_))
  {
    Logger::log(Error, "Creation of Object dictionary failed");
    return false;
  }

  /* start the the LWM2M server */
  LWM2MServer::instance()->startServer();
  Logger::log (Debug, "LWM2M server started");

  /* create thread */
  threadRun = true;
  t = new boost::thread( &OpcUaLWM2MLib::thread, this );

  /* register the OPC UA server observer object */
  LWM2MServer::instance()->registerObserver( this );

  /* start the database server */
  dbServer_.DBServer().applicationServiceIf(&service());
  dbServer_.dbModelConfig(&dbModelConfig_);

  if (!dbServer_.startup()) {
    Logger::log (Error, "Database server startup failed");
    return false;
  }

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
  Logger::log(Debug, "OpcUaLWM2MLib::shutdown");

  /* deregister OpcUa LWM2M server observer */
  LWM2MServer::instance()->deregisterObserver( this );

  /* stop worker thread */
  threadRun = false;
  t->join();
  delete t;

  /* stop the LWM2M server */
  LWM2MServer::instance()->stopServer();

  /* shut down database server */
  if (!dbServer_.shutdown()) {
    Logger::log (Error, "Database server shutdown failed");
    return false;
  }

  return true;
}


/*---------------------------------------------------------------------------*/
/**
 * thread()
 */
void OpcUaLWM2MLib::thread( void )
{
  while( threadRun == true )
  {
    usleep(OPCUALWM2MLIB_THREAD_TOT_US);

    pthread_mutex_lock(&m_mutex);
    LWM2MServer::instance()->runServer();

    /* Ceck for events */
    s_devEvent_t ev;
    while( !evQueue.empty())
    {
      ev = evQueue.front();
      evQueue.pop_front();

      switch (ev.event)
      {
        case e_lwm2m_serverobserver_event_register:
        {
          Logger::log(Trace, "Device registration triggered, for <> device", ev.param.devName);

          /* execute onDeviceRegister function */
          onDeviceRegister(ev.param.devName);
        }
        break;

        case e_lwm2m_serverobserver_event_update:
        {
           Logger::log(Trace, "Device <> triggered an update.",ev.param.devName);
          /** TODO */
          Logger::log(Warning, "Update event not implemented yet");
        }
        break;

        case e_lwm2m_serverobserver_event_deregister:
        {
          Logger::log(Trace, "Device <> triggered a deregistration event", ev.param.devName);

          /* execute onDeviceRegister function */
          onDeviceDeregister(ev.param.devName);
        }
        break;
      }
    }
    pthread_mutex_unlock(&m_mutex);

    pthread_mutex_lock(&m_mutex);
    /* Check for operations */
    if( mp_op != NULL )
    {
      if( mp_op->getState() == OpcUaOp::opState::opStateIdle )
      {
        /* Start the operation depending on its type */
        switch(mp_op->getType() )
        {
          case  OpcUaOp::opType::opTypeRead:
            mp_op->setState( OpcUaOp::opState::opStateProc );
            /* call the according read function */
            readSensorValueLocal( (OpcUaStackCore::ApplicationReadContext *)mp_op->getParam() );
            mp_op->setState( OpcUaOp::opState::opStateFin );
            break;

          default:
            /* invalid */
            mp_op->setState( OpcUaOp::opState::opStateError );
        }
      }
    }
    pthread_mutex_unlock(&m_mutex);
  }
}


/*---------------------------------------------------------------------------*/
/**
 * loadServerConfig()
 */
bool OpcUaLWM2MLib::loadServerConfig(void)
{
  Logger::log(Debug, "OpcUaLWM2M::loadServerConfig");

  /* load OPC UA server config file */
  Config::SPtr config;
  ConfigXmlManager configXmlManager;
  std::string loggerConfigValue;
  if (!configXmlManager.registerConfiguration(applicationInfo()->configFileName(), config)) {
    Logger::log(Error, "read OPC UA server configuration error");
    return false;
  }
    /* read logging level */
  if (!config->getConfigParameter("OpcUaServerModel.LoggerConfig.LoggingLevel",loggerConfigValue)){
	  Logger::log(Warning, "No Logger value set in config file: <>, using default value: ", applicationInfo()->configFileName());
  }
  else {
	  Logger::setLoggerDisplayLevel(Logger::getLogLevel(loggerConfigValue));
	  Logger::log(Trace, "Logging value has been set to : <>", Logger::getLogLevel());
  }

  /*  load and decode IPSO config file */
  std::string ipsoConfigfile;
  if (!config->getConfigParameter("OpcUaServerModel.IPSOConfig.IPSOConfigPath", ipsoConfigfile)) {
    Logger::log (Error, "read IPSO configuration error");
    return false;
  }
  if (!decodeIPSOConfig(ipsoConfigfile)) {
    Logger::log (Error, "decode IPSO configuration error");
    return false;
  }

  /*  load and decode database config file */
  std::string dbConfigfile;
  if (!config->getConfigParameter("OpcUaServerModel.Database.DatabaseConfig",dbConfigfile)) {
    Logger::log (Error, "read database configuration error");
    return false;
  }
  if (!decodeDbConfig(dbConfigfile)) {
    Logger::log (Error, "decode database configuration error");
    return false;
  }

  return true;
}
/*---------------------------------------------------------------------------*/
/**
 * decodeIPSOConfig()
 */
bool OpcUaLWM2MLib::decodeIPSOConfig(const std::string& configFileName)
{
  Logger::log(Debug, "OpcUaLWM2MLib::decodeIPSOConfig");

  Config::SPtr config;
  ConfigXmlManager configXmlManager;

  /* read configuration file */
  if (!configXmlManager.registerConfiguration(configFileName, config)) {
    Logger::log (Error, "Error reading IPSO config, with message <>, for file <>.", configXmlManager.errorMessage(),configFileName);
    return false;
  }

  /* process and store IPSO file paths */
  std::vector<Config> ipsoConfigVec;
  config->getChilds("IPSOModel.IPSOPath", ipsoConfigVec);

  if (ipsoConfigVec.size() == 0) {
    Logger::log(Error, "IPSO XML file does not exist");
    return false;
  }

  for (auto& ipsoConfig : ipsoConfigVec)
  {
    /* read configuration parameter */
    boost::optional<std::string> ipsoFileName = ipsoConfig.getValue("<xmlattr>.File");
    if (!ipsoFileName) {
    	Logger::log(Error, "eddl path does not exist in configuration file <>, for <> variable",applicationInfo()->configFileName(),"IPSOModel.IPSOPath.<xmlattr>.File");
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
 * decodeDbConfig()
 */
bool OpcUaLWM2MLib::decodeDbConfig(const std::string& configFileName)
{
  Logger::log (Debug, "OpcUaLWM2MLib::loadDbConfig");

  Config::SPtr config;
  ConfigXmlManager configXmlManager;

  /* read database configuration file */
  if (!configXmlManager.registerConfiguration(configFileName, config)) {
    Logger::log (Error, "Error reading Database config. <> for file <>", configXmlManager.errorMessage(), configFileName);
    return false;
  }

  /* decode Database configuration */
  boost::optional<Config> child = config->getChild("DBModel");
  if (!child) {
    Logger::log (Error, "Element missing in config file <>", config->configFileName());
    return false;
  }

  if (!dbModelConfig_.decode(*child)) {
    Logger::log (Error, "Error occurred during database configuration decoding");
    return false;
  }

  return true;

}

/*---------------------------------------------------------------------------*/
/**
 * readSensorValuereadSensorValue()
 */
void OpcUaLWM2MLib::readSensorValue (ApplicationReadContext* applicationReadContext)
{
	Logger::log (Debug, "OpcUaLWM2MLib::readSensorValue");
  OpcUaOp::opState state;
  pthread_mutex_lock( &m_mutex );

  if( mp_op != NULL )
    delete( mp_op );

  /* create a new operation */
  mp_op = new OpcUaOp( OpcUaOp::opType::opTypeRead,
      applicationReadContext );
  state = mp_op->getState();

  pthread_mutex_unlock( &m_mutex );

  /* wait until the operation was finished */
  while( (state == OpcUaOp::opState::opStateIdle) ||
      (state == OpcUaOp::opState::opStateProc) )
  {
    pthread_mutex_lock( &m_mutex );
    state = mp_op->getState();

    if( state == OpcUaOp::opState::opStateError )
    {
      /* An error occurred set the according return value*/
      applicationReadContext->statusCode_ = BadInternalError;
    }
    pthread_mutex_unlock( &m_mutex );
  }
}

/*---------------------------------------------------------------------------*/
/**
 * readSensorValuereadSensorValueLocal()
 */
void OpcUaLWM2MLib::readSensorValueLocal (ApplicationReadContext* applicationReadContext)
{
   Logger::log (Trace, "OpcUaLWM2MLib::readSensorValue called for node <>", applicationReadContext->nodeId_);

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

  if (p_val && it->second.data) {

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

    /* store sensor value in database */
    dbServer_.writeDataToDatabase(dbModelConfig_.databaseConfig().databaseTableName()
      , applicationReadContext->nodeId_
      , *it->second.data );

    /* copy updated value to application read context */
    applicationReadContext->statusCode_ = Success;
    it->second.data->copyTo(applicationReadContext->dataValue_);

  } else if (it->second.resInfo.mandatoryType == "Mandatory") {
    std::cout << "Read resource failed, Resource is not accessible"
              << std::endl;
    applicationReadContext->statusCode_ = BadCommunicationError;

  } else if (it->second.resInfo.mandatoryType == "Optional") {
    std::cout << "Read resource failed, resource is not available"
              << std::endl;
    applicationReadContext->statusCode_ = BadCommunicationError;
  }
}

/*---------------------------------------------------------------------------*/
/**
 * readSensorHistoryValue()
 */
void OpcUaLWM2MLib::readHistorySensorValue (ApplicationHReadContext* applicationHReadContext)
{
  Logger::log  (Debug, "OpcUaLWM2MLib::readSensorHistoryValue");

  OpcUaDateTime startTime (applicationHReadContext->startTime_);
  OpcUaDateTime stopTime (applicationHReadContext->stopTime_);

  /* read history sensor data from database */
  OpcUaDataValue::Vec dataValues;
  dbServer_.readDataFromDatabase(dbModelConfig_.databaseConfig().databaseTableName()
      , applicationHReadContext->nodeId_
      , startTime
      , stopTime
      , dataValues);

   /* create result array */
   applicationHReadContext->dataValueArray_ = constructSPtr<OpcUaDataValueArray>();
   applicationHReadContext->dataValueArray_->resize(dataValues.size());
   for (uint32_t idx = 0; idx < dataValues.size(); idx++) {
     applicationHReadContext->dataValueArray_->set(idx, dataValues[idx]);
   }

  /* update read context status code */
  applicationHReadContext->statusCode_ = Success;
}

/*---------------------------------------------------------------------------*/
/**
 * writeSensorValue()
 */
void OpcUaLWM2MLib::writeSensorValue (ApplicationWriteContext* applicationWriteContext)
{
  Logger::log (Trace, "OpcUaLWM2MLib::writeSensorValue called for node <>",applicationWriteContext->nodeId_);

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

  Logger::log(Trace, "Logging <> as data.",it->second.data);

  /* check if resource is available */
  if (it->second.dataObject->getVal()) {

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
  } else if (it->second.resInfo.mandatoryType == "Mandatory") {
	  Logger::log(Error, "Resource write failed, resources is inaccessible.");
    std::cout << "Write resource failed, Resource is not accessible"
              << std::endl;

  } else if (it->second.resInfo.mandatoryType == "Optional") {
	  Logger::log(Error, "Resource write failed, resources is unavailabe.");
    std::cout << "Write resource failed, resource is not available"
              << std::endl;
  }
}

/*---------------------------------------------------------------------------*/
/**
 * registerResourceCallback()
 */
void OpcUaLWM2MLib::execSensorMethod(ApplicationReadContext* applicationReadContext)
{

  /** TODO  */
}

/*---------------------------------------------------------------------------*/
/**
 * registerResourceCallback()
 */
bool OpcUaLWM2MLib::registerCallbacks(OpcUaUInt32 id)
{
  Logger::log(Debug, "OpcUaLWM2MLib::registerCallbacks");

  OpcUaNodeId::SPtr nodeId = constructSPtr<OpcUaNodeId>();
  nodeId->set(id, namespaceIndex_);

  ServiceTransactionRegisterForwardNode::SPtr trx
       = constructSPtr<ServiceTransactionRegisterForwardNode>();
  RegisterForwardNodeRequest::SPtr  req = trx->request();
  RegisterForwardNodeResponse::SPtr res = trx->response();

  req->forwardNodeSync()->readService().setCallback(readSensorValueCallback_);
  req->forwardNodeSync()->readHService().setCallback(readHistorySensorValueCb_);
  req->forwardNodeSync()->writeService().setCallback(writeSensorValueCallback_);
  req->nodesToRegister()->resize(1);
  req->nodesToRegister()->set(0, nodeId);

  service().sendSync(trx);
  if (trx->statusCode() != Success) {
    Logger::log(Error, "Status code response error");
    return false;
  }

  OpcUaStatusCode statusCode;
  res->statusCodeArray()->get(0, statusCode);
  if (statusCode != Success) {
    Logger::log(Error, "Status Code Array response error");
    return false;
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * UnregisterResourceCallback()
 */
bool OpcUaLWM2MLib::unregisterCallbacks(OpcUaUInt32 id)
{
  Logger::log(Debug, "OpcUaLWM2MLib::unregisterCallbacks");

  OpcUaNodeId::SPtr nodeId = constructSPtr<OpcUaNodeId>();
  nodeId->set(id, namespaceIndex_);
  ServiceTransactionRegisterForwardNode::SPtr trx
      = constructSPtr<ServiceTransactionRegisterForwardNode>();
  RegisterForwardNodeRequest::SPtr  req = trx->request();
  RegisterForwardNodeResponse::SPtr res = trx->response();

  req->forwardNodeSync()->readService().unsetCallback();
  req->forwardNodeSync()->readHService().unsetCallback();
  req->forwardNodeSync()->writeService().unsetCallback();
  req->nodesToRegister()->resize(1);
  req->nodesToRegister()->set(0, nodeId);

  service().sendSync(trx);
  if (trx->statusCode() != Success) {
    Logger::log(Error, "Status code response error");
    return false;
  }

  OpcUaStatusCode statusCode;
  res->statusCodeArray()->get(0, statusCode);
  if (statusCode != Success) {
    Logger::log(Error, "Status Code Array response error");
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
  Logger::log(Debug, "OpcUaLWM2MLib::createObjectDictionary");

  for (auto& desc : ipsoDesc) {
    if (!objectDictionary_.insert(objectDictionary_t::value_type(
        desc.objectDesc.id, &desc)).second) {
      Logger::log(Error, "Object dictionary creation from IPSO descriptions failed");
      return false;
    }
  }
  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createObjectNode()
 */
bool OpcUaLWM2MLib::createObjectNode(objectMap_t& objectMap)

{
  Logger::log(Debug, "OpcUaLWM2MLib::createObjectNode");

  for (auto& objectInfo : objectMap)
  {
    /* set id of OPC UA object node */
    OpcUaNodeId objectNodeId;
    objectNodeId.set(objectInfo.first, namespaceIndex_);

    OpcUaStackServer::BaseNodeClass::SPtr objectNode = informationModel()->find(objectNodeId);
    if (objectNode.get() == nullptr) {

      OpcUaStackServer::BaseNodeClass::SPtr objectNode;
      objectNode = constructSPtr<OpcUaStackServer::ObjectNodeClass>();
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
      objectInfo.second.deviceId = deviceId_;
      parentObjectId.set(objectInfo.second.deviceId, namespaceIndex_);

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
 * DeleteObjectNode()
 */
bool OpcUaLWM2MLib::deleteObjectNode(std::string devName,
    objectMaps_t& objectMaps)
{
  Logger::log (Debug, "OpcUaLWM2MLib::deleteObjectNode");

  uint32_t deviceId;
  objectMaps_t::iterator it = objectMaps_.find(devName);
  if (it != objectMaps_.end())
  {
    objectMap_t::iterator it2 = it->second.begin();
    while (it2 != it->second.end())
    {

        /* delete object node */
        OpcUaNodeId objectNodeId;
        objectNodeId.set(it2->first, namespaceIndex_);
        informationModel()->remove(objectNodeId);

        /* store parent id of deleted object node */
        deviceId = it2->second.deviceId;

      it2++;
    }

    /* delete objectMap from objectMaps */
    objectMaps_.erase(it);
  }

  /* delete device object node */
  deleteDeviceObjecttNode(deviceId);

  std::cout << devName
            << " object nodes successfully deleted from server"
            << std::endl;

  return true;
}
/*---------------------------------------------------------------------------*/
/**
 * createObjectNode()
 */
bool OpcUaLWM2MLib::createDeviceObjectNode(const LWM2MDevice* device)
{

  Logger::log(Debug, "OpcUaLWM2MLib::createDeviceObjectNode");

  OpcUaStackServer::BaseNodeClass::SPtr deviceobjectNode;
  deviceobjectNode = constructSPtr<OpcUaStackServer::ObjectNodeClass>();

  /* set node id of object */
  OpcUaNodeId deviceObjectNodeId;
  uint32_t hash = boost::hash_value(device->getName());
  deviceId_ = hash % ID_MODULO_DIVISOR;
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
 * deleteDeviceObjecttNode()
 */
bool OpcUaLWM2MLib::deleteDeviceObjecttNode(uint32_t deviceId)
{
  Logger::log (Debug, "OpcUaLWM2MLib::deleteDeviceObjecttNode");

  OpcUaNodeId deviceIdNode;
  deviceIdNode.set(deviceId, namespaceIndex_);
  informationModel()->remove(deviceIdNode);

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createVariableNode()
 */
bool OpcUaLWM2MLib::createVariableNode (resourceMap_t& resourceMap)
{
  Logger::log(Debug, "OpcUaLWM2MLib::createVariableNode");

  /* iterate through resource map and create OPC UA variable nodes */
  for (auto& varInfo : resourceMap)
  {
    OpcUaStackServer::BaseNodeClass::SPtr variableNode;
    variableNode = constructSPtr<OpcUaStackServer::VariableNodeClass>();

    if (varInfo.second.operation == "RW" || varInfo.second.operation == "W" ||
            varInfo.second.operation == "R") {

      /* set OPC UA variable node id */
      OpcUaNodeId varNodeId;
      uint32_t resourceId =  varInfo.first;

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
      uint32_t parentObjectId = varInfo.second.opcuaObjectId;
      objectNodeId.set(parentObjectId, namespaceIndex_);

      OpcUaStackServer::BaseNodeClass::SPtr parentObject = informationModel()->find(objectNodeId);
      if (parentObject.get() != nullptr) {

        /* create references to object node */
        variableNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
              ReferenceType_HasComponent, false, objectNodeId);

        parentObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
              ReferenceType_HasComponent, true, varNodeId);
      }

        /* create LWM2M device data for OPC UA variables */
        opcUaNodeContext variableCtx;
        variableCtx = createDeviceDataLWM2M(varInfo.second, variableNode);

        /* observe variable nodes */
        if((varInfo.second.dynamicType == "Dynamic") ||
            varInfo.second.dynamicType == "Object"){
          Logger::log (Debug, "Started LWM2M resource observation for object <>, instance <>, resource <>", variableCtx.resInfo.objectId,variableCtx.resInfo.instanceId,variableCtx.resInfo.resourceId);

          bool obsDirect = false;

          if( varInfo.second.dynamicType == "Object" )
            obsDirect = false;
          else
            obsDirect = true;
          if( variableCtx.dataObject->observeVal( this, NULL, obsDirect ) == 0 )
          {
	    Logger::log(Debug, "LWM2M Observation has succeeded.");
	    Logger::log(Debug, "Registered a dynamic value observer.");
          }
          else
          {
            Logger::log(Error, "LWM2M observation has failed.");
            Logger::log(Error, "Registering an observer has failed.");
            continue;
          }
        }

         /* store variable node info into variableContextMap */
         variables_.insert(std::make_pair(varNodeId, variableCtx));

         /* add variable node to OPC UA server information model */
         informationModel()->insert(variableNode);

         /* register callback for OPC UA variable nodes */
         if (!registerCallbacks(resourceId)) {
            Logger::log (Error, "Register callback failed");
         }
    }
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createMethodNode()
 */
bool OpcUaLWM2MLib::createMethodNode(resourceMap_t& resourceMap)
{
  Logger::log  (Debug, "OpcUaLWM2MLib::createMethodNode");


  /* create Method node */
  /* iterate through method map and create OPC UA method nodes */
  for (auto& methodInfo : resourceMap)
  {

    OpcUaStackServer::BaseNodeClass::SPtr methodNode;
    methodNode = constructSPtr<OpcUaStackServer::MethodNodeClass>();

    if (methodInfo.second.operation == "E") {

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
      uint32_t parentObjectId = methodInfo.second.opcuaObjectId;
      objectNodeId.set(parentObjectId, namespaceIndex_);

      OpcUaStackServer::BaseNodeClass::SPtr parentObject = informationModel()->find(objectNodeId);
      if (parentObject.get() != nullptr) {

        /* create references to object node */
        methodNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
              ReferenceType_HasComponent, false, objectNodeId);

        parentObject->referenceItemMap().add(OpcUaStackServer::ReferenceType::
              ReferenceType_HasComponent, true, methodNodeId);
      }

      /* create LWM2M device data for OPC UA methods */
       opcUaNodeContext methodCtx;
       methodCtx = createDeviceDataLWM2M(methodInfo.second, methodNode);

       /* store method node info into methodContextMap */
       methods_.insert(std::make_pair(methodNodeId, methodCtx));

      /* add the method node to the information model */
      informationModel()->insert(methodNode);
    }
  }

  /* clear the method Map */
  resourceMap.clear();

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * deleteResourceNodes()
 */
bool OpcUaLWM2MLib::deleteResourceNodes(std::string devName,
    resourceMaps_t& resourceMaps)
{
  Logger::log  (Debug, "OpcUaLWM2MLib::deleteResourceNodes");

  resourceMaps_t::iterator it = resourceMaps_.find(devName);
  if (it != resourceMaps_.end())
  {
    auto it2 = it->second.begin();
    while (it2 != it->second.end())
    {
        /* unregister callbacks */
        unregisterCallbacks(it2->first);

        /* delete resource node */
        OpcUaNodeId resourceNodeId;
        resourceNodeId.set(it2->first, namespaceIndex_);
        informationModel()->remove(resourceNodeId);

        /* remove item from variable map */
        variableContextMap::iterator it3 = variables_.find(it2->first);
        if (it3 != variables_.end()) {
          variables_.erase(it3);
        }

        /* remove from method map */
        methodContextMap::iterator it4 = methods_.find(it2->first);
        if (it4 != methods_.end()) {
          methods_.erase(it4);
        }

      it2++;
    }

    /* delete resourceMap from resourceMaps */
    resourceMaps_.erase(it);
  }



  std::cout << devName
            << " resource nodes successfully deleted from server"
            << std::endl;

  return true;
}

/*---------------------------------------------------------------------------*/
/*
* createDeviceDataLWM2M()
*/
OpcUaLWM2MLib::opcUaNodeContext OpcUaLWM2MLib::createDeviceDataLWM2M
  (IPSOParser::ipsoResourceDescription opcUaNodeInfo , OpcUaStackServer::BaseNodeClass::SPtr opcUaNode)
{
  Logger::log  (Debug, "OpcUaLWM2MLib::createDeviceDataLWM2M");

  opcUaNodeContext ctx;

  /* set resource information */
  ctx.resInfo = opcUaNodeInfo;

  ctx.data = constructSPtr<OpcUaDataValue>();
  ctx.data->statusCode(Success);
  ctx.data->sourceTimestamp(boost::posix_time::microsec_clock::universal_time());
  ctx.data->serverTimestamp(boost::posix_time::microsec_clock::universal_time());

  if (opcUaNodeInfo.operation == "R") {

    ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
          opcUaNodeInfo.resource->getDevice()->getName()
        , opcUaNodeInfo.desc
        , opcUaNodeInfo.type
        , (DeviceData::ACCESS_READ | DeviceData::ACCESS_OBSERVE)
        , opcUaNodeInfo.resource);

  } else if (opcUaNodeInfo.operation == "W") {

    ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
          opcUaNodeInfo.resource->getDevice()->getName()
        , opcUaNodeInfo.desc
        , opcUaNodeInfo.type
        , (DeviceData::ACCESS_READ | DeviceData::ACCESS_WRITE)
        , opcUaNodeInfo.resource);

  } else if (opcUaNodeInfo.operation == "E") {

    ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
          opcUaNodeInfo.resource->getDevice()->getName()
        , opcUaNodeInfo.desc
        , opcUaNodeInfo.type
        , DeviceData::ACCESS_NONE
        , opcUaNodeInfo.resource);

  } else if  (opcUaNodeInfo.operation == "RW") {

    ctx.dataObject = boost::make_shared<DeviceDataLWM2M>(
          opcUaNodeInfo.resource->getDevice()->getName()
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
}

/*---------------------------------------------------------------------------*/
/*
* onDeviceRegister()
*/
int8_t OpcUaLWM2MLib::onDeviceRegister(std::string devName)
{
  int8_t ret = 0;

  Logger::log (Debug, "OpcUaLWM2MLib::onDeviceRegister");
  Logger::log (Trace, "Device <> registered.", devName);

  LWM2MDevice* device;
  device = const_cast<LWM2MDevice*>(
      LWM2MServer::instance()->getLWM2MDevice(devName.c_str()));

  if( device == NULL )
    ret = -1;

  if( ret == 0 )
  {
    /* create device object */
    if (!createDeviceObjectNode(device))
    {
      Logger::log (Error, "Creation of device object failed");
      ret = -1;
    }
  }

  if(ret == 0)
  {
    std::vector<LWM2MObject*>::iterator objectIterator;
    for (objectIterator = device->objectStart();
        objectIterator != device->objectEnd();
        ++objectIterator)
    {
      /* check id match between LWM2M device and object dictionary objects  */
      if (!matchObjectId((*objectIterator), objectDictionary_, objectMap_)) {
    	  Logger::log(Debug, "LWM2M Object ID exist in dictionary");
      }
    }

    if( ret == 0 )
    {
      /* create OPC UA object nodes from object map */
      if (!createObjectNode(objectMap_)) {
    	Logger::log(Debug, "Object node creation failed");
        ret = -1;
      }
    }

    if( ret == 0 )
    {
      /* create LWM2M resources of LWM2M object instances */
      if(!createLWM2MResources(objectMap_, objectDictionary_, resourceMap_)) {
    	Logger::log(Debug, "Creation of resources failed");
        ret = -1;
      }
    }

    if( ret == 0 )
    {
      /* create OPC UA variable nodes from resource map */
      if (!createVariableNode(resourceMap_)) {
    	Logger::log(Debug, "Creation of variable node failed");
        ret = -1;
      }
    }

    if( ret == 0 )
    {
      /* create OPC UA method nodes from method map */
      if (!createMethodNode(resourceMap_)) {
    	Logger::log (Debug, "Creation of method node failed");
        ret = -1;
      }
    }

    if( ret == 0 )
    {
      objectMaps_t::iterator it = objectMaps_.find( devName );
      if( it != objectMaps_.end() )
      {
        for (auto& objectInfo : it->second )
        {
          if(objectInfo.second.dynamicType == "Dynamic"){
        	  Logger::log(Debug, "Started LWM2M object observation for object <> with instance <>", objectInfo.second.id, objectInfo.second.instanceId);
              /* Add the Observer to all the Resources */

            if( LWM2MServer::instance()->observe(objectInfo.second.object, true) == 0 )
            {
            	Logger::log(Debug, "LWM2M Observation has succeeded.");
            	Logger::log(Debug, "Registered a dynamic value observer.");
            }
            else
            {
            	Logger::log(Error, "LWM2M observation has failed.");
            	Logger::log(Error, "Registering an observer has failed.");
              continue;
            }
          }
        }
      }
    }
  }

  return ret;
}

/*---------------------------------------------------------------------------*/
/*
* onDeviceUnregister()
*/
int8_t OpcUaLWM2MLib::onDeviceDeregister(std::string devName)
{
  Logger::log(Debug, "OpcUaLWM2MLib::onDeviceUnregister");
  Logger::log(Trace, "Device <> deregistered", devName);

  std::cout << devName<< " is deregistering from the server"
              << std::endl;

  /* delete variable and method nodes from server */
  if (!deleteResourceNodes(devName, resourceMaps_)) {
    Logger::log(Error, "deletion of variable and method nodes failed");
    return -1;
  }

  /* delete object and device object nodes from server */
  if (!deleteObjectNode(devName, objectMaps_)) {
    Logger::log(Error, "deletion of object nodes failed");
    return -1;
  }

  if (resourceMaps_.empty() && objectMaps_.empty()) {
      std::cout << "Server has no objects and resources created"
                << std::endl;
  }

  LWM2MDevice* p_dev = LWM2MServer::instance()->getLWM2MDevice( devName );
  if( p_dev != NULL )
  {
      std::vector< LWM2MObject* >::iterator objIt = p_dev->objectStart();
      while( objIt != p_dev->objectEnd() )
      {
        (*objIt)->clearResources();
        objIt++;
      }
  }

  return 0;
}


/*---------------------------------------------------------------------------*/
/*
* matchObjectId()
*/
bool OpcUaLWM2MLib::matchObjectId(LWM2MObject* lwm2mObj, objectDictionary_t& dictionary
    , objectMap_t& objectMap)
{
  Logger::log(Debug, "OpcUaLWM2MLib::matchObjectId");

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
      objectInfo.dynamicType = dict.second->objectDesc.dynamicType;

      /* generate unique object id */
      std::string instIdStr = boost::lexical_cast<std::string>(objectInfo.instanceId);
      std::string hashname
        = objectInfo.object->getDevice()->getName() + objectInfo.name + instIdStr;
      uint32_t hash = boost::hash_value(hashname);
      uint32_t id = hash % ID_MODULO_DIVISOR;

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
  Logger::log(Debug, "OpcUaLWM2MLib::createLWM2MResources");

  /* parent device name */
  std::string deviceName;

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
        resourceInfo.mandatoryType = resourceItem.mandatoryType;
        if( objectItem.second.dynamicType != "Dynamic")
          resourceInfo.dynamicType = resourceItem.dynamicType;
        else
          resourceInfo.dynamicType = "Object";

        /* copy node id of parent object and instance ID */
        resourceInfo.opcuaObjectId = objectItem.first;
        resourceInfo.objectId = objectItem.second.id;
        resourceInfo.instanceId = objectItem.second.instanceId;

        /* generate unique resource id */
        std::string objIdStr = boost::lexical_cast<std::string>(resourceInfo.opcuaObjectId);
        std::string hashname
          = objectItem.second.object->getDevice()->getName() + resourceInfo.name + objIdStr;
        uint32_t hash = boost::hash_value(hashname);
        uint32_t hashid = hash % ID_MODULO_DIVISOR;

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
            uint32_t resourceId =  hashid;
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
            uint32_t resourceId =  hashid;
            resourceMap.insert(resourceMap_t::value_type(resourceId, resourceInfo));

          } else if (resourceItem.operation == "E") {

            /* create resource for EXEC access */
            LWM2MResource* resource =
                new LWM2MResource(resourceInfo.resourceId, false, false, true);

            /* add resource to parent object */
            objectItem.second.object->addResource(resource);

            /* copy resource attributes */
            resourceInfo.resource = resource;

            /* add executable resource info to resource map */
            uint32_t methodId = hashid;
            resourceMap.insert(resourceMap_t::value_type(methodId, resourceInfo));

          } else if  (resourceItem.operation == "RW") {

            /* create resource for READ and WRITE access */
            LWM2MResource* resource =
                new LWM2MResource(resourceItem.resourceId, true, true, false);

            /* add resource to parent object */
            objectItem.second.object->addResource(resource);

            /* copy resource attributes */
            resourceInfo.resource = resource;

            /* add resource info to map */
            uint32_t resourceId =  hashid;
            resourceMap.insert(resourceMap_t::value_type(resourceId, resourceInfo));
          }
        }
      }
    }
    deviceName = objectItem.second.object->getDevice()->getName();
  }

  /* store created object map and resource map */
  resourceMaps_.insert(resourceMaps_t::value_type(deviceName, resourceMap));
  objectMaps_.insert(objectMaps_t::value_type(deviceName, objectMap));

  /* clear object map */
  objectMap.clear();

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createDataValue()
 */
OpcUaDataValue::SPtr OpcUaLWM2MLib::createDataValue(const DeviceDataValue* val)
{
  Logger::log(Debug, "OpcUaLWM2MLib::createObjectNode");

  OpcUaDataValue::SPtr dataValue = constructSPtr<OpcUaDataValue>();
  OpcUaDateTime dateTime (boost::posix_time::microsec_clock::universal_time());
  dataValue->sourceTimestamp(dateTime);
  dataValue->serverTimestamp(dateTime);
  dataValue->statusCode(Success);

  if (val->getType() == DeviceDataValue::TYPE_INTEGER) {
    int32_t readSensorVal = val->getVal().i32;
    dataValue->variant()->variant(readSensorVal);

  } else if (val->getType() == DeviceDataValue::TYPE_FLOAT) {
    float readSensorVal = val->getVal().f;
    dataValue->variant()->variant(readSensorVal);

  } else if (val->getType() == DeviceDataValue::TYPE_STRING) {
    std::string readSensorVal(val->getVal().cStr);
    OpcUaString::SPtr str = constructSPtr<OpcUaString>();
    str->value(readSensorVal);
    dataValue->variant()->variant(str);
  }

  return dataValue;

}

} /* namespace OpcUalwm2m */

  extern "C" DLLEXPORT void init(OpcUaStackServer::ApplicationIf** applicationIf) {
  *applicationIf = new OpcUaLWM2M::OpcUaLWM2MLib();
}

