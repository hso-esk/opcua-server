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


#ifndef NIKIDBMODELCONFIG_H_
#define NIKIDBMODELCONFIG_H_

#include "OpcUaStackCore/Base/ConfigXmlManager.h"
#include "NikiDatabaseConfig.h"

namespace OpcUaNikiDB
{

class NikiDBModelConfig
{
  public:

  /**
   * \brief   Default Constructor
   */
  NikiDBModelConfig(void);

  /**
   * \brief   Default Destructor
   */
  ~NikiDBModelConfig(void);

  /**
   * \brief   Returns reference to DatabaseConfig class
   */
  NikiDatabaseConfig& databaseConfig(void);

  /**
   * \brief   decodes database config
   */
  bool decode(Config& config);

  private:
  NikiDatabaseConfig dbConfig_;

};

}




#endif /* NIKIDBMODELCONFIG_H_ */
