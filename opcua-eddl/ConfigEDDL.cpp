/*
 * ConfigEDDL.cpp
 *
 *  Created on: 11 Oct 2016
 *      Author: osboxes
 */

/*
 * --- DEFINES ------------------------------------------------------------- *
 */

#define BOOST_SPIRIT_DEBUG

/*
 * --- Includes ------------------------------------------------------------- *
 */
#include "ConfigEDDL.h"
#include "OpcUaStackServer/Server/Server.h"
#include "OpcUaStackCore/Base/os.h"
#include "OpcUaStackCore/Base/Log.h"
#include "OpcUaStackCore/BuildInTypes/BuildInTypes.h"
#include "OpcUaStackCore/Base/ObjectPool.h"
#include "OpcUaStackServer/ServiceSetApplication/ApplicationService.h"
#include "OpcUaStackServer/Application/Application.h"
#include "OpcUaStackCore/ServiceSet/ServiceTransaction.h"

#include <boost/shared_ptr.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/include/qi_omit.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/variant.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/filesystem.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>

#include <algorithm>

// ======================================================================

namespace OpcUaStackCore
{

namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;
namespace fusion = boost::fusion;
namespace repo = boost::spirit::repository;

/**
 * ConfigEDDL()
 */
ConfigEDDL::ConfigEDDL(void)
  : namespaceIndex_(0)
  , currentSensorVal(0)
  , sensorValue()
  , variableNode()
  , objectNode()
  //, informationModel()
{
  Log (Debug, "ConfigEDDL::ConfigEDDL");
}

/**
 * ~ConfigEDDL()
 */
ConfigEDDL::~ConfigEDDL(void)
{
  Log (Debug, "ConfigEDDL::~ConfigEDDL");
}

/**
 * SensorDataFile()
 */
SensorDataFile:: SensorDataFile(std::string name, std::string descr,
                                   DeviceDataValue::e_type type, int access)
  : DeviceDataFile( name, descr, type, access)
{
}

/**
 * eddlVariableNode()
 */
VariableNode::VariableNode()
  : variableNodeData(nullptr)
  , informationModel_(nullptr)
  , objectNode_(nullptr)
  , variableNode_(nullptr)
  , variableTypeNode_(nullptr)
  , namespaceIndex_(0)
{

}

/**
 * ~eddlVariableNode()
 */
VariableNode::~VariableNode()
{
}

/**
 * createNode()
 */
void VariableNode::createNode(void)
{
  /* create instance of information model */
  informationModel_ = OpcUaStackServer::InformationModel::construct();
  OpcUaStackServer::InformationModelAccess informationModelAccess;
  informationModelAccess.informationModel(informationModel_);

  informationModelAccess.add(informationModel_, 0);
#if 0
  /* get root Object node information model */
  OpcUaStackServer::BaseNodeClass::SPtr rootObjectNode;
  rootObjectNode = OpcUaStackServer::ObjectNodeClass::construct();
  OpcUaNodeId rootObjectNodeId;
  rootObjectNodeId.set(OpcUaId_RootFolder);
  //informationModel_->insert(rootObjectNode);
#endif
  /* get object node and check if it is already created */
#if 0
  OpcUaNodeId objectNodeId;
  objectNodeId.set(1800, namespaceIndex_);
  //objectNode_ = informationModel_->find(objectNodeId_);
  //if (objectNode_.get() == nullptr) {
#endif
    /* create object node */
    objectNode_ = OpcUaStackServer::ObjectNodeClass::construct();
    OpcUaNodeId objectNodeId;
    objectNodeId.set(1006, namespaceIndex_);
    OpcUaNodeId rootObjectId;
    rootObjectId.set(85, namespaceIndex_);

    OpcUaQualifiedName browseName("NIKI_SENSOR");
    objectNode_->setBrowseName(browseName);
    OpcUaLocalizedText description("de", "NIKI_SENSOR");
    objectNode_->setDescription(description);
    OpcUaLocalizedText displayName("de", "NIKI_SENSOR");
    objectNode_->setDisplayName(displayName);

    objectNode_->referenceItemMap().add(OpcUaStackServer::ReferenceType::
             ReferenceType_Organizes, false, rootObjectId);

    informationModel_->insert(objectNode_);

#if 0
    /* add object node type reference */
    OpcUaNodeId objectRefId;
    objectRefId.set(OpcUaId_TypesFolder);
    objectNode_->referenceItemMap().add(OpcUaStackServer::ReferenceType::
        ReferenceType_HasTypeDefinition, true, objectRefId);


    /* get root Object node information model */
    OpcUaStackServer::BaseNodeClass::SPtr rootObjectNode;
    rootObjectNode = OpcUaStackServer::ObjectNodeClass::construct();
    OpcUaNodeId rootObjectNodeId;
    rootObjectNodeId.set(OpcUaId_ObjectsFolder);
    informationModel_->insert(rootObjectNode);

#endif

#if 0
  OpcUaNodeId objectNodeId;
  objectNodeId.set(1800, namespaceIndex_);
  baseNodeClass_->setNodeId(objectNodeId);
#endif

#if 0
  /* add variable type node */
  variableTypeNode_ = OpcUaStackServer::VariableTypeNodeClass::construct();
  OpcUaNodeId typeNodeId;
  typeNodeId.set(1700, namespaceIndex_);
  variableTypeNode_->setNodeId(typeNodeId);
  informationModel_->insert(variableTypeNode_);
#endif

  /* add variable node to the information model */
  variableNode_ = OpcUaStackServer::VariableNodeClass::construct();
  OpcUaNodeId varNodeId;
  varNodeId.set(1005, namespaceIndex_);
  variableNode_->setNodeId(varNodeId);

  OpcUaQualifiedName varbrowseName("NIKI_browseName", namespaceIndex_);
  variableNode_->setBrowseName(varbrowseName);
  OpcUaLocalizedText vardescription("de", "NIKI_description");
  variableNode_->setDescription(vardescription);
  OpcUaLocalizedText  vardisplayName("de", "NIKI_displayName");
  variableNode_->setDisplayName(vardisplayName);

   /* create references to object node */
   variableNode_->referenceItemMap().add(OpcUaStackServer::ReferenceType::
       ReferenceType_HasComponent, false, objectNodeId);

   objectNode_->referenceItemMap().add(OpcUaStackServer::ReferenceType::
       ReferenceType_HasComponent, true, varNodeId);

  informationModel_->insert(variableNode_);

  /* add node to the OPC UA Object node */
#if 0
  OpcUaStackServer::BaseNodeClass::SPtr baseObjectNode;
  OpcUaNodeId baseObjectNodeId;
  baseObjectNodeId.set(OpcUaId_ObjectsFolder);
  baseObjectNode = informationModel_->find(baseObjectNodeId);
  if (baseObjectNode.get()) {
    baseObjectNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
        ReferenceType_Organizes, true, objectNodeId);
  }
#endif
}

/**
 * ~SensorDataFile()
 */
SensorDataFile::~SensorDataFile(void)
{
  delete instance;
}

/**
 * IDGenerator()
 */
IDGenerator::IDGenerator()
  : id_(1000)
{
}

/**
 * ~IDGenerator()
 */
IDGenerator::~IDGenerator()
{
}

/**
 * initialise static variables
 */
SensorDataFile* SensorDataFile::instance = nullptr;
uint32_t SensorDataFile::obsval = 0;

/**
 * getInstatnce()
 */
