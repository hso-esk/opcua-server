/*
 * EddlParserStruct.h
 *
 *  Created on: 10 Dec 2016
 *      Author: osboxes
 */

#ifndef OPCUAEDDL_EDDLPARSERSTRUCT_H_
#define OPCUAEDDL_EDDLPARSERSTRUCT_H_

/*
 * --- Includes ------------------------------------------------------------- *
 */

//#include "OpcUaStackServer/AddressSpaceModel/VariableNodeClass.h"
//#include "OpcUaStackServer/NodeSet/NodeSetBaseParser.h"
//#include "OpcUaStackServer/NodeSet/NodeSetXmlParser.h"
//#include "OpcUaStackCore/BuildInTypes/OpcUaNodeId.h"
//#include "OpcUaStackServer/AddressSpaceModel/BaseNodeClass.h"
//#include "OpcUaStackServer/AddressSpaceModel/Attribute.h"
//
//#include "OpcUaStackServer/InformationModel/InformationModel.h"
//#include "OpcUaStackServer/InformationModel/InformationModelAccess.h"
//#include "OpcUaStackServer/AddressSpaceModel/Attribute.h"
//#include "OpcUaStackServer/ServiceSetApplication/ApplicationService.h"
//#include "OpcUaStackServer/Application/Application.h"
//#include "OpcUaStackServer/InformationModel/InformationModel.h"
//
//#include "OpcUaSensorInterface/DeviceDataFile.h"
//#include "OpcUaStackCore/Utility/IOThread.h"

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

//#define ENABLE_QI_DEBUG(v) do { v.name(#v); qi::debug(v); } while(0)
//#define ENABLE_QI_DEBUG2(v,n) do { v.name(n); qi::debug(v);} while (0)


namespace OpcUaEddl
{

namespace qi = boost::spirit::qi;
typedef std::pair<std::string, std::string> pair_type;
typedef std::pair<std::string, boost::optional<uint32_t>> pair_type_string_int;
typedef std::pair<std::string, boost::optional<uint32_t>> pair_type_stringInt;
typedef std::pair<std::string, std::vector<pair_type_string_int>> pair_type_string_stringInt;
typedef std::pair<std::string, boost::optional<uint32_t>> pair_type_stringInt;
typedef boost::variant<uint32_t, double, std::string> primitive_type;


// ======================================================================
    /* Primitive parsers structure declaration */
// ======================================================================
  /* String parser struct declaration */
  struct string_type_definition
  {
    std::string type;
    std::string option;
    boost::optional<std::vector<pair_type>> pair_list;
  };

  /* number parser struct declaration */
  typedef boost::optional<std::vector<pair_type>> arithmetic_pair_list;
  struct arithmentic_type_definition
   {
     std::string type;
     std::string size;
     arithmetic_pair_list arithmetic_list;
   };

  /* enumerator parser struct declaration */
  struct enumerator_value_def
  {
    primitive_type value;
    std::string description;
    boost::optional<std::string> help;
  };
  typedef std::vector<boost::variant<pair_type
                        ,std::vector<enumerator_value_def
                        >>> enum_variant;

  struct enumerated_type_definition
  {
    std::string type;
    std::string size;
    enum_variant attributes;
  };

  /* Primitive parsers variant declaration */
  typedef boost::variant <string_type_definition
    , enumerated_type_definition
    , arithmentic_type_definition
    > data_type_variant
    ;

// ======================================================================
       /* EDDL construct structure declaration */
// ----------------------------------------------------------------------
  /* EDD identification struct declaration */
  struct id_definition
  {
    std::vector<pair_type> id_attribute_pairs;
  };

// ----------------------------------------------------------------------
  /* variable type structure declaration */
  struct variable_type_definition
  {
    std::string type;
    data_type_variant var_types_pairs;
  };

  /* variable and type vector declaration */
  typedef std::vector<boost::variant<pair_type, variable_type_definition>> variable_variant_list;
  struct variable_definition
  {
    std::string type;
    std::string name;
    variable_variant_list var_attribute_pairs;
  };
// ----------------------------------------------------------------------
  /* EDDL command response code struct declaration */
  struct responseCode_definition
  {
    std::string description;
    std::string errorCode_id;
    std::vector<std::string> errorCodes;
  };

  /* EDDL command transaction variant declaration */
  typedef boost::variant<pair_type_string_stringInt
    , responseCode_definition
    ,std::string
    > transaction_variant
    ;

  /* EDDL command transaction structure declaration */
  struct command_transaction_definition
  {
    std::string type;
    std::vector<transaction_variant> cmd_tx_def;
  };
  /* EDDL command attribute and transaction vector declaration */
  typedef std::vector<boost::variant<command_transaction_definition, pair_type>> command_variant;

  /* EDDL command structure declaration */
  struct command_definition
  {
    std::string type;
    std::string name;
    command_variant cmd_attribute_pairs;

    //command_variant cmd_def;
  };
// ----------------------------------------------------------------------
  /* EDDL method structure declaration */
  struct method_definition
  {
    std::string type;
    std::string name;
    std::vector<pair_type> method_attribute_pairs;
  };
// ----------------------------------------------------------------------
  /* EDDL unit structure declaration */
  struct unit_definition
  {
    std::string type;
    std::string name;
    std::string dependent;
    std::vector<std::string> dependencies;
  };
// ----------------------------------------------------------------------
  struct recordMember
  {
    std::string key;
    std::string identifier;
    std::vector<pair_type> recordMemberAttribute;
  };
  struct recordConstruct
  {
    std::string key;
    std::string identifier;
    std::vector<recordMember> recordMembers;
    std::vector<pair_type> recordAttributePairs;
  };

} /* namespace OpcUaEddl */



#endif /* OPCUAEDDL_EDDLPARSERSTRUCT_H_ */
