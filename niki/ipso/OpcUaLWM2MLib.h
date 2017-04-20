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
   * \brief   Variable context
   */
  struct variableContext
  {
    /* data of OPC UA variable Node */
    OpcUaDataValue::SPtr data;

    /* device data object */
    boost::shared_ptr<DeviceData> dataObject;
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
  std::vector<int16_t> objectIdVec_;

  /* namespaceIndex of OPC UA nodes */
  OpcUaUInt16 namespaceIndex_;
  OpcUaStackServer::BaseNodeClass::SPtr rootNode_;

  /* OPC UA variable node read and write callbacks */
  Callback readSensorValueCallback_;
  Callback writeSensorValueCallback_;

  /* IPSOParser instance */
  IPSOParser ipsoParser_;
  IPSOParser::ipsoDescriptionVec data_;

  /* LWM2MServer instance */
  boost::shared_ptr<LWM2MServer> lwm2mServer_;

  /* OpcUa LWM2M server observer instance */
  OpcUaLWM2MObserver opcUalwm2mObs;

  /* variable context map */
  typedef std::map<OpcUaNodeId, variableContext> variableContextMap;
  variableContextMap variables_;

  /* object dictionary map */
  typedef std::map<uint32_t, IPSOParser::ipsoDescriptions*> objectDictionary_t;
  objectDictionary_t objectDictionary;

  typedef std::vector<LWM2MObject*> objectVec_t;
  objectVec_t objectVec_;

  typedef boost::tuple<std::string, int32_t, int32_t, int32_t> resourceId_t;

  typedef std::map<resourceId_t, IPSOParser::ipsoResourceDescription> resourceMap_t;
  resourceMap_t resourceMap_;

  /* IPSO object Id */
  uint32_t id_;

  /* LWM2M device id */
  uint16_t deviceId_;

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
     * \brief   Registers read and write callbacks of variable node.
     */
  bool registerCallbacks(OpcUaUInt32 id);

  /**
   * \brief   Unregisters read and write callbacks of variable node.
   */
  bool unregisterCallbacks();

  /**
   * \brief   Creates object dictionary from parsed IPSO descriptions.
   */
  bool createObjectDictionary(IPSOParser::ipsoDescriptionVec& ipsoDesc);

  /**
  * \brief   Create Variable Node to the information model.
  */
  bool createDeviceObjectNode(const LWM2MDevice* p_dev);

  /**
   * \brief   Create Object Node to the information model.
   */
  bool createObjectNode(IPSOParser::ipsoObjectDescription const& objectInfo);

  /**
    * \brief   Create Variable Node to the information model.
    */
  bool createVariableNode(IPSOParser::ipsoResourceDescription& varInfo);


  /**
   * \brief   Creates OPC UA nodes.
   */
  bool createNodes(void);


  /**
    * \brief   Checks for Object ID match between LWM2M server and object dictionary.
    */
  bool hasObjectId(int16_t id, objectDictionary_t dictionary);


  /**
   * \brief   Create LWM2M resources.
   */
  bool createLWM2MResources(objectVec_t& objVec
		, std::vector<IPSOParser::ipsoResourceDescription>& ipsoResource
		, resourceMap_t& resourceMap);
};

} /* namespace OpcUalwm2m */


#endif /* OPCUAIPSOLIB_H_ */