SensorDataFile* SensorDataFile::getInstatnce(void)
{
  if (instance == nullptr) {
    instance = new SensorDataFile("NikiSensor_Temperature_Sensor",
                     "This sensor measures temperature",
                     DeviceDataValue::TYPE_INTEGER,
                     (DeviceData::ACCESS_READ | DeviceData::ACCESS_WRITE
                     | DeviceData::ACCESS_OBSERVE));
  }
  return instance;
}

/**
 * getupdateValue()
 */
void SensorDataFile::getupdateValue(const DeviceDataValue* p_val, void* p_param)
{

  DeviceDataValue* p_observeVal = (DeviceDataValue*)p_param;
  p_observeVal->setVal(p_val->getVal().i32);

  obsval = p_observeVal->getVal().i32;
  std::cout << "Observed sensor value: " << obsval << std::endl;
#if 0
  ConfigEDDL instance;

  OpcUaDataValue::SPtr val;
  val = instance.createSensorDataValue();
  val->variant()->variant((uint32_t)obsval);

  //instance.setObservedDataValue(val);
  OpcUaStackServer::InformationModel::SPtr informationModel = constructSPtr<OpcUaStackServer::InformationModel>();
  informationModel->setValue(1000
      , AttributeId_Value
      ,*val);

  OpcUaStackServer::VariableNodeClass::SPtr variableNode;
  variableNode = OpcUaStackServer::VariableNodeClass::construct();
  OpcUaNodeId nodeId;
  nodeId.set(1000, 0);
  variableNode->setNodeId(nodeId);
  variableNode->setValue(*val);
  std::cout << "observed temp: " << obsval << std::endl;
#endif
}

/**
 * getObservedValue()
 */
int32_t SensorDataFile::getObservedValue(void)
{
  return obsval;
}

/**
 * getSensorValue()
 */
int32_t SensorDataFile::getSensorValue(void)
{
  const DeviceDataValue *p_val = NULL;
  SensorDataFile* nikiSensor = SensorDataFile::getInstatnce();
  if (nikiSensor) {
    p_val = nikiSensor->getVal();
  }
  std::cout << "read temp: " << p_val->getVal().i32 << "\n";
  return p_val->getVal().i32;
}

/**
 * node id instance()
 */
IDGenerator* IDGenerator::instance()
{
  static IDGenerator *idInstance;
  if (!idInstance) {
    idInstance = new IDGenerator ();
  }
  return idInstance;
}

/**
 * node id instance()
 */
OpcUaStackCore::OpcUaInt32 IDGenerator::getNextId()
{
  return id_++;
}

} /* namespace OpcUaStackCore */


/**
 * BOOST_FUSION_ADAPT_STRUCT
 *
 * Required by EDDL parser to parse all struct members
 * as a Fusion sequence.
 */
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::string_type_definition,
  (std::string, type)
  (std::string, option)
  (boost::optional<std::vector<OpcUaStackCore::pair_type>>, pair_list)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::enumerator_value_def,
  (OpcUaStackCore::primitive_type, value)
  (std::string, description)
  (boost::optional<std::string>, help)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::enumerated_type_definition,
  (std::string, type)
  (std::string, size)
  (OpcUaStackCore::enum_variant, attributes)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::arithmentic_type_definition,
  (std::string, type)
  (std::string, size)
  (OpcUaStackCore::arithmetic_pair_list, arithmetic_list)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::id_definition,
  (std::vector<OpcUaStackCore::pair_type>, id_attribute_pairs)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::variable_type_definition,
  (std::string, type)
  (OpcUaStackCore::data_type_variant, var_types_pairs)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::variable_definition,
  (std::string, type)
  (std::string, name)
  (OpcUaStackCore::variable_variant_list, var_attribute_pairs)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::command_definition,
  (std::string, type)
  (std::string, name)
  (OpcUaStackCore::command_variant, cmd_attribute_pairs)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::command_transaction_definition,
  (std::string, type)
  (std::vector<OpcUaStackCore::transaction_variant>, cmd_tx_def)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::responseCode_definition,
  (std::string, description)
  (std::string, errorCode_id)
  (std::vector<std::string> , errorCodes)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::method_definition,
  (std::string, type)
  (std::string, name)
  (std::vector<OpcUaStackCore::pair_type>, method_attribute_pairs)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::unit_definition,
  (std::string, type)
  (std::string, name)
  (std::string, dependent)
  (std::vector<std::string>, dependencies)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::recordMember,
  (std::string, key)
  (std::string, identifier)
  (std::vector<OpcUaStackCore::pair_type>, recordMemberAttribute)
)
BOOST_FUSION_ADAPT_STRUCT(
  OpcUaStackCore::recordConstruct,
  (std::string, key)
  (std::string, identifier)
  (std::vector<OpcUaStackCore::recordMember>, recordMembers)
  (std::vector<OpcUaStackCore::pair_type>, recordAttributePairs)
)


namespace OpcUaStackCore
{

/**
 * enumerated types visitor class
 *
 * Prints the enumeration data types of EDDL VARIABLE constructs
 */
struct print_enum_variant : public boost::static_visitor<void>
{
  /* print enum_variant data */
  void operator()(pair_type const & e) const
  {
    std::cout << e.first << " : " << e.second << "\n";
  }
  /* print value of enumerator value def  */
  void operator()(enumerator_value_def const & s) const
  {
      std::cout << "(" << "Value" << " , " << s.value << ")\n";
      std::cout << "(" << "Help" << " , " << s.help << ")\n";
      std::cout << "(" << "Description" << " , " << s.description << ")\n";
  }
  /* print vector of enumerato_value_def */
  void operator()(std::vector<enumerator_value_def> const & s) const
  {
    for (auto const & item : s) {
      std::cout << "(" << "Enum vector elements" << "(" << item.value << ", "<<
              item.help << ", " << item.description << ")\n";
    }
  }
};

/**
 * data type visitor class
 *
 * Prints data types of EDDL VARIABLE construct
 */
struct print_data_type_variant : public boost::static_visitor<void>
{
  /* print string_type_definition data */
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
  /* print enumerated type def */
  void operator()(enumerated_type_definition const & s) const
  {
    std::cout << "(" << "Type" << " , " << s.type << ")\n";
    std::cout << "(" << "Size" << " , " << s.size << ")\n";
    for (auto const & item : s.attributes) {
      boost::apply_visitor(print_enum_variant(), item);
    }
  }
  /* print arithmetic_list data */
  void operator()(arithmetic_pair_list const & s) const
  {
    if (s) {
      for (auto const& item : *(s)) {
        std::cout << "(" << item.first << " ) " << "\n";
        std::cout << "(" << item.second << " ) " << "\n";
      }
    }
  }
  /* print arithmetic_type_definition data */
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
 * Command transaction visitor class
 *
 * Prints transactions of EDDL COMMAND construct
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
  /* print pair_type data */
  void operator()(pair_type_string_stringInt const & s) const
  {
    for (auto const& item : s.second) {
      std::cout << "(" << s.first << " , " << item.first << " , "
           << item.second <<  ")\n";
    }
  }
  /* print variable type data */
  void operator()(responseCode_definition const & s) const
  {
    std::cout << "(" << "Description" << " , " << s.description << ")\n";
    std::cout << "(" << "Error code id" << " , " << s.errorCode_id << ")\n";
    for (auto const& item : s.errorCodes) {
      std::cout << "(" << "Error code" << " , " << item << ")\n";
    }
  }
  /* print a string */
  void operator()(std::string const & s) const
  {
    std::cout << "(" << "String" << " , " << s << ")\n";
  }

};

/**
 * EDDL COMMAND visitor class
 *
 * Prints elements of EDDL COMMAND construct
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
 * EDDL VARIABLE visitor class
 *
 * Prints elements of EDDL VARIABLE construct
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
    std::cout << "(" << "Type" << " , " << s.type << ")\n";
    boost::apply_visitor(print_data_type_variant(), s.var_types_pairs);
  }
};

/**
 * Parsed EDDL constructs visitor class
 *
 * Prints all elements of parsed EDDL constructs
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

  /* EDDL VARIABLE elements */
  void operator()(variable_definition const &s) const
  {
    std::cout << "\nEDDL construct type ---> " << s.type << std::endl;
    std::cout << "Variable identifier ---> " << s.name << std::endl;
    std::cout << "Parsed variable attributes are: \n";
    for (int i = 0; i< s.var_attribute_pairs.size(); i++) {
      boost::apply_visitor(print_variable_vector_variant(), s.var_attribute_pairs[i]);
    }
  }

