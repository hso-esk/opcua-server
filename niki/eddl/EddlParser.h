/*
 * EddlParser.h
 *
 *  Created on: 10 Dec 2016
 *      Author: osboxes
 */

#ifndef OPCUAEDDL_EDDLPARSER_H_
#define OPCUAEDDL_EDDLPARSER_H_

#include "EddlParserStruct.h"
#include "OpcUaStackCore/Base/Log.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#define ENABLE_QI_DEBUG(v) do { v.name(#v); qi::debug(v); } while(0)
#define ENABLE_QI_DEBUG2(v,n) do { v.name(n); qi::debug(v);} while (0)

namespace OpcUaEddl
{

/**
 * \brief   EDDL parser Class.
 *
 *          The EDDL parser class parses EDDL file and
 *          creates information model from the device
 *          descriptions.
 */
class EddlParser
{

public:

/**
 * \brief   Default Constructor..
 */
EddlParser(void);

/**
 * \brief   Default Destructor.
 */
~EddlParser(void);

/**
 * \brief   Variant structure declaration to store parsed EDDL constructs.
 */
typedef boost::variant< id_definition
  , variable_definition
  , command_definition
  , method_definition
  , unit_definition
  > eddlconstruct;

/**
 * \brief   Vector of variant declaration.
 *
 *    Stores parsed data of all EDDL constructs
 */
typedef std::vector<eddlconstruct> eddlParsedData;

/**
 * \brief   Parse the EDDL file.
 *
 * \param   eddlfile   EDDL file to parse.
 * \param   data       storage for parsed EDDL data
 *
 * \return  returns true if EDDL parses successfully..
 */
bool parseEDDLfile(const std::string& eddlfile, eddlParsedData& data);

/**
 * \brief   Print successfully parsed EDDL data to console.
 *
 * \param   data      Parsed data to print to console.
 * \return  returns true if EDDL parses successfully.
 */
void printEDDLDataItems (eddlParsedData& data);

};

} /* namespace OpcUaEddl */

#endif /* OPCUAEDDL_EDDLPARSER_H_ */
