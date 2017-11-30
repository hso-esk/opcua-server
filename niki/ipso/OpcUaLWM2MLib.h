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


#ifndef OPCUAIPSOLIB_H_
#define OPCUAIPSOLIB_H_

#include "IPSOParser.h"
#include "OpcUaStackCore/Application/ApplicationReadContext.h"
#include "OpcUaStackCore/Application/ApplicationHReadContext.h"
#include "OpcUaStackCore/Application/ApplicationWriteContext.h"
#include "OpcUaStackServer/Application/ApplicationIf.h"
#include "OpcUaStackServer/AddressSpaceModel/BaseNodeClass.h"
#include "DeviceDataValue.h"
#include "DeviceDataLWM2M.h"
#include "DeviceDataFile.h"
#include "OpcUaLWM2MObserver.h"
#include "NikiDatabaseServer.h"
#include "NikiDbModelConfig.h"
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>


namespace OpcUaLWM2M
{

class OpcUaLWM2MLib
  : public OpcUaStackServer::ApplicationIf
{

public:

  typedef std::map<uint32_t, IPSOParser::ipsoDescriptions*> objectDictionary_t;
  typedef boost::tuple<std::string, int32_t, int32_t, int32_t> resourceId_t;
  typedef std::map<uint32_t, IPSOParser::ipsoResourceDescription> resourceMap_t;
  typedef std::map<uint32_t, IPSOParser::ipsoObjectDescription> objectMap_t;
  typedef std::map<std::string, resourceMap_t> resourceMaps_t;
  typedef std::map<std::string, objectMap_t> objectMaps_t;

  /**
   * \brief   Default Constructor.
   */
  OpcUaLWM2MLib(void);

  /**
   * \brief   Default Destructor
   */
  virtual ~OpcUaLWM2MLib(void);

  /**
   * \brief   OPC UA node context
   */
  struct opcUaNodeContext
  {
    /* data of OPC UA variable Node */
    OpcUaDataValue::SPtr data;

    /* observed OPC UA datavalue*/
    OpcUaDataValue::SPtr obsDataValue;

    /* device data object */
    boost::shared_ptr<DeviceData> dataObject;

    /* Resource description object */
    IPSOParser::ipsoResourceDescription resInfo;
  };
  typedef std::map<OpcUaNodeId, opcUaNodeContext> variableContextMap;
  typedef std::map<OpcUaNodeId, opcUaNodeContext> methodContextMap;

  /**
   * \brief   Starts up the OpcUaIPSO library.
   */
  virtual bool startup(void);

  /**
   * \brief   Shuts down the OpcUaIPSO library.
   */
  virtual bool shutdown(void);

  /**
   * \brief   Function triggered when a new device registers.
   */
   int8_t onDeviceRegister(const LWM2MDevice* dev);

   /**
    * \brief   Function triggered when a new device deregisters.
    */
   int8_t onDeviceDeregister(const LWM2MDevice* dev);

   /**
    * \brief   processes observed data.
    */
   void processObserveData (const DeviceDataValue* p_val);
  // static bool isObserved;
private:

  std::string ipsofileName_;
  std::vector<std::string> ipsofileNameVec_;
  OpcUaUInt16 namespaceIndex_;
  /* LWM2M device id */
  uint32_t deviceId_;
  IPSOParser ipsoParser_;
  IPSOParser::ipsoDescriptionVec data_;
  boost::shared_ptr<LWM2MServer> lwm2mServer_;
  OpcUaNikiDB::DbServer dbServer_;
  OpcUaNikiDB::NikiDBModelConfig dbModelConfig_;
  OpcUaLWM2MObserver opcUalwm2mObs_;
  static bool isObserved;

  /* OPC UA node access callbacks */
  Callback readSensorValueCallback_;
  Callback readHistorySensorValueCb_;
  Callback writeSensorValueCallback_;
  Callback execSensorMethodCallback_;

  /* data structure instance */
  variableContextMap variables_;
  methodContextMap methods_;
  objectDictionary_t objectDictionary_;
  resourceMap_t resourceMap_;
  objectMap_t objectMap_;
  resourceMaps_t resourceMaps_;
  objectMaps_t objectMaps_;

  /**
   * \brief   Load IPSO file from OPC UA server configuration file.
   */
  bool loadServerConfig(void);

  /**
   * \brief   Load IPSO config file.
   */
  bool decodeIPSOConfig(const std::string& configFileName);

  /**
   * \brief   Load Database config file.
   */
  bool decodeDbConfig(const std::string& configFile);

  /**
   * \brief   Updates the OPC UA application read Context.
   */
  void readSensorValue (ApplicationReadContext* applicationReadContext);

  /**
   * \brief   reads history data of sensor.
   */
  void readHistorySensorValue (ApplicationHReadContext* applicationHReadContext);

  /**
   * \brief   Updates the OPC UA application write Context.
   */
  void writeSensorValue (ApplicationWriteContext* applicationWriteContext);

  /**
   * \brief   Call Sensor Method - not implemented.
   */
  void execSensorMethod(ApplicationReadContext* applicationReadContext);

  /**
   * \brief   Registers read and write callbacks of variable node.
   */
  bool registerCallbacks(OpcUaUInt32 id);

  /**
   * \brief   Unregisters read and write callbacks of variable node.
   */
  bool unregisterCallbacks(OpcUaUInt32 id);

  /**
   * \brief   Creates object dictionary from parsed IPSO descriptions.
   */
  bool createObjectDictionary(IPSOParser::ipsoDescriptionVec& ipsoDesc);

  /**
  * \brief   Create Variable Node to the information model.
  */
  bool createDeviceObjectNode(const LWM2MDevice* p_dev);

  /**
   * \brief   Delete device object node from information model.
   */
  bool deleteDeviceObjecttNode(uint32_t deviceId);

  /**
   * \brief   Create Object Node to the information model.
   */
  bool createObjectNode(objectMap_t& objectMap);

  /**
   * \brief   Delete object node from information model.
   */
   bool deleteObjectNode(const LWM2MDevice* p_dev, objectMaps_t& objectMaps);

  /**
   * \brief   Create Variable Node to the information model.
   */
  bool createVariableNode(resourceMap_t& resourceMap);

  /**
   * \brief   Create method Node to the information model.
   */
  bool createMethodNode(resourceMap_t& resourceMap);

  /**
   * \brief   Delete variable and method nodes from information model.
   */
  bool deleteResourceNodes(const LWM2MDevice* p_dev, resourceMaps_t& resourceMaps);

  /**
   * \brief   Checks for object ID match between LWM2M Device objects
   *  and object dictionary objects.
   */
  bool matchObjectId(LWM2MObject* lwm2mObj
      , objectDictionary_t& dictionary
      , objectMap_t& objectMap);

  /**
   * \brief   Create LWM2M resources.
   */
  bool createLWM2MResources(objectMap_t& objectMap
      , objectDictionary_t& dictionary
      , resourceMap_t& resourceMap);

  /**
   * \brief   Create LWM2M device data for nodes.
   */
  opcUaNodeContext createDeviceDataLWM2M(IPSOParser::ipsoResourceDescription opcUaNodeInfo
     , OpcUaStackServer::BaseNodeClass::SPtr opcUaNode);

 OpcUaDataValue::SPtr createDataValue(const DeviceDataValue* value);
};

} /* namespace OpcUalwm2m */


#endif /* OPCUAIPSOLIB_H_ */