  /* EDDL COMMAND elements */
  void operator()(command_definition const &s) const
  {
    std::cout << "\nEDDL construct type ---> " << s.type << std::endl;
    std::cout << "Command identifier ---> " << s.name << std::endl;
    std::cout <<  "Parsed Command attributes are: \n";
    for (auto const & item : s.cmd_attribute_pairs) {
      boost::apply_visitor(print_command_variant(), item);
    }
  }

  /* EDDL METHOD elements */
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

  /* EDDL UNIT elements */
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

/*
* printEDDLDataItems()
*/
void ConfigEDDL::printEDDLDataItems(eddlParsedData& data)
{
  unsigned int i;
  for (i=0; i< data.size(); i++) {
    boost::apply_visitor(print_EDDL_construct_data(), data[i]);
  }
}

// ======================================================================
//
//  Description:  Visitor class to map EDDL keywords to OPC UA nodes.
//
// ----------------------------------------------------------------------
/* map EDDL variable keyword to OPC UA variable node */
struct map_eddl_opcUa : public boost::static_visitor<void>
{
  void operator()(variable_definition const& s) const
  {

  }

};

/**
 * Skipper grammar parser
 *
 * Parses comments, whitespaces, preprocessor directives
 *
 */
template <typename Iterator>
struct skipper
  : qi::grammar<Iterator>
{
  skipper()
    : skipper::base_type(start)
  {

    /* skip all whitespaces, single and block comments */
    start %= ascii::space
      | (qi::lit("#") >> *(qi::char_-qi::eol) >> qi::eol)
      | repo::confix("//", qi::eol)[*(qi::char_ - qi::eol)]
      | repo::confix("/*", "*/")[*(qi::char_ - "*/")]
      ;
  }

private:
  qi::rule<Iterator> start;
};

/*
 * --- Primitive parsers -------------------------------------------------------------- *
 */

/**
 * String parser
 *
 * Parses quoted strings.
 *
 */
template <typename Iterator>
struct string_parser
  : qi::grammar<Iterator, std::string()>
{
  string_parser()
    : string_parser::base_type(string)
  {
    string
      %=  qi::lit('"')
      >> *(~ascii::char_('"'))
      >>  qi::lit('"');
  }

private:
  qi::rule<Iterator, std::string()> string;
};

/**
 * Integer parser
 *
 * Parses integers (binary, octal, decimal and hex).
 *
 */
template <typename Iterator>
struct integer_parser
  : qi::grammar<Iterator, uint32_t()>
{
integer_parser()
  : integer_parser::base_type(integer)
{
  binary_number %=  qi::lit('0')
    >> (qi::lit('b') | qi::lit('B')) >> binary_value;

  octal_number %= &(ascii::char_('0')) >> octal_value;

  dec_number %= &(ascii::char_("1-9")) >> dec_value;

  hex_number %=  qi::lit('0')
    >> (qi::lit('x') | qi::lit('X')) >> hex_value;

  integer %= (binary_number
    |   hex_number
    |   octal_number
    |   dec_number)
    >> &(~ascii::char_('.') | qi::eoi);
  }

private:

  /* integer parser rules */
  qi::uint_parser<uint32_t, 2> binary_value;
  qi::uint_parser<uint32_t, 8> octal_value;
  qi::uint_parser<uint32_t, 10> dec_value;
  qi::uint_parser<uint32_t, 16> hex_value;
  qi::rule<Iterator, uint32_t()> binary_number;
  qi::rule<Iterator, uint32_t()> octal_number;
  qi::rule<Iterator, uint32_t()> dec_number;
  qi::rule<Iterator, uint32_t()> hex_number;
  qi::rule<Iterator, uint32_t()> integer;
};

/**
 * String specifier type parser
 *
 * Parses different strings (e.g. ASCII, BITSTRING).
 *
 */
template <typename Iterator>
struct string_type_parser
  : qi::grammar<Iterator, string_type_definition(), skipper<Iterator>>
{
  typedef skipper<Iterator> skipper_t;

  string_type_parser()
    : string_type_parser::base_type(start)
  {
    string_option_type %= qi::string("ASCII")
      | qi::string("BITSTRING")
      | qi::string("EUC")
      | qi::string("PACKED_ASCII")
      | qi::string("PASSWORD")
      | qi::string("VISIBLE")
      | qi::string("OCTET");

    string_default_value %= qi::string("DEFAULT_VALUE")
      >> parse_string
      >> qi::lit(';');

    string_initial_value %= qi::string("INITIAL_VALUE")
      >> parse_string
      >> qi::lit(';');

    string_option %= string_default_value
      | string_initial_value;

    string_option_list %= *string_option;

    string_option_size %= qi::lit('(')
      >> parse_integer
      >> qi::lit(')');

    start %= string_option_type
      >> qi::raw[string_option_size]
      >> (qi::lit(';') | (qi::lit('{')
      >> string_option_list
      >> qi::lit('}')));
  }

private:

  /* instantiate primitive parsers */
  integer_parser<Iterator> parse_integer;
  string_parser<Iterator> parse_string;

  /* string specifier type parser rules */
  qi::rule<Iterator, void(), skipper_t> string_option_size;
  qi::rule<Iterator, std::string(), skipper_t> string_option_type;
  qi::rule<Iterator, pair_type(), skipper_t> string_option;
  qi::rule<Iterator, std::vector<pair_type>(), skipper_t> string_option_list;
  qi::rule<Iterator, pair_type(), skipper_t> string_default_value;
  qi::rule<Iterator, pair_type(), skipper_t> string_initial_value;

  /* start rule */
  qi::rule<Iterator, string_type_definition(), skipper_t> start;
};

/**
 * Identifier parser
 *
 * Parses identifier of EDDL constructs.
 *
 */
template <typename Iterator>
struct identifier_parser
  : qi::grammar<Iterator, std::string()>
{
  identifier_parser()
    : identifier_parser::base_type(identifier)
  {
    identifier %= ascii::char_("A-Za-z")
      >> *ascii::char_("A-Za-z0-9_");
  }
private:
  qi::rule<Iterator, std::string()> identifier;
};

/**
 * Real number parser
 *
 * Parses real numbers (e.g. double, float).
 *
 */
template <typename Iterator>
struct real_parser
  : qi::grammar<Iterator, double()>
{
  real_parser()
    : real_parser::base_type(start)
  {
    start %= qi::double_ | qi::float_;
  }

private:
  qi::rule<Iterator, double()> start;
};

/**
 * Expression parser
 *
 * Parses expressions.
 *
 */
template <typename Iterator>
struct expr_parser
  : qi::grammar<Iterator, void(), skipper<Iterator>>
{
  typedef skipper<Iterator> skipper_t;

  expr_parser()
    : expr_parser::base_type(start)
  {
    primary_expr %= parse_integer
      | parse_real
      | parse_string
      | (qi::lit('(') >> start >> qi::lit(')'));

    start %= primary_expr % qi::lit(',');
  }

private:

  /* instantiate primitive parsers */
  integer_parser<Iterator> parse_integer;
  real_parser<Iterator> parse_real;
  string_parser<Iterator> parse_string;

  /* expression parser rule */
  qi::rule<Iterator, void(), skipper_t> primary_expr;

  /* start rule */
  qi::rule<Iterator, void(), skipper_t> start;
};

/**
 * Expression specifier parser
 *
 * Parses all expressions types.
 * So far, only expr_parser is covered
 *
 */
template <typename Iterator>
struct expr_specifier_parser
  : qi::grammar<Iterator, void(), skipper<Iterator>>
{
  typedef skipper<Iterator> skipper_t;

  expr_specifier_parser()
    : expr_specifier_parser::base_type(start)
  {
    start %= (parse_expression >> qi::lit(';'));
  }

private:

  /* instantiate expression parser */
  expr_parser<Iterator> parse_expression;

  qi::rule<Iterator, void(), skipper_t> start;
};

/**
 * Arithmetic specifier type parser
 *
 * Parses all arithmetic types.
 *
 */
template <typename Iterator>
struct arithmetic_type_parser
  : qi::grammar<Iterator, arithmentic_type_definition(), skipper<Iterator>>
{
  typedef skipper<Iterator> skipper_t;

  arithmetic_type_parser()
    : arithmetic_type_parser::base_type(start)
  {
    arithmetic_default_value %= qi::string("DEFAULT_VALUE") >> qi::raw[parse_expression];
    arithmetic_initial_value %= qi::string("INITIAL_VALUE") >> qi::raw[parse_expression];

    maximum_value %= qi::string("MAX_VALUE") >> qi::raw[parse_expression];
    maximum_value_n %= qi::string("MAX_VALUE_n") >> qi::raw[parse_expression];
    minimum_value %= qi::string("MIN_VALUE") >> qi::raw[parse_expression];
    minimum_value_n %= qi::string("MIN_VALUE_n") >> qi::raw[parse_expression];

    arithmetic_option %= arithmetic_default_value
      | arithmetic_initial_value
      | maximum_value
      | maximum_value_n
      | minimum_value
      | minimum_value_n;

    arithmetic_option_list %= +arithmetic_option;
    arithmetic_options %= qi::lit(';')
      | (qi::lit('{') >> arithmetic_option_list >> qi::lit('}'));
    arithmetic_option_size %= -(qi::lit('(') >> parse_integer >> qi::lit(')'));

    start %= (qi::string("INTEGER") >> qi::raw[arithmetic_option_size] >> arithmetic_options)
      | (qi::string("UNSIGNED_INTEGER") >> qi::raw[arithmetic_option_size] >> arithmetic_options)
      | (qi::string("DOUBLE")>> qi::attr("") >> arithmetic_options)
      | (qi::string("FLOAT")>> qi::attr("") >> arithmetic_options);
  }

private:

  /* initialise primitive parsers */
  expr_specifier_parser<Iterator> parse_expression;
  integer_parser<Iterator> parse_integer;

  /* arithmetic parser rules */
  qi::rule<Iterator, void(), skipper_t> arithmetic_option_size;
  qi::rule<Iterator, boost::optional<std::vector<pair_type>>(), skipper_t> arithmetic_options;
  qi::rule<Iterator, std::vector<pair_type>(), skipper_t> arithmetic_option_list;
  qi::rule<Iterator, pair_type(), skipper_t> arithmetic_option;
  qi::rule<Iterator, pair_type(), skipper_t> arithmetic_default_value;
  qi::rule<Iterator, pair_type(), skipper_t> arithmetic_initial_value;
  qi::rule<Iterator, pair_type(), skipper_t> maximum_value;
  qi::rule<Iterator, pair_type(), skipper_t> maximum_value_n;
  qi::rule<Iterator, pair_type(), skipper_t> minimum_value;
  qi::rule<Iterator, pair_type(), skipper_t> minimum_value_n;

  qi::rule<Iterator, arithmentic_type_definition(), skipper_t> start;
};

/**
 * Enumerator specifier type parser
 *
 * Parses all enumerator types.
 *
 */
template <typename Iterator>
struct enumerated_type_parser
  : qi::grammar<Iterator, enumerated_type_definition(), skipper<Iterator>>
{
  typedef skipper<Iterator> skipper_t;

  enumerated_type_parser()
    : enumerated_type_parser::base_type(start)
  {
    arithmetic_default_value %= qi::string("DEFAULT_VALUE") >> qi::raw[expr_specifier];;
    arithmetic_initial_value %= qi::string("INITIAL_VALUE") >> qi::raw[expr_specifier];;

    description_string %= parse_string;
    help_string %= parse_string;

    enumerator_value %= parse_integer
      | parse_real
      | parse_string;

    enumerator %= qi::lit('{')
      >> enumerator_value
      >> qi::lit(',') >> description_string
      >> -(qi::lit(',') >> help_string)
      >> qi::lit('}');

    enumerator_list %= enumerator % qi::lit(',');
    enumerators_specifier %= enumerator_list;

    enumerated_option %= arithmetic_default_value
      | arithmetic_initial_value
      | enumerators_specifier
      ;
    enumerated_option_list %= +enumerated_option;

    enumerated_option_size %= -(qi::lit('(')
      >> parse_integer
      >> qi::lit(')'));

    start %= qi::string("ENUMERATED")
      >> qi::raw[enumerated_option_size]
      >> qi::lit("{")
      >> enumerated_option_list
      >> qi::lit("}");
    init_debug();
  }

  void init_debug()
  {
    ENABLE_QI_DEBUG(enumerated_option_size);
    ENABLE_QI_DEBUG(enumerated_option_list);
    ENABLE_QI_DEBUG(enumerated_option);
    ENABLE_QI_DEBUG(enumerator_list);
    ENABLE_QI_DEBUG(enumerator);
    ENABLE_QI_DEBUG(enumerator_value);
    ENABLE_QI_DEBUG(description_string);
    ENABLE_QI_DEBUG(help_string);
    ENABLE_QI_DEBUG(arithmetic_default_value);
    ENABLE_QI_DEBUG(arithmetic_initial_value);
    ENABLE_QI_DEBUG(enumerators_specifier);
    ENABLE_QI_DEBUG(string);

    ENABLE_QI_DEBUG2(start, "enumerated_type_parser");
  }

private:

  /* instantiate primitive parsers */
  integer_parser<Iterator> parse_integer;
  real_parser<Iterator> parse_real;
  string_parser<Iterator> parse_string;
  identifier_parser<Iterator> parse_identifier;
  expr_specifier_parser<Iterator> expr_specifier;

  /* enumerated type parser rules */
  qi::rule<Iterator, void(), skipper_t> enumerated_option_size;
  qi::rule<Iterator, enum_variant(), skipper_t> enumerated_option_list;
  qi::rule<Iterator, enum_variant(), skipper_t> enumerated_option;
  qi::rule<Iterator, std::vector<enumerator_value_def>(), skipper_t> enumerator_list;
  qi::rule<Iterator, enumerator_value_def(), skipper_t> enumerator;
  qi::rule<Iterator, primitive_type(), skipper_t> enumerator_value;
  qi::rule<Iterator, std::string(), skipper_t> description_string;
  qi::rule<Iterator, std::string(), skipper_t> help_string;
  qi::rule<Iterator, std::string(), skipper_t> string;
  qi::rule<Iterator, pair_type(), skipper_t> arithmetic_default_value;
  qi::rule<Iterator, pair_type(), skipper_t> arithmetic_initial_value;
  qi::rule<Iterator, std::vector<enumerator_value_def>(), skipper_t> enumerators_specifier;

  /* start rule */
  qi::rule<Iterator, enumerated_type_definition(), skipper_t> start;
};

/**
 * VARIABLE TYPE parser
 *
 * Parses the type definition of the EDDL VARIABE construct.
 *
 */
template <typename Iterator>
struct variable_type_parser
: qi::grammar<Iterator, variable_type_definition(), skipper<Iterator> >
{
  typedef skipper<Iterator> skipper_t;

  variable_type_parser()
    : variable_type_parser::base_type(start)
  {
    def_type %= qi::string("TYPE");

    /* match variable type and invoke assoicated parser */
    def_var_type %= (&qi::string("DOUBLE") >> parse_number)
      | (&qi::string("FLOAT") >> parse_number)
      | (&qi::string("INTEGER") >> parse_number)
      | (&qi::string("UNSIGNED_INTEGER") >> parse_number)
      | (&qi::string("ENUMERATED") >> parse_enum)
      | (&qi::string("ASCII") >> parse_string)
      | (&qi::string("EUC") >> parse_string)
      | (&qi::string("OCTET") >> parse_string)
      | (&qi::string("PACKED_ASCII") >> parse_string)
      | (&qi::string("PASSWORD") >> parse_string)
      | (&qi::string("VISIBLE") >> parse_string)
      ;

    start %= def_type
      >> def_var_type;

    init_debug();
  }

  void init_debug()
  {
    ENABLE_QI_DEBUG(def_type);
    ENABLE_QI_DEBUG(def_var_type);

    ENABLE_QI_DEBUG2(start, "variable_type_parser");
  }

private:

  /* instantiate parsers */
  enumerated_type_parser<Iterator> parse_enum;
  arithmetic_type_parser<Iterator> parse_number;
  string_type_parser<Iterator> parse_string;

  /* variable type parser rules */
  qi::rule<Iterator, std::string(), skipper_t> def_type;
  qi::rule<Iterator, data_type_variant(), skipper_t> def_var_type;

  /* start rule */
  qi::rule<Iterator, variable_type_definition(), skipper_t> start;
};

/**
 * EDDL VARIABLE parser
 *
 * Parses the EDDL VARIABE construct.
 *
 */
template <typename Iterator>
struct variable_parser
  : qi::grammar<Iterator, variable_definition(), skipper<Iterator> >
{
  typedef skipper<Iterator> skipper_t;

  variable_parser()
    : variable_parser::base_type(start)
  {
    def_type %= qi::string("VARIABLE");

    def_name %= identifier;

    var_attr_pair %= classtype
     | label
     | handling
     | help;

    classtype %= qi::string("CLASS")
      >> qi::raw[(qi::string("CONTAINED") | qi::string("DYNAMIC"))
      % qi::lit('&')];

    label %= qi::string("LABEL")
      >> (identifier | qi::raw[(qi::lit('[')
      >> identifier
      >> qi::lit(']'))]);

    handling %= qi::string("HANDLING")
      >> qi::raw[(qi::string("READ")
      | qi::string("WRITE"))
      % qi::lit('&')];

    help %= qi::string("HELP")
      >> (identifier | qi::raw[(qi::lit('[')
      >> identifier
      >> qi::lit(']'))]);

    var_attr_pair_list %= +(var_type | ((var_attr_pair) >> ';'));

    start %= def_type
      >> def_name
      >> qi::lit("{")
      >> var_attr_pair_list
      >> qi::lit("}");

    init_debug();
  }

  void init_debug()
  {
    ENABLE_QI_DEBUG(def_type);
    ENABLE_QI_DEBUG(def_name);
    ENABLE_QI_DEBUG(classtype);
    ENABLE_QI_DEBUG(label);
    ENABLE_QI_DEBUG(handling);
    ENABLE_QI_DEBUG(help);
    ENABLE_QI_DEBUG(var_attr_pair);
    ENABLE_QI_DEBUG(var_attr_pair_list);

    ENABLE_QI_DEBUG2(start, "variable_parser");
  }

  private:

    /* instantiate primitive parsers */
    variable_type_parser<Iterator> var_type;
    identifier_parser<Iterator> identifier;

    /* variable parser rules */
    qi::rule<Iterator, std::string(), skipper_t> def_type;
    qi::rule<Iterator, std::string(), skipper_t> def_name;
    qi::rule<Iterator, pair_type(), skipper_t> classtype;
    qi::rule<Iterator, pair_type(), skipper_t> label;
    qi::rule<Iterator, pair_type(), skipper_t> handling;
    qi::rule<Iterator, pair_type(), skipper_t> help;
    qi::rule<Iterator, pair_type(), skipper_t> var_attr_pair;
    qi::rule<Iterator, variable_variant_list(), skipper_t> var_attr_pair_list;

    /* start rule */
    qi::rule<Iterator, variable_definition(), skipper_t> start;
};

/**
 * EDDL METHOD parser
 *
 * Parses the EDDL METHOD construct.
 *
 */
template <typename Iterator>
struct method_parser
  : qi::grammar<Iterator, method_definition(), skipper<Iterator> >
{
  typedef skipper<Iterator> skipper_t;

  method_parser()
    : method_parser::base_type(start)
  {
    def_type %= qi::string("METHOD");

    def_name %= parse_identifier;

    method_attr_pair %=  help
      | label
      | method_access
      | definition
      | validity
      | method_class
      ;

    help %= qi::string("HELP")
      >> (parse_identifier | qi::raw[(qi::lit('[')
      >> parse_identifier >> qi::lit(']'))]);

    label %= qi::string("LABEL")
      >> (parse_identifier | qi::raw[(qi::lit('[')
      >> parse_identifier >> qi::lit(']'))]);

    method_access %= qi::string("ACCESS")
      >> (qi::string("OFFLINE") | qi::string("ONLINE"));

    definition %= qi::lit("DEFINITION")
      >> qi::lit("{")
      >> *(~ascii::char_('}')) // method with with no implementation
      >> qi::lit("}");

    validity %= qi::string("VALIDITY")
      >> qi::raw[qi::bool_];

    method_class %= qi::string("CLASS")
      >> qi::raw[(qi::string("CONTAINED") | qi::string("DYNAMIC"))
      % qi::lit('&')];

    method_attr_pair_list %= +(
      // Fake an attribute for the definition, so we don't get an empty pair
      (definition >> qi::attr("DEFINITION") >> qi::attr(""))
      | (method_attr_pair >> ';'));

    start %= def_type
      >> def_name
      >> qi::lit("{")
      >> method_attr_pair_list
      >> qi::lit("}");

    init_debug();
  }

  void init_debug()
  {

    ENABLE_QI_DEBUG(def_type);
    ENABLE_QI_DEBUG(def_name);
    ENABLE_QI_DEBUG(method_attr_pair);
    ENABLE_QI_DEBUG(help);
    ENABLE_QI_DEBUG(label);
    ENABLE_QI_DEBUG(validity);
    ENABLE_QI_DEBUG(method_class);
    ENABLE_QI_DEBUG(method_attr_pair_list);

    ENABLE_QI_DEBUG2(start, "method_parser");
  }

private:

  /* instantiate primitive parser */
  identifier_parser<Iterator> parse_identifier;

  /* method parser rules */
  qi::rule<Iterator, std::string(), skipper_t> def_type;
  qi::rule<Iterator, std::string(), skipper_t> def_name;
  qi::rule<Iterator, pair_type(), skipper_t> help;
  qi::rule<Iterator, pair_type(), skipper_t> label;
  qi::rule<Iterator, pair_type(), skipper_t> method_access;
  qi::rule<Iterator, void(), skipper_t> definition;
  qi::rule<Iterator, pair_type(), skipper_t> validity;
  qi::rule<Iterator, pair_type(), skipper_t> method_class;
  qi::rule<Iterator, std::string(), skipper_t> method_attr;
  qi::rule<Iterator, std::string(), skipper_t> method_attr_value;
  qi::rule<Iterator, pair_type(), skipper_t> method_attr_pair;
  qi::rule<Iterator, std::vector<pair_type>(), skipper_t> method_attr_pair_list;

  /* start rule*/
  qi::rule<Iterator, method_definition(), skipper_t> start;
};

/**
 * EDDL COMMAND parser
 *
 * Parses the EDDL COMMAND construct.
 *
 */
template <typename Iterator>
struct command_parser
  : qi::grammar<Iterator, command_definition(), skipper<Iterator> >
{
  typedef skipper<Iterator> skipper_t;

  command_parser()
    : command_parser::base_type(start)
  {
    def_type %= qi::string("COMMAND");

    def_name %= parse_identifier;

    command_attribute
      %= command_header
      |  command_slot
      |  command_index
      |  command_block
      |  command_number
      |  command_operation;

    command_header %= qi::string("HEADER")
            >> parse_identifier;

    command_slot %= qi::string("SLOT")
      >> parse_identifier;

    command_index %= qi::string("INDEX")
      >> qi::raw[parse_integer];

     command_block %= qi::string("BLOCK")
       >> parse_identifier;

     command_number %= qi::string("NUMBER")
       >> qi::raw[parse_integer];

     command_operation %= qi::string("OPERATION")
       >> parse_identifier;

     command_transaction %= qi::string("TRANSACTION")
       >> qi::lit('{')
       >> +transaction_attribute
       >> qi::lit('}');

     transaction_attribute %= reply
       | request
       | qi::raw[parse_integer]
       | response_code;

     reply %= qi::string("REPLY")
       >> qi::lit('{')
       >> -(data_items_pair % qi::lit(','))
       >> qi::lit('}');

    request%= qi::string("REQUEST")
      >> qi::lit('{')
      >> -(data_items_pair % qi::lit(','))
      >> -qi::lit(',')
      >> qi::lit('}');

    data_items_pair %= dataitems >> mask ;
    dataitems %= parse_identifier;
    mask %= -(qi::lit('<') >> parse_integer >> qi::lit('>'));

    response_code %=  qi::lit("RESPONSE_CODES")
      >> qi::lit('{')
      >> parse_identifier
      >> qi::raw[parse_integer]
      >> +parse_identifier
      >> qi::lit('}');

    cmd_attr_pair_list %= +(command_transaction | ((command_attribute)
      >> ';'));

    start %= def_type
      >> def_name
      >> qi::lit("{")
      >> cmd_attr_pair_list
      >> qi::lit("}");

    init_debug();
  }
  void init_debug()
  {
    ENABLE_QI_DEBUG(def_type);
    ENABLE_QI_DEBUG(def_name);
    ENABLE_QI_DEBUG(command_header);
    ENABLE_QI_DEBUG(command_slot);
    ENABLE_QI_DEBUG(command_index);
    ENABLE_QI_DEBUG(command_block);
    ENABLE_QI_DEBUG(command_number);
    ENABLE_QI_DEBUG(command_operation);
    ENABLE_QI_DEBUG(command_connection);
    ENABLE_QI_DEBUG(command_module);
    ENABLE_QI_DEBUG(command_connection);
    ENABLE_QI_DEBUG(command_response_codes);
    ENABLE_QI_DEBUG(command_transaction);
    ENABLE_QI_DEBUG(command_attribute);
    ENABLE_QI_DEBUG(cmd_attr_pair_list);
    ENABLE_QI_DEBUG(transaction_attribute);
    ENABLE_QI_DEBUG(reply);
    ENABLE_QI_DEBUG(request);
    ENABLE_QI_DEBUG(response_code);

    ENABLE_QI_DEBUG2(start, "command_parser");
  }

private:

  /* instantiate primitive parsers */
  identifier_parser<Iterator> parse_identifier;
  string_parser<Iterator> parse_string;
  integer_parser<Iterator> parse_integer;

  /* command parser rules */
  qi::rule<Iterator, std::string(), skipper_t> def_type;
  qi::rule<Iterator, std::string(), skipper_t> def_name;
  qi::rule<Iterator, pair_type(), skipper_t> command_header;
  qi::rule<Iterator, pair_type(), skipper_t> command_slot;
  qi::rule<Iterator, pair_type(), skipper_t> command_index;
  qi::rule<Iterator, pair_type(), skipper_t> command_block;
  qi::rule<Iterator, pair_type(), skipper_t> command_number;
  qi::rule<Iterator, pair_type(), skipper_t> command_operation;
  qi::rule<Iterator, command_transaction_definition(), skipper_t> command_transaction;
  qi::rule<Iterator, pair_type(), skipper_t> command_connection;
  qi::rule<Iterator, pair_type(), skipper_t> command_module;
  qi::rule<Iterator, pair_type(), skipper_t> command_response_codes;
  qi::rule<Iterator, pair_type(), skipper_t> command_attribute;
  qi::rule<Iterator, std::pair<std::string,std::vector<pair_type_string_int>>(), skipper_t> reply;
  qi::rule<Iterator, std::pair<std::string,std::vector<pair_type_string_int>>(), skipper_t> request;
  qi::rule<Iterator, pair_type_string_int(), skipper_t> data_items_pair;
  qi::rule<Iterator, std::string(), skipper_t> dataitems;
  qi::rule<Iterator, boost::optional<uint32_t>(), skipper_t> mask;
  qi::rule<Iterator, transaction_variant(), skipper_t> transaction_attribute;
  qi::rule<Iterator, responseCode_definition(), skipper_t> response_code;
  qi::rule<Iterator, command_variant(), skipper_t> cmd_attr_pair_list;

  /* start rule */
  qi::rule<Iterator, command_definition(), skipper_t> start;
};

/**
 * EDDL UNIT parser
 *
 * Parses the EDDL UNIT construct.
 *
 */
template <typename Iterator>
struct unit_parser
: qi::grammar<Iterator, unit_definition(), skipper<Iterator>>
{
  typedef skipper<Iterator> skipper_t;

  unit_parser()
    : unit_parser::base_type(start)
  {
    def_type %= qi::lit("UNIT");

    def_name %= (ascii::upper | ascii::lower)
      >> *(ascii::upper | ascii::lower | ascii::digit | qi::char_('_'));

    def_identifier %= (ascii::upper | ascii::lower)
      >> *(ascii::upper | ascii::lower | ascii::digit | qi::char_('_'));

    def_dependent %= def_identifier;
    def_dependency %= def_identifier;
    def_dependencies %= def_dependency % qi::lit(',');

    start %= def_type
      >> def_name
      >> qi::lit('{')
      >> def_dependent
      >> qi::lit(':')
      >> def_dependencies
      >> qi::lit('}');

    init_debug();
  }

  void init_debug()
  {
    ENABLE_QI_DEBUG(def_type);
    ENABLE_QI_DEBUG(def_name);
    ENABLE_QI_DEBUG(def_identifier);
    ENABLE_QI_DEBUG(def_dependent);
    ENABLE_QI_DEBUG(def_dependency);
    ENABLE_QI_DEBUG(def_dependencies);

    ENABLE_QI_DEBUG2(start, "unit_parser");
  }

private:

  /* unit parser rules */
  qi::rule<Iterator, std::string(), skipper_t> def_type;
  qi::rule<Iterator, std::string(), skipper_t> def_name;
  qi::rule<Iterator, std::string(), skipper_t> def_identifier;
  qi::rule<Iterator, std::string(), skipper_t> def_dependent;
  qi::rule<Iterator, std::string(), skipper_t> def_dependency;
  qi::rule<Iterator, std::vector<std::string>(), skipper_t> def_dependencies;

  /* start rule */
  qi::rule<Iterator, unit_definition(), skipper_t> start;
};

/**
 * EDDL identification information parser
 *
 * Parses the EDDL identification information.
 *
 */
template <typename Iterator>
struct id_parser
  : qi::grammar<Iterator, id_definition(), skipper<Iterator> >
{
  typedef skipper<Iterator> skipper_t;

  id_parser()
    : id_parser::base_type(start)
  {
    manufacturer_id %= qi::string("MANUFACTURER");
    edd_revision %= qi::string("DD_REVISION");
    device_revision %= qi::string("DEVICE_REVISION");
    device_type %= qi::string("DEVICE_TYPE");
    edd_profile %= qi::string("EDD_PROFILE");
    edd_version %= qi::string("EDD_VERSION");

    identifier %=  +(qi::graph - ',');
    id_pair %=  (manufacturer_id | edd_revision | device_revision |
         device_type | edd_profile | edd_version) >> identifier;

    id_pair_list %= id_pair % ',';
    start %= id_pair_list;

    init_debug();
  }

  void init_debug()
  {
    ENABLE_QI_DEBUG(manufacturer_id);
    ENABLE_QI_DEBUG(edd_revision);
    ENABLE_QI_DEBUG(device_revision);
    ENABLE_QI_DEBUG(device_type);
    ENABLE_QI_DEBUG(edd_profile);
    ENABLE_QI_DEBUG(edd_version);
    ENABLE_QI_DEBUG(id_pair);
    ENABLE_QI_DEBUG(id_pair_list);

    ENABLE_QI_DEBUG2(start, "id_parser");
  }

private:

  /*id identification parser rules */
  qi::rule<Iterator, std::string()> manufacturer_id;
  qi::rule<Iterator, std::string()> edd_revision;
  qi::rule<Iterator, std::string()> identifier;
  qi::rule<Iterator, std::string()> device_revision;
  qi::rule<Iterator, std::string()> device_type;
  qi::rule<Iterator, std::string()> edd_profile;
  qi::rule<Iterator, std::string()> edd_version;
  qi::rule<Iterator, pair_type(), skipper_t> id_pair;
  qi::rule<Iterator, std::vector<pair_type>(), skipper_t> id_pair_list;

  /* start rule */
  qi::rule<Iterator, id_definition(), skipper_t> start;
};

/**
 * Generic parser
 *
 * Instantiates all defined parsers and parses the EDDL file.
 *
 */
template <typename Iterator>
struct eddl_parser
  : qi::grammar<Iterator, ConfigEDDL::eddlParsedData(), skipper<Iterator>>
{
  typedef skipper<Iterator> skipper_t;

  eddl_parser()
    : eddl_parser::base_type(start)
  {
    start %= +(id_def | var_def | unit_def | method_def | cmd_def);
  }

private:

  /* instantiate defined parsers */
  id_parser<Iterator> id_def;
  variable_parser<Iterator> var_def;
  command_parser<Iterator> cmd_def;
  method_parser<Iterator> method_def;
  unit_parser<Iterator> unit_def;

  /* start rule */
  qi::rule<Iterator, ConfigEDDL::eddlParsedData(), skipper<Iterator> > start;
 };

/*
* parseEDDLfile()
*/
bool ConfigEDDL::parseEDDLfile(const std::string& eddlfile, eddlParsedData& data)
{
  typedef std::string::const_iterator iterator_type;

  eddl_parser<iterator_type> p;
  skipper<iterator_type> s;
  std::string edd_string;

  /* check if eddl file exist */
  if (!boost::filesystem::exists(eddlfile)) {
        std::cout << "EDDL file not found" << std::endl;
  }

  /* read input EDD file as string */
  std::ifstream filestream (eddlfile.c_str(), std::ios_base::in);
  filestream.unsetf(std::ios::skipws);
  std::copy(std::istream_iterator<char>(filestream), std::istream_iterator<char>(), std::back_inserter(edd_string));

  iterator_type begin = edd_string.begin();
  iterator_type end = edd_string.end();

  /* Invoke the parser */
  bool ok =  qi::phrase_parse (begin, end, p, s, data);
  if (ok)
  {
    std::cout << "Bytes left = " << std::distance(begin, end) << " -> "
             << ((begin == end) ? "SUCCEEDED" : "FAILED") << "\n";

    /* Print size of parsed EDDL data */
    std::cout << "Size of parsed EDDL data: " << data.size() << "\n";

    /* Print parsed EDDL data */
    printEDDLDataItems(data);

    /* create information model*/
    createInformationModel();

  } else {
    std::cout << "PARSING FAILED COMPLETELY" << std::endl;
  }

  return true;
}

/*
* createSensorDataValue()
*/
OpcUaDataValue::SPtr ConfigEDDL::createSensorDataValue(void)
{
  OpcUaDataValue::SPtr dataValue;
  dataValue = constructSPtr<OpcUaDataValue>();
  dataValue->statusCode(Success);
  dataValue->sourceTimestamp(boost::posix_time::microsec_clock::universal_time());
  dataValue->serverTimestamp(boost::posix_time::microsec_clock::universal_time());

  return dataValue;
}


void SensorDataFile::copyTo(Attribute* attribute)
{
  OpcUaDataValue::SPtr datavalue;
  OpcUaStackServer::ValueAttribute* valueAttr;
  valueAttr = reinterpret_cast<OpcUaStackServer::ValueAttribute*>(&attribute);
  DeviceDataFile* nikiSensor = getInstatnce();
  int32_t sensorVal = nikiSensor->getVal()->getVal().i32;

  OpcUaDataValue::SPtr dataValue;
  dataValue = constructSPtr<OpcUaDataValue>();
  dataValue->statusCode(Success);
  dataValue->sourceTimestamp(boost::posix_time::microsec_clock::universal_time());
  dataValue->serverTimestamp(boost::posix_time::microsec_clock::universal_time());

  dataValue->variant()->variant((uint32_t)36);
#if 1
  dataValue->variant()->copyTo(*valueAttr->data().variant());

#else
  dataValue->variant()->copyTo(*valueAttr->data());
#endif
}

/*
* createObjectNode()
*/
bool ConfigEDDL::createObjectNode (void)
{
  /* set object node browse name */
  OpcUaQualifiedName browseName;
  browseName.set("Temperature_Sensor", 0);

  /* set object node description */
  OpcUaLocalizedText description;
  description.set("de", "Temperature_Sensor");

  /* set object node displayName */
  OpcUaLocalizedText displayName;
  displayName.set("de", "Temperature_Sensor");

  /* set object node id */
  OpcUaNodeId nodeId;
  nodeId.set(1104, namespaceIndex_);
  OpcUaNodeId refId;
  refId.set(85, namespaceIndex_);

  /* create object node and set node properties */
  objectNode = OpcUaStackServer::ObjectNodeClass::construct();
  objectNode->setNodeId(nodeId);
  objectNode->setBrowseName(browseName);
  objectNode->setDescription(description);
  objectNode->setDisplayName(displayName);

  /* create reference relationship of object and variable node */
  OpcUaNodeId variableNodeId;
  if (variableNode->getNodeId(variableNodeId)) {
    /* create object references to variable node */
    objectNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
    ReferenceType_HasComponent, true, variableNodeId);

    objectNode->referenceItemMap().add(OpcUaStackServer::ReferenceType::
    ReferenceType_Organizes, false, refId);
  }

