/*
 * NikiDBModelConfig.h
 *
 *  Created on: 6 Sep 2017
 *      Author: osboxes
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
