/*
 * nikiDbServer.h
 *
 *  Created on: 5 Sep 2017
 *      Author: osboxes
 */

#ifndef NIKIDBSERVER_H_
#define NIKIDBSERVER_H_

#include "OpcUaDB/DBServer/DBServer.h"
//#include "OpcUaDB/DBServer/DBModelConfig.h"
#include "OpcUaDB/odbc/Connection.h"
#include "NikiDatabaseConfig.h"
#include "NikiDbModelConfig.h"


namespace OpcUaNikiDB
{

/**
 * \brief   Database Server Class.
 *
 */
class DbServer
{
public:
  typedef std::map<std::string, std::string> dataMap;

  /**
   * \brief   Constructor.
   */
  DbServer(void);

  /**
   * \brief   Destructor.
   */
  ~DbServer(void);

  /**
   * \brief   starts up database server.
   */
  bool startup(void);

  /**
    * \brief   Sets the data base config.
    */
  void dbModelConfig(NikiDBModelConfig* dbModelConfig);

  /**
   * \brief   Reference to Asneg Database server.
   */
  OpcUaDB::DBServer& DBServer(void);

  /**
   * \brief   shuts down database server.
   */
  bool shutdown(void);

  /**
   * \brief   creates database.
   */
  bool createDatabase(std::string& dbName);

  /**
   * \brief   creates database table.
   */
  bool createDatabaseTable(std::string& dbTableName);

  /**
   * \brief   writes the data value of a node to database.
   */
  bool writeDataToDatabase(std::string& dbTableName
    , OpcUaNodeId& nodeId
    , OpcUaDataValue& dataValue);

  /**
   * \brief   reads history data values of a node from database.
   *
   * \param    nodeId          nodeId of node to read history data.
   * \param    startTime       start time to read history data.
   * \param    stopTime        end time to read history data.
   * \param    dataValues      Vector to store results.
   */
  bool readDataFromDatabase(std::string& dbTableName
    , OpcUaNodeId& nodeId
    , OpcUaDateTime startTime
    , OpcUaDateTime stopTime
    , OpcUaDataValue::Vec& dataValues);


private:
  /**
   * \brief   executes SQL query.
   */
  bool execSQLDirect(const std::string& sqlQuery);

  /**
    * \brief   executes SQL query.
    *
    * \param    sqlQuery     query to execute.
    * \param    dataValues   Vector to store dataValue result.
    */
  bool execSQLDirect(const std::string& sqlQuery, OpcUaDataValue::Vec& dataValues);

  /**
    * \brief   parses the SQL query result.
    *
    * \param    inputString     SQL dataValue query result.
    * \param    dataStrMap      Map to store parsed SQL query result.
    */
  bool parseDataValue(std::string& inputString, dataMap& dataStrMap);

  /**
    * \brief   creates resultSets from database tables.
    *
    * \param    resultSet       stored database table result.
    * \param    statusCode      status code.
    * \param    header          header of database table.
    * \param    dataVec         Vector to store results.
    */
  bool createResultSet(OpcUaDB::ResultSet& resultSet
      , OpcUaDataValue::SPtr& statusCode
      , OpcUaDataValue::SPtr& header
      , OpcUaDataValue::Vec& dataVec);


  OpcUaDB::DBServer dbServer_;
  NikiDBModelConfig* dbModelConfig_;


};

} /* namespace OpcUaNikiDB */

#endif /* NIKIDBSERVER_H_ */