  /* encode Object node */
  nodeSetXmlParser.objectNodeClassVec().push_back(objectNode);
  nodeSetXmlParser.encode(configxml.ptree());

  return true;
}

/*
* createVariableNode()
*/
bool ConfigEDDL::createVariableNode(void)
{
  /* initialise variable node properties */
  OpcUaByte accessLevel(0x01);

  /* set variable node browse name */
  OpcUaQualifiedName browseName;
  browseName.set("sample_browseName", namespaceIndex_);

  /* set variable node description */
  OpcUaLocalizedText description;
  description.set("de", "sample_description");

  /* set displayName */
  OpcUaLocalizedText  displayName;
  displayName.set("de", "sample_displayName");

  /* set variable node id */
  OpcUaInt32 nodeIdValue = IDGenerator::instance()->getNextId();
  OpcUaNodeId nodeId;
  nodeId.set(1000, namespaceIndex_);

  /*  set initial value of variable node */
  variableNodedataValue_ = createSensorDataValue();
  variableNodedataValue_ ->variant()->variant((uint32_t)currentSensorVal);

  /* create variable Node and set node properties */
  variableNode = OpcUaStackServer::VariableNodeClass::construct();
  variableNode->setNodeId(nodeId);
  variableNode->setAccessLevel(accessLevel);
  variableNode->setDataType(nodeId);
  variableNode->setBrowseName(browseName);
  variableNode->setDescription(description);
  variableNode->setDisplayName(displayName);
  variableNode->setValue(*variableNodedataValue_);

  SensorDataFile* nikiSensor = SensorDataFile::getInstatnce();
  //nikiSensor->copyTo(variableNode->valueAttribute());

  OpcUaStackServer::InformationModel::SPtr infoModel;
  infoModel = OpcUaStackServer::InformationModel::construct();
  OpcUaStackServer::InformationModelAccess infoModelAccess;
  infoModelAccess.informationModel(infoModel);
  //infoModel = server_.getInformationModel();

  infoModel->insert(variableNode);
  //infoModelAccess.add(infoModel, namespaceIndex_);

  uint32_t infoModelSize = infoModel->size();
  std::cout << "Size of information model is: " << infoModelSize << std::endl;

  OpcUaStackServer::Server server;
  OpcUaStackServer::InformationModel::SPtr infoModel2;
  infoModel2 = server.getInformationModel();
  uint32_t infoModelSize2 = infoModel2->size();
  std::cout << "Size of information model2 is: " << infoModelSize2 << std::endl;

  OpcUaNodeId nodeId2;
  nodeId.set(85, namespaceIndex_);
  OpcUaStackServer::BaseNodeClass::SPtr baseptr = infoModel->find(nodeId2);
  if (baseptr.get() != nullptr) {
    std::cout << "Found object node" << std::endl;
  }

  /* encode Variable node */
  nodeSetXmlParser.variableNodeClassVec().push_back(variableNode);
  nodeSetXmlParser.encode(configxml.ptree());

  return true;
}

