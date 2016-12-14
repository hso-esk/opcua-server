/*
 * EddlParserPrintUtil.h
 *
 *  Created on: 13 Dec 2016
 *      Author: osboxes
 */

#ifndef OPCUAEDDL_EDDLPARSERPRINTUTIL_H_
#define OPCUAEDDL_EDDLPARSERPRINTUTIL_H_

#include "EddlParserStruct.h"
#include <boost/fusion/include/adapt_struct.hpp>
#include <iostream>

/**
 * BOOST_FUSION_ADAPT_STRUCT
 *
 * Required by EDDL parser to parse all struct members
 * as a Fusion sequence.
 */
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::string_type_definition,
  (std::string, type)
  (std::string, option)
  (boost::optional<std::vector<OpcUaEddl::pair_type>>, pair_list)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::enumerator_value_def,
  (OpcUaEddl::primitive_type, value)
  (std::string, description)
  (boost::optional<std::string>, help)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::enumerated_type_definition,
  (std::string, type)
  (std::string, size)
  (OpcUaEddl::enum_variant, attributes)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::arithmentic_type_definition,
  (std::string, type)
  (std::string, size)
  (OpcUaEddl::arithmetic_pair_list, arithmetic_list)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::id_definition,
  (std::vector<OpcUaEddl::pair_type>, id_attribute_pairs)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::variable_type_definition,
  (std::string, type)
  (OpcUaEddl::data_type_variant, var_types_pairs)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::variable_definition,
  (std::string, type)
  (std::string, name)
  (OpcUaEddl::variable_variant_list, var_attribute_pairs)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::command_definition,
  (std::string, type)
  (std::string, name)
  (OpcUaEddl::command_variant, cmd_attribute_pairs)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::command_transaction_definition,
  (std::string, type)
  (std::vector<OpcUaEddl::transaction_variant>, cmd_tx_def)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::responseCode_definition,
  (std::string, description)
  (std::string, errorCode_id)
  (std::vector<std::string> , errorCodes)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::method_definition,
  (std::string, type)
  (std::string, name)
  (std::vector<OpcUaEddl::pair_type>, method_attribute_pairs)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaEddl::unit_definition,
  (std::string, type)
  (std::string, name)
  (std::string, dependent)
  (std::vector<std::string>, dependencies)
)


namespace OpcUaEddl
{

/**
 * \brief   Prints to console parsed EDDL data stored in enumerator
 *          variant structure.
 */
struct print_enum_variant : public boost::static_visitor<void>
{
  void operator()(pair_type const & e) const
  {
    std::cout << e.first << " : " << e.second << "\n";
  }

  void operator()(enumerator_value_def const & s) const
  {
      std::cout << "(" << "Value" << " , " << s.value << ")\n";
      std::cout << "(" << "Help" << " , " << s.help << ")\n";
      std::cout << "(" << "Description" << " , " << s.description << ")\n";
  }

  void operator()(std::vector<enumerator_value_def> const & s) const
  {
    for (auto const & item : s) {
      std::cout << "(" << "Enum vector elements" << "(" << item.value << ", "<<
              item.help << ", " << item.description << ")\n";
    }
  }
};

/**
 * \brief   Prints to console parsed EDDL data stored in data
 *          type variant structure. This structure stores the
 *          type of data stored by EDDL Variable.
 */
struct print_data_type_variant : public boost::static_visitor<void>
{

  void operator()(string_type_definition const &s) const
  {
    std::cout << "(" << "Type" << " , " << s.type << ")\n";
    std::cout << "(" << "Option" << " , " << s.option << ")\n";
    if (s.pair_list) {
       for (auto const& item : *(s.pair_list)) {
          std::cout << "(" << item.first << " , " << item.second << ")\n";
        }
    }
  }

  void operator()(enumerated_type_definition const & s) const
  {
    std::cout << "(" << "Type" << " , " << s.type << ")\n";
    std::cout << "(" << "Size" << " , " << s.size << ")\n";
    for (auto const & item : s.attributes) {
      boost::apply_visitor(print_enum_variant(), item);
    }
  }

  void operator()(arithmetic_pair_list const & s) const
  {
    if (s) {
      for (auto const& item : *(s)) {
        std::cout << "(" << item.first << " ) " << "\n";
        std::cout << "(" << item.second << " ) " << "\n";
      }
    }
  }

  void operator()(arithmentic_type_definition const & s) const
  {
    std::cout << "(" << "Type" << " , " << s.type << ")\n";
    std::cout << "(" << "Size" << " , " << s.size << ")\n";
    if (s.arithmetic_list) {
      for (auto const& item : *(s.arithmetic_list)) {
       std::cout << item.first << " : " << item.second << "\n";
      }
    }
  }
};

/**
 * \brief   Prints to console parsed EDDL data stored in transaction
 *          variant structure. This structure stores transaction information
 *          of the EDDL Command construct.
 */
struct print_transaction_variant : public boost::static_visitor<void>
{

