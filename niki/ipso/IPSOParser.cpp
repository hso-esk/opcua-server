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


#include "IPSOParser.h"
#include "OpcUaStackCore/Base/Config.h"
#include "OpcUaStackCore/Base/ConfigXml.h"
#include <boost/filesystem.hpp>
#include <algorithm>
#include <cstring>
#include "NikiLogger.h"


namespace OpcUaLWM2M
{

using namespace OpcUaStackCore;

/**
 * IPSOParser ()
 */
IPSOParser::IPSOParser(void)
{
  NikiLogger::log(Trace, "IPSOParser::IPSOParser");

}

/*---------------------------------------------------------------------------*/
/**
 * ~IPSOParser ()
 */
IPSOParser::~IPSOParser()
{
	NikiLogger::log(Trace, "IPSOParser::~IPSOParser");
}

/*---------------------------------------------------------------------------*/
/**
 * parseIPSOfile()
 */
bool IPSOParser::parseIPSOfile(const std::string& IPSOfile, ipsoDescriptionVec& data)
{
  Log(Trace, "IPSOParser::parseIPSOfile");

  /* check if IPSO file exist */
  if (!boost::filesystem::exists(IPSOfile))
  {
	  NikiLogger::log(Error, "Error, IPSO file: <> not found", IPSOfile);
    return false;
  }

  ConfigXml configXml;
  /* parse the IPSO file */
  if (!configXml.parse(IPSOfile, &config_))
  {
    std::string errorMessage = configXml.errorMessage();
    NikiLogger::log(Error, "Could not parse IPSO file: <>, error: <>", IPSOfile, errorMessage);
    return false;
  }

  /* Parse IPSO objects */
  std::vector<Config> objectVec;
  config_.getChilds("LWM2M.Object", objectVec);

  if (objectVec.size() == 0)
  {
	NikiLogger::log(Error, "IPSO object descriptions not defined");
    return false;
  }

  /* process IPSO object descriptions */
  for (auto& objectChild : objectVec)
  {
    ipsoDescriptions ipsoDescription;

    if (!processIpsoObject(objectChild, ipsoDescription))
    {
      NikiLogger::log(Error, "IPSO object descriptions processing failed.");
      return false;
    }

    /* store parsed IPSO description */
    data.push_back(ipsoDescription);
  }
  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * processIpsoObject()
 */
bool IPSOParser::processIpsoObject(Config& objectChild
		, IPSOParser::ipsoDescriptions& ipsoDescription)
{
  NikiLogger::log(Trace, "IPSOParser::processIpsoObject");

  /* read IPSO object type */
  ipsoObjectDescription ipsoObject;
  boost::optional<std::string> objectType = objectChild.getValue("<xmlattr>.ObjectType");

  if (!objectType)
  {
	NikiLogger::log(Error, "IPSO Object type not defined");
    return false;
  }
  ipsoObject.type = *objectType;

  /* read IPSO object name */
  if (!objectChild.getConfigParameter("Name", ipsoObject.name))
  {
	NikiLogger::log(Error, "IPSO object name not defined");
    return false;
  }

  /* read IPSO object description */
  if (!objectChild.getConfigParameter("Description1", ipsoObject.desc))
  {
	NikiLogger::log(Error, "IPSO object description not defined");
    return false;
  }

  /* read IPSO object id */
  if (!objectChild.getConfigParameter("ObjectID", ipsoObject.id))
  {
	NikiLogger::log(Error, "IPSO object Id not defined");
    return false;
  }

  /* read IPSO object URN */
  if (!objectChild.getConfigParameter("ObjectURN", ipsoObject.urn))
  {
	NikiLogger::log(Error, "IPSO object URN not defined");
    return false;
  }

  /* read IPSO instance type */
  if (!objectChild.getConfigParameter("MultipleInstances", ipsoObject.instanceType))
  {
	NikiLogger::log(Error, "IPSO object instance type not defined");
	return false;
  }

  /* store IPSO objects info */
  ipsoDescription.objectDesc.name = ipsoObject.name;
  ipsoDescription.objectDesc.desc = ipsoObject.desc;
  ipsoDescription.objectDesc.id = ipsoObject.id;
  ipsoDescription.id = ipsoObject.id;
  ipsoDescription.objectDesc.urn = ipsoObject.urn;
  ipsoDescription.objectDesc.instanceType = ipsoObject.instanceType;

  NikiLogger::log(Trace, "Start parsing resources for <> with id: <>",ipsoDescription.objectDesc.name,ipsoDescription.objectDesc.id);

  /* Parse IPSO resources */
  std::vector<Config> resourceVec;
  objectChild.getChilds("Resources.Item", resourceVec);

  if (resourceVec.size() == 0)
  {
	  NikiLogger::log(Error, "IPSO resource descriptions does not exist");
    return false;
  }

  NikiLogger::log(Error, "Got <> resources", resourceVec.size());

  /* process IPSO resource descriptions */
  for (auto& resourceChild : resourceVec)
  {
    ipsoResourceDescription ipsoResource;

    if (!processIpsoResource(resourceChild, ipsoResource))
    {
      NikiLogger::log(Error, "IPSO resource descriptions processing failed.");
      return false;
    }

    /* store parsed IPSO resource */
    ipsoDescription.resourceDesc.push_back(ipsoResource);
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * processIpsoResource()
 */
bool IPSOParser::processIpsoResource(Config& resourceChild
		, IPSOParser::ipsoResourceDescription& ipsoResource)
{
  NikiLogger::log(Trace, "IPSOParser::processIpsoResource");

  /* read IPSO resource ID */
  if (!resourceChild.getConfigParameter("<xmlattr>.ID", ipsoResource.resourceId))
  {
	NikiLogger::log(Error, "IPSO resource Id not defined");
    return false;
  }

  /* read IPSO resource name */
  if (!resourceChild.getConfigParameter("Name", ipsoResource.name))
  {
	NikiLogger::log(Error, "IPSO resource name not defined");
    return false;
  }

  /* read IPSO resource operation */
  if (!resourceChild.getConfigParameter("Operations", ipsoResource.operation))
  {
	NikiLogger::log(Error, "IPSO resource operation not defined");
    return false;
  }

  /* read IPSO resource instance type */
  if (!resourceChild.getConfigParameter("MultipleInstances", ipsoResource.instanceType))
  {
	NikiLogger::log(Error, "IPSO resource instance type not defined");
    return false;
  }

  /* read IPSO resource dynamic data value? */
  if (!resourceChild.getConfigParameter("Dynamic", ipsoResource.dynamicType))
  {
	NikiLogger::log(Error, "IPSO resource dynamic type not defined");
    return false;
  }

  /* read IPSO resource mandatory field */
  if (!resourceChild.getConfigParameter("Mandatory", ipsoResource.mandatoryType))
  {
	NikiLogger::log(Error, "IPSO resource mandatory type not defined");
    return false;
  }

  /* read IPSO resource datatype */
  boost::optional<std::string> type = resourceChild.getValue("Type");
  if (!type)
  {
	NikiLogger::log(Error, "IPSO resource data type not defined");
    return false;
  }

  if (*type == "Integer") {
	ipsoResource.type = DeviceDataValue::TYPE_INTEGER;
    ipsoResource.value.i32  = 0;

  } else if (*type == "Float") {
    ipsoResource.type = DeviceDataValue::TYPE_FLOAT;
    ipsoResource.value.f = 0.0;

  } else if (*type == "String") {
    ipsoResource.type = DeviceDataValue::TYPE_STRING;
    memset(ipsoResource.value.cStr, 0, DEVICEDATAVALUE_STRMAX);
  }

  /* read IPSO range enumeration */
  boost::optional<std::string> rangeEnumeration = resourceChild.getValue("RangeEnumeration");
  if (!rangeEnumeration)
  {
	NikiLogger::log(Error, "IPSO resource range Enumeration not defined");
    return false;
  }
  ipsoResource.rangeEnum = *rangeEnumeration;

  /* read IPSO resource unit */
  boost::optional<std::string> unit = resourceChild.getValue("Units");
  if (!unit)
  {
	NikiLogger::log(Error, "IPSO resource unit not defined");
    return false;
  }
  ipsoResource.unit = *unit;

  /* read IPSO resource description */
  if (!resourceChild.getConfigParameter("Description", ipsoResource.desc))
  {
	NikiLogger::log(Error, "IPSO resource description not defined");
    return false;
  }

  return true;
}

} /* namespace OpcUaLWM2M */

