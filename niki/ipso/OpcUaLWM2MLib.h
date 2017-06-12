/*
 * OpcUaIPSOLib.h
 *
 *  Created on: 24 Feb 2017
 *      Author: osboxes
 */

#ifndef OPCUAIPSOLIB_H_
#define OPCUAIPSOLIB_H_

#include "IPSOParser.h"
#include "OpcUaStackCore/Application/ApplicationReadContext.h"
#include "OpcUaStackCore/Application/ApplicationWriteContext.h"
#include "OpcUaStackServer/Application/ApplicationIf.h"
#include "OpcUaStackServer/AddressSpaceModel/BaseNodeClass.h"
#include "DeviceDataValue.h"
#include "DeviceDataLWM2M.h"
#include "DeviceDataFile.h"
#include "OpcUaLWM2MObserver.h"
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>


namespace OpcUaLWM2M
{

class OpcUaLWM2MLib
  : public OpcUaStackServer::ApplicationIf
{

public:

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

    /* device data object */
    boost::shared_ptr<DeviceData> dataObject;

    /* Resource description object */
    IPSOParser::ipsoResourceDescription resInfo;
  };

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


private:

  /* string to store loaded ipso file */
  std::string ipsofileName_;
  std::vector<std::string> ipsofileNameVec_;

  /* namespaceIndex of OPC UA nodes */
  OpcUaUInt16 namespaceIndex_;

  /* LWM2M device id */
  uint32_t deviceId_;

  /* OPC UA nodes Callbacks */
  Callback readSensorValueCallback_;
  Callback writeSensorValueCallback_;
  Callback execSensorMethodCallback_;

  /* IPSO Parser instance */
  IPSOParser ipsoParser_;

  /* storage for parsed IPSO XML files */
  IPSOParser::ipsoDescriptionVec data_;

  /* LWM2MServer instance */
  boost::shared_ptr<LWM2MServer> lwm2mServer_;

  /* OpcUa LWM2M server observer instance */
  OpcUaLWM2MObserver opcUalwm2mObs_;

  /* OPC UA variable context map */
  typedef std::map<OpcUaNodeId, opcUaNodeContext> variableContextMap;
  variableContextMap variables_;

  /* OPC UA method context map */
  typedef std::map<OpcUaNodeId, opcUaNodeContext> methodContextMap;
  methodContextMap methods_;

  /* object dictionary map */
  typedef std::map<uint32_t, IPSOParser::ipsoDescriptions*> objectDictionary_t;
  objectDictionary_t objectDictionary_;

  typedef boost::tuple<std::string, int32_t, int32_t, int32_t> resourceId_t;

  typedef std::map<uint32_t, IPSOParser::ipsoResourceDescription> resourceMap_t;
  resourceMap_t resourceMap_;

  typedef std::map<uint32_t, IPSOParser::ipsoObjectDescription> objectMap_t;
  objectMap_t objectMap_;

  typedef std::map<std::string, resourceMap_t> resourceMaps_t;
  resourceMaps_t resourceMaps_;

  typedef std::map<std::string, objectMap_t> objectMaps_t;
  objectMaps_t objectMaps_;

  /**
   * \brief   Load IPSO file from OPC UA server configuration file.
   */
  bool loadConfig(void);

  /**
   * \brief   Updates the OPC UA application read Context.
   */
  void readSensorValue (ApplicationReadContext* applicationReadContext);

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
};

} /* namespace OpcUalwm2m */


#endif /* OPCUAIPSOLIB_H_ */
