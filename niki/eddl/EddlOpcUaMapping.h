/*
 * EddlOpcUaMapping.h
 *
 *  Created on: 13 Dec 2016
 *      Author: osboxes
 */

#ifndef OPCUAEDDL_EDDLOPCUAMAPPING_H_
#define OPCUAEDDL_EDDLOPCUAMAPPING_H_

#include "EddlParserStruct.h"

namespace OpcUaEddl
{

struct OPC_ACCESS {
  enum {
      CurrentRead = 1 << 0
    , CurrentWrite = 1 << 1
  };
};

/**
 * \brief   Maps EDDL Variable to OPC UA variable node.
 */
struct variableMapper
    : public boost::static_visitor<void>
{
  variableMapper()
    : accessLevel(0)
    , type(DeviceDataValue::TYPE_INTEGER)
  {}

  void operator()(pair_type const &s)
  {
    if (s.first == "LABEL") {
      displayName = s.second;
    } else if (s.first == "HELP") {
      description = s.second;
    } else if (s.first == "CLASS") {
      // ignored
    } else if (s.first == "HANDLING") {
      std::string handling_str(s.second);

      if (handling_str.find("READ") != std::string::npos) {
        accessLevel |= OPC_ACCESS::CurrentRead;
      }
      if (handling_str.find("WRITE") != std::string::npos) {
        accessLevel |= OPC_ACCESS::CurrentWrite;
      }
    }
  }

  void operator()(variable_type_definition const & s)
  {
    boost::apply_visitor(*this, s.var_types_pairs);
  }

  void operator()(string_type_definition const &s)
  {
    type = DeviceDataValue::TYPE_STRING;
    value.cStr[0] = '\0';

    if (s.pair_list) {
      for (auto const& i : *s.pair_list) {
        if ((i.first == "DEFAULT_VALUE") && (!i.second.empty())) {
          strcpy(value.cStr, i.second.c_str());
        }
      }
    }
  }

  void operator()(enumerated_type_definition const & s)
  {
    // NB: Right now, we just map all enumeration to an INTEGER type
    // No handling of default value yet

    type = DeviceDataValue::TYPE_INTEGER;
    value.i32 = 0;
  }

  void operator()(arithmetic_pair_list const & s)
  {
  }

  void operator()(arithmentic_type_definition const & s)
  {
    if (s.type == "FLOAT") {
      type = DeviceDataValue::TYPE_FLOAT;
      value.f = 0.0;
    } else if (s.type == "DOUBLE") {
      type = DeviceDataValue::TYPE_FLOAT;
      value.f = 0.0;
    } else if (s.type == "INTEGER") {
      type = DeviceDataValue::TYPE_INTEGER;
      value.i32 = 0;
    } else if (s.type == "UNSIGNED_INTEGER") {
      type = DeviceDataValue::TYPE_INTEGER;
      value.i32 = 0;
    }

    if (s.arithmetic_list) {
      for (auto const& i : *s.arithmetic_list) {
        if ((i.first == "DEFAULT_VALUE") && (!i.second.empty())) {
          if (type == DeviceDataValue::TYPE_INTEGER) {
            value.i32 = boost::lexical_cast<int32_t>(i.second);
          } else if (type == DeviceDataValue::TYPE_FLOAT) {
            value.f = boost::lexical_cast<float>(i.second);
          }
        }
      }
    }

  }


  /* variable Node attributes */
  std::string description;
  std::string displayName;
  DeviceDataValue::e_type type;
  DeviceDataValue::u_val value;
  uint8_t accessLevel;
};


/**
* \brief   Maps EDDL constructs to OPC UA nodes.
*
*/
struct mapEddlToOpcUa
    : public boost::static_visitor<void>
{
  mapEddlToOpcUa(uint32_t parentNodeId, uint32_t variableNodeId)
    : parentNodeId_(parentNodeId)
    , variableNodeId_(variableNodeId)
  {
  }

  void operator()(id_definition const &s)
  {
  }

  void operator()(variable_definition const &s)
  {
    variableMapper v;
    for (auto const& attr_pair : s.var_attribute_pairs) {
      boost::apply_visitor(v, attr_pair);
    }

    OpcUaEddlLib::variableNodeCreateInfo info;
    info.parentNodeId = parentNodeId_;
    info.variableNodeId = getNextNodeId();
    info.variableBrowseName = s.name;
    info.variableDescription = v.description;
    info.variableDisplayName = v.displayName;
    info.accessLevel = v.accessLevel;
    info.type = v.type;
    info.value = v.value;

    infos.push_back(info);
  }

  void operator()(command_definition const &s)
  {
  }

  void operator()(method_definition const &s)
  {
  }

  void operator()(unit_definition const &s)
  {
  }

  uint32_t getNextNodeId()
  {
    return variableNodeId_++;
  }

  std::vector<OpcUaEddlLib::variableNodeCreateInfo> infos;

private:

  uint32_t parentNodeId_;
  uint32_t variableNodeId_;

};

} /* namespace OpcUaEddl */


#endif /* OPCUAEDDL_EDDLOPCUAMAPPING_H_ */
