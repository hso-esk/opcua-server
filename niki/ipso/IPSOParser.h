/*
 * IPSOParser.h
 *
 *  Created on: 21 Feb 2017
 *      Author: osboxes
 */

#ifndef IPSOPARSER_H_
#define IPSOPARSER_H_

#include "OpcUaStackCore/Base/Config.h"
#include "DeviceData.h"
#include "DeviceDataValue.h"
#include "DeviceDataLWM2M.h"
#include <boost/variant.hpp>
#include <string>
#include <vector>


namespace OpcUaLWM2M
{

/**
 * \brief   IPSO Parser Class.
 *
 */
class IPSOParser
{

public:

   /**
   * \brief   Default Constructor.
   */
  IPSOParser ();

  /**
   * \brief   Default destructor.
   */
  virtual ~IPSOParser ();

  /**
   * \brief   IPSO object description
   */
  struct ipsoObjectDescription
  {
    /* IPSO object id */
    uint32_t id;

    /* instance id */
    uint32_t instanceId;

    /* IPSO object type */
    std::string type;

    /* IPSO object name */
    std::string name;

    /* IPSO object description */
    std::string desc;

    /* IPSO object unique identity */
    std::string urn;

    /* IPSO object instance type */
    std::string instanceType;

    /* device object is attached to */
    std::string deviceName;
  };

  /**
   * \brief   IPSO resource description
   */
  struct ipsoResourceDescription
  {
    /* IPSO resource id */
    uint32_t resourceId;

    /* parent id of resource */
    uint32_t objectId;

    /* parent instance id */
    uint32_t objectInstanceId;

    /* IPSO resource name */
    std::string name;

    /* IPSO resource operation */
    std::string operation;

    /* IPSO resource instance Type */
    std::string instanceType;

    /* IPSO resource data type */
    DeviceDataValue::e_type type;

    /* IPSO resource data value */
    DeviceDataValue::u_val value;

    /* IPSO resource range enumeration */
    boost::optional<std::string> rangeEnum;

    /* IPSO resource units */
    std::string unit;

    /* IPSO resource description */
    std::string desc;

    /* parent device */
    std::string deviceName;

    LWM2MResource* resource;
  };

  /**
   * \brief   Struct declaration of object dictionary
   */
  struct ipsoDescriptions
  {
    /* IPSO object Id */
    //uint32_t id;
    int16_t id;

    /* IPSO object descriptions */
    ipsoObjectDescription objectDesc;

    /* IPSO resource descriptions */
    std::vector<ipsoResourceDescription> resourceDesc;
  };
  typedef std::vector<ipsoDescriptions> ipsoDescriptionVec;


  /**
   * \brief   parse IPSO file.
   */
  bool parseIPSOfile (const std::string& IPSOfile, ipsoDescriptionVec& data);


  /**
   * \brief   process IPSO objects.
   */
  bool processIpsoObject(OpcUaStackCore::Config& node
		  , ipsoDescriptions& ipsoDescription);

  /**
   * \brief   process IPSO resources.
   */
  bool processIpsoResource(OpcUaStackCore::Config& resourceChild
  		, IPSOParser::ipsoResourceDescription& ipsoDescription);

private:

  /* IPSO file to parse */
  std::string IPSOfile_;

  OpcUaStackCore::Config config_;

  friend class OpcUaLWM2MObserver;

};

} /* namespace OpcUaLWM2M */


#endif /* IPSOPARSER_H_ */
