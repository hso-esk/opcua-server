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

    /* parent device id of object */
    uint32_t deviceId;

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

    LWM2MObject* object;
  };

  /**
   * \brief   IPSO resource description
   */
  struct ipsoResourceDescription
  {
    /* IPSO resource id */
    uint32_t resourceId;

    /* IPSO instance id */
    uint32_t instanceId;

    /* parent id of resource */
    uint32_t objectId;

    /* IPSO resource name */
    std::string name;

    /* IPSO resource operation */
    std::string operation;

    /* IPSO resource mandatory type */
    std::string mandatoryType;

    /* IPSO resource instance Type */
    std::string instanceType;

    /* IPSO resource data type */
    DeviceDataValue::e_type type;

    /* IPSO resource data value */
    DeviceDataValue::u_val value;

    /* IPSO resource range enumeration */
    boost::optional<std::string> rangeEnum;

    /* IPSO resource dynamic data value? */
    std::string dynamicType;

    /* IPSO resource units */
    std::string unit;

    /* IPSO resource description */
    std::string desc;

    LWM2MResource* resource;
  };

  /**
   * \brief   Struct declaration of object dictionary
   */
  struct ipsoDescriptions
  {
    /* IPSO object Id */
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

  OpcUaStackCore::Config config_;
};

} /* namespace OpcUaLWM2M */


#endif /* IPSOPARSER_H_ */
