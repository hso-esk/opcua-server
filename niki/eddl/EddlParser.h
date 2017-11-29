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


#ifndef OPCUAEDDL_EDDLPARSER_H_
#define OPCUAEDDL_EDDLPARSER_H_

#include "EddlParserStruct.h"
#include "OpcUaStackCore/Base/Log.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#define   ENABLE_EDDL_PARSER_DEBUG_OUTPUT    0

#if (ENABLE_EDDL_PARSER_DEBUG_OUTPUT)
#define ENABLE_QI_DEBUG(v) do { v.name(#v); qi::debug(v); } while(0)
#define ENABLE_QI_DEBUG2(v,n) do { v.name(n); qi::debug(v);} while (0)
#else
#define ENABLE_QI_DEBUG(v)
#define ENABLE_QI_DEBUG2(v,n)
#endif

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
