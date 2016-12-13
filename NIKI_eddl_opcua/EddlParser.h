/*
 * EddlParser.h
 *
 *  Created on: 10 Dec 2016
 *      Author: osboxes
 */

#ifndef OPCUAEDDL_EDDLPARSER_H_
#define OPCUAEDDL_EDDLPARSER_H_


/*
 * --- Includes ------------------------------------------------------------- *
 */
#include "OpcUaEddl/EddlParserStruct.h"
#include "OpcUaStackCore/Base/ConfigXml.h"

#include "OpcUaStackServer/AddressSpaceModel/VariableNodeClass.h"
#include "OpcUaStackServer/NodeSet/NodeSetBaseParser.h"
#include "OpcUaStackServer/NodeSet/NodeSetXmlParser.h"
#include "OpcUaStackCore/BuildInTypes/OpcUaNodeId.h"
#include "OpcUaStackServer/AddressSpaceModel/BaseNodeClass.h"
#include "OpcUaStackServer/AddressSpaceModel/Attribute.h"

#include "OpcUaStackServer/InformationModel/InformationModel.h"
#include "OpcUaStackServer/InformationModel/InformationModelAccess.h"
#include "OpcUaStackServer/AddressSpaceModel/Attribute.h"

#include "OpcUaStackServer/ServiceSetApplication/ApplicationService.h"
#include "OpcUaStackServer/Application/Application.h"
#include "OpcUaStackServer/InformationModel/InformationModel.h"

#include "OpcUaSensorInterface/DeviceDataFile.h"
#include "OpcUaStackCore/Utility/IOThread.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/variant.hpp>

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

/*
 * --- MACROS-------------------------------------------------------------- *
 */

#define ENABLE_QI_DEBUG(v) do { v.name(#v); qi::debug(v); } while(0)
#define ENABLE_QI_DEBUG2(v,n) do { v.name(n); qi::debug(v);} while (0)


namespace OpcUaEddl
{

/*
 * --- Class Definition ----------------------------------------------------- *
 */

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
 *          Stores parsed data of all EDDL constructs
 */
typedef std::vector<eddlconstruct> eddlParsedData;


/**
 * \brief   Parse the EDDL file.
 *
 * \param eddlfile   EDDL file to parse.
 * \param data       storage for parsed EDDL data
 *
 * \return  returns true if EDDL parses successfully..
 */
bool parseEDDLfile(const std::string& eddlfile, eddlParsedData& data);

/**
 * \brief   Print successfully parsed EDDL data to console.
 *
 * \param data       Parsed data to print to console.
 *
 * \return  returns true if EDDL parses successfully.
 */
void printEDDLDataItems (eddlParsedData& data);

};

} /* namespace OpcUaEddl */

#endif /* OPCUAEDDL_EDDLPARSER_H_ */
