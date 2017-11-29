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
 * This file was developed by [Kofi Atta Nsiah], [Hahn-Schickard-Gesellschaft
 * für angewandte Forschung e.V.]
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


