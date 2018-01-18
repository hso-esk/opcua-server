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

#include "OpcUaStackCore/Base/Log.h"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace OpcUaNikiDB
{

/**
 * DbServer ()
 */
DbServer::DbServer(void) :
    dbModelConfig_(nullptr)
{
  Log(Debug, "DbServer::DbServer");
}

/*---------------------------------------------------------------------------*/
/**
 * ~DbServer ()
 */
DbServer::~DbServer(void)
{
  Log(Debug, "DbServer::~DbServer");
}

/*---------------------------------------------------------------------------*/
/**
 * startup ()
 */
bool DbServer::startup(void)
{
  Log(Debug, "DbServer::startup");

  /* create database */
  if (!createDatabase(dbModelConfig_->databaseConfig().databaseName())) {
    return false;
  }

  /* create database table */
  if (!createDatabaseTable(dbModelConfig_->databaseConfig().databaseTableName())) {
    return false;
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * dbModelConfig ()
 */
void DbServer::dbModelConfig(NikiDBModelConfig* dbModelConfig)
{
  Log(Debug, "DbServer::dbModelConfig");

  dbModelConfig_ = dbModelConfig;
}

/*---------------------------------------------------------------------------*/
/**
 * DBServer ()
 */
OpcUaDB::DBServer& DbServer::DBServer(void)
{
  Log(Debug, "DbServer::DBServer");

  return dbServer_;
}
/*---------------------------------------------------------------------------*/
/**
 * shutdown ()
 */
bool DbServer::shutdown(void)
{
  Log(Debug, "DbServer::shutdown");

  dbServer_.shutdown();
  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createDatabase ()
 */
bool DbServer::createDatabase(std::string& dbName)
{
  Log(Debug, "DbServer::createDatabase");

  std::string sqlQuery = "CREATE DATABASE IF NOT EXISTS " + dbName;
  bool success = execSQLDirect(sqlQuery);
  if (success) {
    Log (Debug, "Database created successfully");
  } else {
    Log (Debug, "Database already exist");
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createDatabaseTable ()
 */
bool DbServer::createDatabaseTable(std::string& dbTableName)
{
  Log(Debug, "DbServer::createDatabaseTable");

  std::string dbNameTableName = str(
  boost::format("%s.%s")
    % dbModelConfig_->databaseConfig().databaseName()
    % dbTableName);

  std::string sqlQuery = str(
  boost::format("CREATE TABLE %s (id  INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
              "nodeId varchar(255), sourceTimeStamp varchar(255),"
              "ServerTimeStamp varchar(255),  dataValue varchar(255))")
    % dbNameTableName);

  bool success = execSQLDirect(sqlQuery);
  if (success) {
   Log (Debug, "Database table created successfully");
  } else {
   Log (Debug, "Database table already exist");
  }


  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * writeDataToDatabase ()
 */
bool DbServer::writeDataToDatabase(std::string& dbTableName
    , OpcUaNodeId& nodeId
    , OpcUaDataValue& dataValue)
{
  Log(Debug, "DbServer::writeDataToDatabase");

  std::string dbNameTableName = str(
  boost::format("%s.%s")
     % dbModelConfig_->databaseConfig().databaseName()
     % dbTableName);

  std::string sqlQuery = str(
  boost::format("INSERT INTO %s (nodeId, sourceTimeStamp, ServerTimeStamp, dataValue)"
                "VALUES('%s', '%s', '%s', '%s')")
      % dbNameTableName
      % nodeId
      % dataValue.sourceTimestamp()
      % dataValue.serverTimestamp()
      % dataValue);

  bool success = execSQLDirect(sqlQuery);
  if (success) {
    Log (Debug, "Database insert data successfully");
  } else {
    Log (Debug, "Database insert data already exist");
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * readDataFromDatabase ()
 */
bool DbServer::readDataFromDatabase(std::string& dbTableName
    , OpcUaNodeId& nodeId
    , OpcUaDateTime startTime
    , OpcUaDateTime stopTime
    , OpcUaDataValue::Vec& dataValues)
{
  Log(Debug, "DbServer::readDataFromDatabase");

  std::string dbNameTableName = str(
  boost::format("%s.%s")
    % dbModelConfig_->databaseConfig().databaseName()
    % dbTableName);

  std::string sqlQuery = str(
  boost::format("SELECT dataValue FROM %s WHERE nodeId = '%s'"
    "AND sourceTimeStamp BETWEEN '%s' AND '%s'")
    % dbNameTableName
    % nodeId
    % startTime
    % stopTime);

  bool success = execSQLDirect(sqlQuery, dataValues);
  if (success) {
   Log (Debug, "Read history data success");
   std::cout << "Read history data success" << std::endl;
  } else {
   std::cout << "Read history data failed" << std::endl;
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * execSQLDirect ()
 */
bool DbServer::execSQLDirect(const std::string& sqlQuery)
{
  Log(Debug, "DbServer::execSQLDirect");

  bool success;
  OpcUaDB::Connection connection;

  /* connect to database */
  connection.dnsName(dbModelConfig_->databaseConfig().dsnName());
  connection.userName(dbModelConfig_->databaseConfig().userName());
  connection.password(dbModelConfig_->databaseConfig().password());
  success = connection.connect();
  if (!success) {
    Log (Debug, "Error connecting to database");
    return false;
  }
  /* execute sql query */
  success = connection.execDirect(sqlQuery);
  if (!success) {
    Log (Error, "sql query error");
    connection.disconnect();
    return false;
  }

   /* disconnect from database */
   success = connection.disconnect();
   if (!success) {
 	Log(Error, "Failed to disconnect from sql server");
     return false;
   }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * execSQLDirect ()
 */
bool DbServer::execSQLDirect(const std::string& sqlQuery, OpcUaDataValue::Vec& dataValues)
{
  Log(Debug, "DbServer::execSQLDirect");

  bool success;
  OpcUaDB::Connection connection;

  /* connect to database */
  connection.dnsName(dbModelConfig_->databaseConfig().dsnName());
  connection.userName(dbModelConfig_->databaseConfig().userName());
  connection.password(dbModelConfig_->databaseConfig().password());
  success = connection.connect();
  if (!success) {
   Log (Debug, "Error connecting to database" );
   return false;
  }

  /* execute sql statement */
  success = connection.execDirect(sqlQuery);
  if (!success) {
    Log (Error, "sql query error");
    return false;
  }

  /* get result set */
  OpcUaDB::ResultSet& resultSet = connection.resultSet();
  OpcUaDataValue::SPtr statusCode = constructSPtr<OpcUaDataValue>();
  OpcUaDataValue::SPtr header = constructSPtr<OpcUaDataValue>();

  if (!createResultSet(resultSet, statusCode, header, dataValues)) {
    connection.disconnect();
    return false;
  }

  /* disconnect to database */
  success = connection.disconnect();
  if (!success) {
    return false;
  }

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * parseDataValue ()
 */
bool DbServer::parseDataValue(std::string& inputString, dataMap& dataStrMap)
{
  Log(Debug, "DbServer::parseDataValue");

  return true;
}

/*---------------------------------------------------------------------------*/
/**
 * createResultSet ()
 */
bool DbServer::createResultSet(OpcUaDB::ResultSet& resultSet
    , OpcUaDataValue::SPtr& statusCode
    , OpcUaDataValue::SPtr& header
    , OpcUaDataValue::Vec& dataVec)
{
  Log(Debug, "DbServer::createResultSet");

  /* create header */
  OpcUaVariantValue::Vec variantVec1;
  for (uint32_t idx=0; idx<resultSet.colDescriptionVec_.size(); idx++) {
   OpcUaString::SPtr value = constructSPtr<OpcUaString>();
   value->value((char*)resultSet.colDescriptionVec_[idx].colName_);

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
  for (uint32_t i=0; i<resultSet.tableData_.size(); i++) {
   for (uint32_t j=0; j<resultSet.colDescriptionVec_.size(); j++) {
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
       boost::posix_time::ptime timestamp2 (boost::posix_time::from_iso_string(timestamp));

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

} /* namespace OpcUaNikiDB */