bool ConfigEDDL::createObjectNode2(void)
{
  OpcUaStackServer::InformationModel::SPtr infoModel2;
  OpcUaServer::Server server;
  infoModel2 = server.getInformationModel();
  uint32_t infoModelSize2 = infoModel2->size();
  std::cout << "Size of information model2 in Server is: " << infoModelSize2 << std::endl;

  /* set object node browse name */
  OpcUaQualifiedName browseName;
  browseName.set("NIKI_NIKI_SAMPLE", 0);

  /* set object node description */
  OpcUaLocalizedText description;
  description.set("de", "NIKI_NIKI_SAMPLE");

  /* set object node displayName */
  OpcUaLocalizedText displayName;
  displayName.set("de", "NIKI_NIKI_SAMPLE");

  OpcUaNodeId nodeId;
  nodeId.set(1102, 0);

  OpcUaNodeId nodeId2;
  nodeId2.set(85, 0);
  OpcUaStackServer::BaseNodeClass::SPtr baseptr = infoModel2->find(nodeId2);
  if (baseptr.get() != nullptr) {
    /* create object node and set node properties */
    OpcUaStackServer::BaseNodeClass::SPtr objectNode3 = OpcUaStackServer::ObjectNodeClass::construct();
    objectNode3->setNodeId(nodeId);
    objectNode3->setBrowseName(browseName);
    objectNode3->setDescription(description);
    objectNode3->setDisplayName(displayName);

    /* add object node type reference */
    baseptr->setNodeId(nodeId2);

    baseptr->referenceItemMap().add(OpcUaStackServer::ReferenceType::
         ReferenceType_Organizes, true, nodeId);

    baseptr->referenceItemMap().add(OpcUaStackServer::ReferenceType::
               ReferenceType_HasComponent, true, nodeId);

    objectNode3->referenceItemMap().add(OpcUaStackServer::ReferenceType::
    ReferenceType_Organizes, false, nodeId2);

    /* insert the object node to the information model */
    infoModel2->insert(objectNode3);
  }
  return true;
}

/**
 * createInformationModel()
 */
bool ConfigEDDL::createInformationModel(void)
{
  /* create nikiSensor object */
  SensorDataFile* nikiSensor = SensorDataFile::getInstatnce();
  DeviceDataValue *p_val = new DeviceDataValue(DeviceDataValue::TYPE_INTEGER);

  /* set initial sensor value */
  p_val->setVal(0);
  nikiSensor->setVal(p_val);

  /* suscribe for data changes */
  nikiSensor->observeVal(nikiSensor->getupdateValue, p_val);

  /* initialise variable node value */
  const DeviceDataValue *p_sensorVal = NULL;
  p_sensorVal = nikiSensor->getVal();
  currentSensorVal = p_val->getVal().i32;

  /* create variable node */
  if (!createVariableNode()) {
    return false;
  }

  /* create object node */
  if (!createObjectNode()) {
    return false;
  }

  /* generate information model */
  configxml.write("/usr/local/etc/OpcUaStack/TemperatureSensorNIKI.xml");

  /* temp - create nodes */
  newNode.createNode();

  return true;
}

} /* OpcUaStackCore namespace */



