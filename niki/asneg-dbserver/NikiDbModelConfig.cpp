/*
 * NikiDBModelConfig.cpp
 *
 *  Created on: 6 Sep 2017
 *      Author: osboxes
 */

#include "NikiDbModelConfig.h"
#include "OpcUaStackCore/Base/Log.h"


namespace OpcUaNikiDB
{

/*---------------------------------------------------------------------------*/
/**
 * NikiDBModelConfig ()
 */
NikiDBModelConfig::NikiDBModelConfig(void)
  : dbConfig_()
{
  Log(Debug, "NikiDBModelConfig::NikiDBModelConfig");
}

/*---------------------------------------------------------------------------*/
/**
 * ~NikiDBModelConfig ()
 */
NikiDBModelConfig::~NikiDBModelConfig(void)
{
  Log(Debug, "NikiDBModelConfig::~NikiDBModelConfig");
}

/*---------------------------------------------------------------------------*/
/**
 *  databaseConfig ()
 */
NikiDatabaseConfig&
NikiDBModelConfig::databaseConfig(void)
{
  Log(Debug, "NikiDBModelConfig::databaseConfig");

  return dbConfig_;
}

/*---------------------------------------------------------------------------*/
/**
 *  decode ()
 */
bool NikiDBModelConfig::decode(Config& config)
{
  Log(Debug, "NikiDBModelConfig::decode");

  /* get database configuration */
  boost::optional<Config> child = config.getChild("Database");
  if (!child) {
    Log(Error, "element missing in config file")
      .parameter("Element", "DBModel.Database")
      .parameter("ConfigFileName", config.configFileName());
    return false;
  }
  if (!dbConfig_.decode(*child)) {
    return false;
  }

  return true;
}

} /* namespace OpcUaNikiDB */
