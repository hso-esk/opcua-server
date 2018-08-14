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

#include "NikiDatabaseServer.h"

#include "Logger.h"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace OpcUaNikiDB
{
  using namespace OpcUaLWM2M;

DbServer::DbServer(void) : dbModelConfig_(nullptr)
{
  Logger::log(Debug, "DbServer::DbServer");
}

DbServer::~DbServer(void)
{
  Logger::log(Debug, "DbServer::~DbServer");
  if(!disconnectDatabase()){
    Logger::log(Error,"Failed to dissconnect from the database during server destruction.");
  }
}

bool DbServer::startup(void)
{
  Logger::log(Debug, "DbServer::startup");

  if(!connectDatabase()){
    Logger::log(Error,"Failed connecting to the Database server");
    return false;
  }

  if (!createDatabase(dbModelConfig_->databaseConfig().databaseName()))
  {
    return false;
  }

  if (!createDatabaseTable(dbModelConfig_->databaseConfig().databaseTableName()))
  {
    return false;
  }

  return true;
}

void DbServer::dbModelConfig(NikiDBModelConfig *dbModelConfig)
{
  Logger::log(Debug, "DbServer::dbModelConfig");

  dbModelConfig_ = dbModelConfig;
}

OpcUaDB::DBServer &DbServer::DBServer(void)
{
  Logger::log(Debug, "OpcUaDB::DbServer::DBServer");

  return dbServer_;
  
}

bool DbServer::shutdown(void)
{
  Logger::log(Debug, "DbServer::shutdown");

  if(!disconnectDatabase()){
    Logger::log(Error, "Error disconecting from the database.");
  }

  dbServer_.shutdown();
  return true;
}

bool DbServer::createDatabase(std::string &dbName)
{
  Logger::log(Debug, "DbServer::createDatabase");

  std::string sqlQuery = "CREATE DATABASE IF NOT EXISTS " + dbName;
  bool success = execSQLDirect(sqlQuery);
  if (success)
  {
    Logger::log(Debug, "Database created successfully");
  }
  else
  {
    Logger::log(Debug, "Database already exist");
  }

  return true;
}

bool DbServer::createDatabaseTable(std::string &dbTableName)
{
  Logger::log(Debug, "DbServer::createDatabaseTable");

  std::string dbNameTableName = str(
      boost::format("%s.%s") % dbModelConfig_->databaseConfig().databaseName() % dbTableName);

  std::string sqlQuery = str(
      boost::format("CREATE TABLE IF NOT EXISTS %s (id  INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                    "nodeId varchar(255), sourceTimeStamp varchar(255),"
                    "ServerTimeStamp varchar(255),  dataValue varchar(255))") %
      dbNameTableName);

  bool success = execSQLDirect(sqlQuery);
  if (success)
  {
    Logger::log(Debug, "Database table created successfully");
  }
  else
  {
    Logger::log(Debug, "Database table already exist");
  }

  return true;
}

bool DbServer::writeDataToDatabase(std::string &dbTableName, OpcUaNodeId &nodeId, OpcUaDataValue &dataValue)
{
  std::string dbNameTableName = str(
      boost::format("%s.%s") % dbModelConfig_->databaseConfig().databaseName() % dbTableName);

  std::string sqlQuery = str(
      boost::format("INSERT INTO %s (nodeId, sourceTimeStamp, ServerTimeStamp, dataValue)"
                    "VALUES('%s', '%s', '%s', '%s')") %
      dbNameTableName % nodeId % dataValue.sourceTimestamp() % dataValue.serverTimestamp() % dataValue);

  bool success = execSQLDirect(sqlQuery);
  if (success)
  {
    Logger::log(Debug, "Database insert data successfully");
  }
  else
  {
    Logger::log(Debug, "Database insert data already exist");
  }

  return true;
} 

bool DbServer::readDataFromDatabase(std::string &dbTableName, OpcUaNodeId &nodeId, OpcUaDateTime startTime, OpcUaDateTime stopTime, OpcUaDataValue::Vec &dataValues)
{
  std::string dbNameTableName = str(
      boost::format("%s.%s") % dbModelConfig_->databaseConfig().databaseName() % dbTableName);

  std::string sqlQuery = str(
      boost::format("SELECT dataValue FROM %s WHERE nodeId = '%s'"
                    "AND sourceTimeStamp BETWEEN '%s' AND '%s'") %
      dbNameTableName % nodeId % startTime % stopTime);

  bool success = execSQLDirect(sqlQuery, dataValues);
  if (success)
  {
    Logger::log(Debug, "Read history data success");
    std::cout << "Read history data success" << std::endl;
  }
  else
  {
    std::cout << "Read history data failed" << std::endl;
  }

  return true;
}

bool DbServer::execSQLDirect(const std::string &sqlQuery)
{
  bool success;

  if (!isConnectedDatabase())
  {
    Logger::log(Error, "Error not connected to the database");
    return false;
  }
  /* execute sql query */
  success = connection.execDirect(sqlQuery);
  if (!success)
  {
    Logger::log(Error, "sql query error");
    return false;
  }

  return true;
}

bool DbServer::execSQLDirect(const std::string &sqlQuery, OpcUaDataValue::Vec &dataValues)
{
  bool success;

  if (!isConnectedDatabase())
  {
    Logger::log(Debug, "Error not connected to the database");
    return false;
  }

  /* execute sql statement */
  success = connection.execDirect(sqlQuery);
  if (!success)
  {
    Logger::log(Error, "sql query error");
    return false;
  }

  /* get result set */
  OpcUaDB::ResultSet &resultSet = connection.resultSet();
  OpcUaDataValue::SPtr statusCode = constructSPtr<OpcUaDataValue>();
  OpcUaDataValue::SPtr header = constructSPtr<OpcUaDataValue>();

  if (!createResultSet(resultSet, statusCode, header, dataValues))
  {
    Logger::log(Error, "An error occoured while creating a result set");
    return false;
  }

  return true;
}

bool DbServer::parseDataValue(std::string &inputString, dataMap &dataStrMap)
{
  return true;
}

bool DbServer::createResultSet(OpcUaDB::ResultSet &resultSet, OpcUaDataValue::SPtr &statusCode, OpcUaDataValue::SPtr &header, OpcUaDataValue::Vec &dataVec)
{
  /* create header */
  OpcUaVariantValue::Vec variantVec1;
  for (uint32_t idx = 0; idx < resultSet.colDescriptionVec_.size(); idx++)
  {
    OpcUaString::SPtr value = constructSPtr<OpcUaString>();
    value->value((char *)resultSet.colDescriptionVec_[idx].colName_);

    std::cout << "Value header is: " << value->value() << std::endl;
    OpcUaVariantValue v;
    v.variant(value);
    variantVec1.push_back(v);
  }
  header->variant()->variant(variantVec1);

  /* create data */
  OpcUaVariantValue::Vec variantVec2;
  std::vector<std::string> valueVec;
  std::vector<std::string> dataMaps;
  for (uint32_t i = 0; i < resultSet.tableData_.size(); i++)
  {
    for (uint32_t j = 0; j < resultSet.colDescriptionVec_.size(); j++)
    {
      OpcUaString::SPtr value = constructSPtr<OpcUaString>();
      value->value(resultSet.tableData_[i][j]);
      valueVec.push_back(value->value());

      /* parse dataValue result */
      std::string valueData = value->value();
      boost::split(dataMaps, valueData, boost::is_any_of("=,"));
      std::vector<std::string>::iterator it;
      it = std::find(dataMaps.begin(), dataMaps.end(), "SourceTime");
      if (it != dataMaps.end())
      {
        std::string timestamp = *++it;
        boost::posix_time::ptime timestamp2(boost::posix_time::from_iso_string(timestamp));

        /* create dataValue */
        OpcUaDataValue::SPtr data = constructSPtr<OpcUaDataValue>();
        data->sourceTimestamp(timestamp2);
        data->serverTimestamp(timestamp2);
        OpcUaString::SPtr str = constructSPtr<OpcUaString>();
        str->value(value->value());
        data->variant()->variant(str);
        data->statusCode(Success);
        dataVec.push_back(data);
      }
      dataMaps.clear();
    }
  }

  /* create status code */
  statusCode->variant()->set(constructSPtr<OpcUaString>("Success"));

  return true;
}

bool DbServer::connectDatabase()
{
  // Check if there was a previous connection, if there was terminates it
  connection.disconnect();

  connection.dnsName(dbModelConfig_->databaseConfig().dsnName());
  connection.userName(dbModelConfig_->databaseConfig().userName());
  connection.password(dbModelConfig_->databaseConfig().password());

  if (connection.connect())
  {
    databaseConected = true;
    return true;
  }
  else
    return false;
}

bool DbServer::isConnectedDatabase()
{
  return databaseConected;
}

bool DbServer::disconnectDatabase()
{
  if (isConnectedDatabase())
  {
    if (connection.disconnect())
      return true;
    else
      return false;
  }
}

} /* namespace OpcUaNikiDB */