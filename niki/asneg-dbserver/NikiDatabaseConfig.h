/*
 * nikiDbConfig.h
 *
 *  Created on: 6 Sep 2017
 *      Author: osboxes
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