  /* print pair of string int data */
  void operator()(pair_type_string_int const & s) const
  {
    std::cout << "(" << s.first << " , ";
    if (s.second) {
      std::cout << s.second << ")\n";
    }
  }

  void operator()(pair_type_string_stringInt const & s) const
  {
    for (auto const& item : s.second) {
      std::cout << "(" << s.first << " , " << item.first << " , "
           << item.second <<  ")\n";
    }
  }

  void operator()(responseCode_definition const & s) const
  {
    std::cout << "(" << "Description" << " , " << s.description << ")\n";
    std::cout << "(" << "Error code id" << " , " << s.errorCode_id << ")\n";
    for (auto const& item : s.errorCodes) {
      std::cout << "(" << "Error code" << " , " << item << ")\n";
    }
  }

  void operator()(std::string const & s) const
  {
    std::cout << "(" << "String" << " , " << s << ")\n";
  }

};

/**
 * \brief   Prints to console parsed EDDL data stored in command
 *          variant structure. This structure stores information
 *          of the EDDL Command construct.
 */
struct print_command_variant : public boost::static_visitor<void>
{
  /* print parsed variable information */
  void operator()(pair_type const &pt) const
  {
    std::cout << "(" << pt.first << " , " << pt.second << ")\n";
  }
  /* print parsed variable type information */
  void operator()(command_transaction_definition const & s) const
  {
    for (auto const& item : s.cmd_tx_def) {
      boost::apply_visitor(print_transaction_variant(), item);
    }
  }
};

/**
 * \brief   Prints to console parsed EDDL data stored in variable
 *          variant structure. This structure stores information
 *          of the EDDL Variable construct.
 */
struct print_variable_vector_variant : public boost::static_visitor<void>
{
  /* print parsed variable information */
  void operator()(pair_type const &s) const
  {
    std::cout << "(" << s.first << " , " << s.second << ")\n";
  }
  /* print parsed variable type information */
  void operator()(variable_type_definition const & s) const
  {
    //std::cout << "(" << "Type" << " , " << s.type << ")\n"; /* comment SELF*/
    boost::apply_visitor(print_data_type_variant(), s.var_types_pairs);
  }
};

/**
 * \brief   Prints to console all parsed EDDL constructs.
 *
 */
struct print_EDDL_construct_data : public boost::static_visitor<void>
{
  /* EDDL identification information */
  void operator()(id_definition const &s) const
  {
    std::cout << "\nEDD identification attrbutes \n";
    for (int i=0; i< s.id_attribute_pairs.size(); i++) {
       std::cout << "(" << s.id_attribute_pairs[i].first << " , "
        << s.id_attribute_pairs[i].second << ")\n";
    }
  }

  void operator()(variable_definition const &s) const
  {
    std::cout << "\nEDDL construct type ---> " << s.type << std::endl;
    std::cout << "Variable identifier ---> " << s.name << std::endl;
    std::cout << "Parsed variable attributes are: \n";
    for (int i = 0; i< s.var_attribute_pairs.size(); i++) {
      boost::apply_visitor(print_variable_vector_variant(), s.var_attribute_pairs[i]);
    }
  }

  void operator()(command_definition const &s) const
  {
    std::cout << "\nEDDL construct type ---> " << s.type << std::endl;
    std::cout << "Command identifier ---> " << s.name << std::endl;
    std::cout <<  "Parsed Command attributes are: \n";
    for (auto const & item : s.cmd_attribute_pairs) {
      boost::apply_visitor(print_command_variant(), item);
    }
  }

  void operator()(method_definition const &s) const
  {
    std::cout << "\nEDDL construct type ---> " << s.type << std::endl;
    std::cout << "Method identifier ---> " << s.name << std::endl;
    std::cout << "Parsed Method attributes are \n";
    for (int i=0; i< s.method_attribute_pairs.size(); i++) {
      std::cout << "(" << s.method_attribute_pairs[i].first << " , "
        << s.method_attribute_pairs[i].second << ")\n";
    }
  }

  void operator()(unit_definition const &s) const
  {
    std::cout << "\nEDDL construct type ---> " << s.type << std::endl;
    std::cout << "Unit identifier ---> " << s.name << std::endl;
    std::cout << "Unit dependent attribute ---> " << s.name << std::endl;
    std::cout << "Unit dependent attributes are :";
    for (int i=0; i< s.dependencies.size(); i++) {
      std::cout << "Unit dependencies are " << s.dependencies[i] << std::endl;
    }
  }
};

/**
 * \brief   Prints all parsed EDDL data items.
 *
 * \param   data    EDDL parsed data to print.
 */
void EddlParser::printEDDLDataItems(eddlParsedData& data)
{
  unsigned int i;
  for (i=0; i< data.size(); i++) {
    boost::apply_visitor(print_EDDL_construct_data(), data[i]);
  }
}

} /* namespace OpcUaEddl */


#endif /* OPCUAEDDL_EDDLPARSERPRINTUTIL_H_ */
