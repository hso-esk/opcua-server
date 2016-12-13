/*
 * EddlParser.cpp
 *
 *  Created on: 10 Dec 2016
 *      Author: osboxes
 */

/*
 * --- DEFINES ------------------------------------------------------------- *
 */

#define BOOST_SPIRIT_DEBUG

/*
 * --- Includes ------------------------------------------------------------- *
 */
#include "OpcUaEddl/EddlParser.h"
#include "OpcUaEddl/EddlParserUtils.h"

#include "OpcUaStackServer/Server/Server.h"
#include "OpcUaStackCore/Base/os.h"
#include "OpcUaStackCore/Base/Log.h"
#include "OpcUaStackCore/BuildInTypes/BuildInTypes.h"
#include "OpcUaStackCore/Base/ObjectPool.h"
#include "OpcUaStackServer/ServiceSetApplication/ApplicationService.h"
#include "OpcUaStackServer/Application/Application.h"
#include "OpcUaStackCore/ServiceSet/ServiceTransaction.h"

#include <boost/shared_ptr.hpp>
#include <boost/fusion/include/io.hpp>
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

namespace OpcUaEddl
{

namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;
namespace fusion = boost::fusion;
namespace repo = boost::spirit::repository;

/**
 * EddlParser()
 */
EddlParser::EddlParser(void)
{
  Log (Debug, "EddlParser::EddlParser");
}

/**
 * ~EddlParser()
 */
EddlParser::~EddlParser(void)
{
  Log (Debug, "EddlParser::~EddlParser");
}

/*
 * ---EDDL Parser Grammar definitions----------------------------- *
 */

/**
 * \brief Skipper parser grammar definition.
 *
 *      Parser skips whitespaces, single and block comments,
 *      preprocessor directives in EDDL file
 *
 */
template <typename Iterator>
struct skipper
  : qi::grammar<Iterator>
{
  skipper()
    : skipper::base_type(start)
  {
    start %= ascii::space
      | (qi::lit("#") >> *(qi::char_-qi::eol) >> qi::eol)
      | repo::confix("//", qi::eol)[*(qi::char_ - qi::eol)]
      | repo::confix("/*", "*/")[*(qi::char_ - "*/")];
  }

private:
  qi::rule<Iterator> start;
};


/**
 * \brief String parser grammar definition.
 *
 *      Parser parses quoted strings
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
 * \brief Integer parser grammar definition.
 *
 *      Parser parses the integer primitive type.
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
 * \brief String specifier parser grammar definition.
 *
 *      Parser parses different string types.
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
 * \brief Identifier parser grammar definition.
 *
 *      Parser parses EDDL identifiers.
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
 * \brief Real parser grammar definition.
 *
 *      Parser parses the real primitive type.
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
 * \brief Expression parser grammar definition.
 *
 *      Parser parses EDDL primary expressions.
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

    init_debug();
  }

  void init_debug()
  {
    ENABLE_QI_DEBUG(primary_expr);
    ENABLE_QI_DEBUG2(start, "expr_parser");
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
 * \brief Expression specifier parser grammar definition.
 *
 *      Parser parses EDDL expressions types. Only primary
 *      expression type is currently implemented.
 *
 */
template <typename Iterator>
struct expr_specifier_parser
  : qi::grammar<Iterator, std::string(), skipper<Iterator>>
{
  typedef skipper<Iterator> skipper_t;

  expr_specifier_parser()
    : expr_specifier_parser::base_type(start)
  {
    expr_wrap %= qi::raw[parse_expression];
    start %= (expr_wrap >> qi::lit(';'));

    init_debug();
  }

  void init_debug()
  {
    ENABLE_QI_DEBUG(expr_wrap);
    ENABLE_QI_DEBUG2(start, "expr_specifier_parser");
  }

private:

  /* instantiate expression parser */
  expr_parser<Iterator> parse_expression;

  qi::rule<Iterator, std::string(), skipper_t> expr_wrap;
  qi::rule<Iterator, std::string(), skipper_t> start;
};

/**
 * Arithmetic specifier type parser
 *
 * Parses all arithmetic types.
 *
 */

/**
 * \brief Arithmetic parser grammar definition.
 *
 *      Parser parses EDDL primitive expressions with
 *      their set values (Default value etc.)
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
    arithmetic_default_value %= qi::string("DEFAULT_VALUE") >> parse_expression;
    arithmetic_initial_value %= qi::string("INITIAL_VALUE") >> parse_expression;

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

    init_debug();
  }

  void init_debug()
  {
    ENABLE_QI_DEBUG(arithmetic_option_size);
    ENABLE_QI_DEBUG(arithmetic_option_list);
    ENABLE_QI_DEBUG(arithmetic_options);
    ENABLE_QI_DEBUG(arithmetic_option);
    ENABLE_QI_DEBUG(arithmetic_default_value);

    ENABLE_QI_DEBUG2(start, "arithmetic_type_parser");
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
 * \brief Enumerator parser grammar definition.
 *
 *      Parser parses EDDL enumerator types.
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

  qi::rule<Iterator, enumerated_type_definition(), skipper_t> start;
};


/**
 * \brief Variable type parser grammar definition.
 *
 *      Parser parses type definitions of the EDDL Variable
 *      construct.
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

  qi::rule<Iterator, variable_type_definition(), skipper_t> start;
};


/**
 * \brief Variable parser grammar definition.
 *
 *      Parser parses the EDDL Variable construct.
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

    var_attr_pair_list %= +(var_type | ((var_attr_pair) >> qi::lit(';')));

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

    qi::rule<Iterator, variable_definition(), skipper_t> start;
};


/**
 * \brief Method parser grammar definition.
 *
 *      Parser parses the EDDL Method construct.
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
 * \brief Command parser grammar definition.
 *
 *      Parser parses the EDDL Command construct.
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
 * \brief Unit parser grammar definition.
 *
 *      Parser parses the EDDL Unit construct.
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
 * \brief EDDL identification parser grammar definition.
 *
 *      Parser parses the EDDL identification info.
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
 * \brief EDDL identification parser grammar definition.
 *
 *      Parser parses the EDDL identification info.
 *
 */
template <typename Iterator>
struct eddl_parser
  : qi::grammar<Iterator, EddlParser::eddlParsedData(), skipper<Iterator>>
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
  qi::rule<Iterator, EddlParser::eddlParsedData(), skipper<Iterator> > start;
 };


/*
* parseEDDLfile()
*/
bool EddlParser::parseEDDLfile(const std::string& eddlfile, eddlParsedData& data)
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

  } else {
    std::cout << "PARSING FAILED COMPLETELY" << std::endl;
  }

  return true;
}

} /* namespace OpcUaEddl */




