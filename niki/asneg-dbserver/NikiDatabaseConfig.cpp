/*
 * nikiDbConfig.cpp
 *
 *  Created on: 6 Sep 2017
 *      Author: osboxes
 */

#include "OpcUaStackCore/Base/Log.h"
#include "NikiDatabaseConfig.h"

using namespace OpcUaStackCore;

namespace OpcUaNikiDB
{
/*---------------------------------------------------------------------------*/
/**
 * NikiDatabaseConfig ()
 */
NikiDatabaseConfig::NikiDatabaseConfig(void)
  : databaseName_("")
  , databaseTableName_("")
{
  Log(Debug, "NikiDatabaseConfig::NikiDatabaseConfig");
}

/*---------------------------------------------------------------------------*/
/**
 * ~NikiDatabaseConfig ()
 */
NikiDatabaseConfig::~NikiDatabaseConfig(void)
{
  Log(Debug, "NikiDatabaseConfig::~NikiDatabaseConfig");
}

/*---------------------------------------------------------------------------*/
/**
 * dsnName ()
 */
std::string NikiDatabaseConfig::dsnName(void)
{
  Log(Debug, "NikiDatabaseConfig::dsnName");

  return dbaseConfig_.dsnName();
}

/*---------------------------------------------------------------------------*/
/**
 * userName ()
 */
std::string& NikiDatabaseConfig::userName(void)
{
  Log(Debug, "NikiDatabaseConfig::userName");

  return dbaseConfig_.userName();
}

/*---------------------------------------------------------------------------*/
/**
 * password ()
 */
std::string& NikiDatabaseConfig::password(void)
{
  Log(Debug, "NikiDatabaseConfig::password");

  return dbaseConfig_.password();
}

/*---------------------------------------------------------------------------*/
/**
 * databaseName ()
 */
std::string& NikiDatabaseConfig::databaseName(void)
{
  Log(Debug, "NikiDatabaseConfig::databaseName");

  return databaseName_;
}

/*---------------------------------------------------------------------------*/
/**
 * databaseTableName ()
 */
std::string& NikiDatabaseConfig::databaseTableName(void)
{
  Log(Debug, "NikiDatabaseConfig::databaseTableName");

  return databaseTableName_;
}

/*---------------------------------------------------------------------------*/
/**
 * decode ()
 */
bool NikiDatabaseConfig::decode(Config& config)
{
  Log(Debug, "NikiDatabaseConfig::decode");

  bool success;
  /* decode data source, userName and password */
  dbaseConfig_.decode(config);

  /* get database name */
  config.getConfigParameter("DbName", databaseName_, "");

  /* get database table name */
  config.getConfigParameter("DbTableName", databaseTableName_, "");

  return true;
}

} /* namespace OpcUaNikiDB*/


