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


#ifndef NIKIDBCONFIG_H_
#define NIKIDBCONFIG_H_

#include "OpcUaDB/DBServer/DatabaseConfig.h"
#include "OpcUaStackCore/Base/ConfigXmlManager.h"

using namespace OpcUaStackCore;

namespace OpcUaNikiDB
{

  class NikiDatabaseConfig
  {
    public:

    /**
     * \brief   Default Constructor
     */
    NikiDatabaseConfig(void);

    /**
     * \brief   Default Destructor
     */
    ~NikiDatabaseConfig(void);

    /**
     * \brief   Gets database data source name.
     */
    std::string dsnName(void);

    /**
     * \brief   Gets database user name.
     */
    std::string& userName(void);

    /**
     * \brief   Gets database user password.
     */
    std::string& password(void);

    /**
     * \brief   Gets database name.
     */
    std::string& databaseName(void);

    /**
     * \brief   Gets database table name.
     */
    std::string& databaseTableName(void);

    /**
     * \brief   Decodes database config.
     */
    bool decode(Config& config);

    private:
    std::string databaseName_;
    std::string databaseTableName_;
    OpcUaDB::DatabaseConfig dbaseConfig_;
  };

} /* namespace OpcUaNikiDB */




#endif /* NIKIDBCONFIG_H_ */
