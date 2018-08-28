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

#include "opcuaDatabaseConfig.h"

using namespace OpcUaStackCore;

namespace OpcUaPluginDB
{
OpcUaPluginDatabaseConfig::OpcUaPluginDatabaseConfig(void)
  : databaseName_("")
  , databaseTableName_("")
{
}

OpcUaPluginDatabaseConfig::~OpcUaPluginDatabaseConfig(void)
{
}

std::string OpcUaPluginDatabaseConfig::dsnName(void)
{
  return dbaseConfig_.dsnName();
}

std::string& OpcUaPluginDatabaseConfig::userName(void)
{
  return dbaseConfig_.userName();
}

std::string& OpcUaPluginDatabaseConfig::password(void)
{
  return dbaseConfig_.password();
}

std::string& OpcUaPluginDatabaseConfig::databaseName(void)
{
  return databaseName_;
}

std::string& OpcUaPluginDatabaseConfig::databaseTableName(void)
{
  return databaseTableName_;
}

bool OpcUaPluginDatabaseConfig::decode(Config& config)
{
  bool success;
  /* decode data source, userName and password */
  dbaseConfig_.decode(config);

  /* get database name */
  config.getConfigParameter("DbName", databaseName_, "");

  /* get database table name */
  config.getConfigParameter("DbTableName", databaseTableName_, "");

  return true;
}

} /* namespace OpcUaPluginDB*/


