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
