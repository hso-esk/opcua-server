/*
 * EddlParserStruct.h
 *
 *  Created on: 10 Dec 2016
 *      Author: osboxes
 */

#ifndef OPCUAEDDL_EDDLPARSERSTRUCT_H_
#define OPCUAEDDL_EDDLPARSERSTRUCT_H_

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

namespace OpcUaEddl
{

namespace qi = boost::spirit::qi;
typedef std::pair<std::string, std::string> pair_type;
typedef std::pair<std::string, boost::optional<uint32_t>> pair_type_string_int;
typedef std::pair<std::string, boost::optional<uint32_t>> pair_type_stringInt;
typedef std::pair<std::string, std::vector<pair_type_string_int>> pair_type_string_stringInt;
typedef std::pair<std::string, boost::optional<uint32_t>> pair_type_stringInt;
typedef boost::variant<uint32_t, double, std::string> primitive_type;

/**
 * \brief   String parser struct declaration.
 *
 */
struct string_type_definition
{
  std::string type;
  std::string option;
  boost::optional<std::vector<pair_type>> pair_list;
};

/**
 * \brief   Number primitive types struct declaration.
 *
 */
typedef boost::optional<std::vector<pair_type>> arithmetic_pair_list;
struct arithmentic_type_definition
 {
   std::string type;
   std::string size;
   arithmetic_pair_list arithmetic_list;
 };

/**
 * \brief   Enumerator parser struct declaration.
 *
 */
struct enumerator_value_def
{
  primitive_type value;
  std::string description;
  boost::optional<std::string> help;
};
typedef std::vector<boost::variant<pair_type
                      ,std::vector<enumerator_value_def
                      >>> enum_variant;

/**
 * \brief   Enumerator type parser struct declaration.
 *
 */
struct enumerated_type_definition
{
  std::string type;
  std::string size;
  enum_variant attributes;
};

/**
 * \brief   Primitive parsers variant declaration.
 *
 */
typedef boost::variant <string_type_definition
  , enumerated_type_definition
  , arithmentic_type_definition
  > data_type_variant
  ;


/**
 * \brief   EDDL Identification parser struct declaration.
 *
 */
struct id_definition
{
  std::vector<pair_type> id_attribute_pairs;
};

/**
 * \brief   Data type of Variable parser struct declaration.
 *
 */
struct variable_type_definition
{
  std::string type;
  data_type_variant var_types_pairs;
};

/**
 * \brief   Variable definition parser struct declaration.
 *
 */
typedef std::vector<boost::variant<pair_type, variable_type_definition>> variable_variant_list;
struct variable_definition
{
  std::string type;
  std::string name;
  variable_variant_list var_attribute_pairs;
};

/**
 * \brief   EDDL Command response code struct declaration.
 *
 */
struct responseCode_definition
{
  std::string description;
  std::string errorCode_id;
  std::vector<std::string> errorCodes;
};

/**
 * \brief   EDDL Command transaction variant struct declaration.
 *
 */
typedef boost::variant<pair_type_string_stringInt
  , responseCode_definition
  ,std::string
  > transaction_variant;

/**
 * \brief   EDDL Command transaction struct declaration.
 *
 */
struct command_transaction_definition
{
  std::string type;
  std::vector<transaction_variant> cmd_tx_def;
};

/**
 * \brief   EDDL Command variant vector declaration.
 *
 */
typedef std::vector<boost::variant<command_transaction_definition, pair_type>> command_variant;

/**
 * \brief   EDDL Command definition struct declaration.
 *
 */
struct command_definition
{
  std::string type;
  std::string name;
  command_variant cmd_attribute_pairs;
};

/**
 * \brief   EDDL Method definition struct declaration.
 *
 */
struct method_definition
{
  std::string type;
  std::string name;
  std::vector<pair_type> method_attribute_pairs;
};

/**
 * \brief   EDDL Unit definition struct declaration.
 *
 */
struct unit_definition
{
  std::string type;
  std::string name;
  std::string dependent;
  std::vector<std::string> dependencies;
};

} /* namespace OpcUaEddl */



#endif /* OPCUAEDDL_EDDLPARSERSTRUCT_H_ */
