#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <variant>
#include <memory>

#include "lexer.hpp"
#include "parser.hpp"

namespace nukac::parser {

  ParserException::ParserException(std::string what, std::string faulty_expression) {
    what_did_i_do = std::format("{}...\n{}", what, faulty_expression);
  }

  ParserException::ParserException(std::string what, nukac::lexer::Literal literal) {
    std::stringstream ss;
    ss << what << "...\nat" << literal << "\n";
    what_did_i_do = ss.str();
  }

  const char *ParserException::what() {
    return what_did_i_do.c_str();
  }

  ast::NumberExpression::NumberExpression(double val): val(val) {}
  
  ast::VariableExpression::VariableExpression(std::string &name, TypeExpression &type): name(name), type(type) {}
  ast::VariableExpression::VariableExpression(std::string &name, 
            ast::TypeExpression &type, 
            std::vector<ast::Expression> &stored): name(name), type(type), stored(stored) {}

  void ast::VariableExpression::store(std::vector<ast::Expression> stored) {
    this->stored = stored;
  }

  std::vector<ast::Expression> ast::VariableExpression::getStored() {
    return stored;
  }

  ast::BinaryExpression::BinaryExpression(ast::BinaryExpression::Operand operand, ast::Expression &lhs,
      Expression &rhs): operand(operand), lhs(lhs), rhs(rhs) {}
  ast::CallExpression::CallExpression(const std::string &callee, 
      std::vector<ast::Expression> args): callee(callee), args(args) {}


  ast::TypeExpression::TypeExpression(const std::string &name, ast::Expression &of_other_type): 
    name(name), of_other_type(of_other_type) {}

  std::string ast::TypeExpression::TypeExpression::getName() {
    return name; 
  }
  ast::Expression &ast::TypeExpression::TypeExpression::referencingType() {
    return of_other_type;
  }
  
  ast::StructExpression::StructExpression(const std::string &name, std::vector<ast::Expression> contents):
    name(name), contents(std::move(contents)) {}
  ast::Prototype::Prototype(const std::string &name, ast::TypeExpression &return_type, 
      std::vector<ast::VariableExpression> args):
    name(name), return_type(return_type), args(std::move(args)) {}
  ast::Function::Function(Prototype &proto, std::vector<ast::Expression> body):
    proto(proto), body(std::move(body)) {}

#pragma clang diagnostic ignored "-Wunused" 
  // types, functions, traits
  constexpr const std::string function_kw = "fn";
  constexpr const std::string struct_kw = "struct";
  constexpr const std::string implements_kw = "implements";
  constexpr const std::string trait_kw = "trait";
  constexpr const std::string defer_kw = "defer";
  constexpr const std::string errdefer_kw = "errdefer";
  constexpr const std::string error_kw = "error";
  constexpr const std::string try_kw = "try";
  constexpr const std::string ignore_kw = "ignore";
  constexpr const std::string union_kw = "union";
  constexpr const std::string enum_kw = "enum";
  constexpr const std::string return_kw = "return";

  // modifiers
  constexpr const std::string mut_kw = "mut";
  constexpr const std::string pub_kw = "pub";

  // logical statements
  constexpr const std::string and_kw = "and";
  constexpr const std::string or_kw = "or";
  constexpr const std::string not_kw = "not";

  // modules
  constexpr const std::string import_kw = "import";
  constexpr const std::string module_kw = "module";


#define GET_PRSR(var, literal, msg) \
  try { \
    var = std::get<typeof(var)>(literal.literal_variant); \
  } catch(...) { \
    throw ParserException(msg, literal); \
  }

  inline void Parser::parserPFunction() {
    using namespace nukac::lexer;
    Literal return_type_l = lexer.swallow();
    std::string return_type_n;
    GET_PRSR(return_type_n, return_type_l, "Invalid token in function declaration");
    ast::TypeExpression &return_type = scope_types[return_type_n];

    Literal fun_name_l = lexer.swallow();
    std::string fun_name;
    GET_PRSR(fun_name, fun_name_l, "Invalid token in function declaration.");

    if(!lexer.next(Token::lparen)) {
      Literal l = lexer.swallow();
      throw ParserException("Invalid token in function declaration.", l);
    }
    lexer.swallowZ();
    std::vector<ast::VariableExpression> arguments;
    
    while(!lexer.next(Token::rparen)) {
      Literal name_l = lexer.swallow();
      std::string name_n;
      GET_PRSR(name_n, name_l, "Invalid token in function declaration.");
      
      if(!lexer.next(Token::colon)) {
        Literal l = lexer.swallow();
        throw ParserException("Invalid token in function declaration.", l);
      }
      lexer.swallowZ();

      Literal type_l = lexer.swallow();
      std::string type_n;
      GET_PRSR(type_n, type_l, "Invalid token in function declaration.");
      ast::TypeExpression type = scope_types[type_n];
      ast::VariableExpression variable(name_n, type);

      arguments.push_back(variable);

      if(!lexer.next(Token::comma) && !lexer.next(Token::rparen)) {
        Literal l = lexer.swallow();
        throw ParserException("Invalid token in function declaration.", l);
      }
    }
    

    ast::Prototype proto(fun_name, return_type, arguments);
    if(lexer.next(Token::lcrbrace)){
      #define PARSER_WHILE_CONDITIONAL lexer.next(lexer::Token::rcrbrace)

      Parser subnode(lexer, variables, expressions, prototypes, functions, scope_types, Scope::function);
      #undef PARSER_WHILE_CONDITIONAL  

      ast::Function func(proto, subnode.getExpressions());
    } else if(!lexer.next(Token::semicolon)){
      throw ParserException("Invalid token in function declaration.", lexer.swallow());
    }
    prototypes.push_back(proto);
  }

  inline void Parser::parserPVariable(std::string name) {
    using namespace lexer;
    lexer.swallowZ();
    Literal type_l = lexer.swallow();
    std::string type_n;
    GET_PRSR(type_n, type_l, "Invalid token in variable decleration.");
    ast::VariableExpression variable(name, scope_types[name]);
    if(lexer.next(Token::equals)) {
      #define PARSER_WHILE_CONDITIONAL lexer.next(lexer::Token::semicolon);
      
      Parser subnode(lexer, variables, expressions, prototypes, functions, scope_types, Scope::variable);
      #undef PARSER_WHILE_CONDITIONAL

      variable.store(subnode.getExpressions());
    }
    
    variables[name] = variable;
    expressions.push_back(variable);

    if(!lexer.next(Token::semicolon)) {
      throw ParserException("Invalid token in variable declaration", lexer.swallow());
    }
  }

  inline void Parser::parserPVariableAssign(std::string name) {
    using namespace lexer;
    lexer.swallowZ();



    Parser subnode(lexer, expressions, prototypes, functions, scope_types, Scope::variable);

  }

  void Parser::parserInternal() {
    nukac::lexer::Literal literal = lexer.swallow();
    using namespace nukac::lexer;
    if(literal.literal_variant == Token::dollar) {
      if(lexer.next("println")) {
        lexer.swallowZ();
        Literal p = lexer.swallow();
        std::cout << p << "\n";
      } else if(lexer.next("error")) {
        lexer.swallowZ();
        Literal what = lexer.swallow();
        throw ParserException("Custom compile error.", what);
      }
    } else if(literal.literal_variant == function_kw) {
      parserPFunction();
    } else if(literal.literal_variant == return_kw && scope == Scope::function) {
      
    } else if(literal.literal_variant == return_kw && scope != Scope::function) {
      throw ParserException("Return in a non-functional scope.", literal);
    } else if(lexer.next(Token::colon)) {
      std::string name;
      GET_PRSR(name, literal, "Invalid token in variable declaration.");
      parserPVariable(name);
    } else if(lexer.next(Token::equals)) {
      std::string name;
      GET_PRSR(name, literal, "Invalid token in a variable assignment.");
      parserPVariableAssign(name);
    }
  }

  Parser::Parser(lexer::Lexer &lexer): lexer(lexer) {
    #ifndef PARSER_WHILE_CONDITIONAL
    #define PARSER_WHILE_CONDITIONAL lexer.isEoC()
    #endif
    
    scope = Scope::structure;

    while(PARSER_WHILE_CONDITIONAL) {
      parserInternal();      
    }
  }
  
  #undef PARSER_WHILE_CONDITIONAL

  Parser::Parser(nukac::lexer::Lexer &lexer, 
      std::vector<ast::Expression> expressions,
      std::vector<ast::Prototype> prototypes,
      std::vector<ast::Function> functions,
      std::unordered_map<std::string, ast::TypeExpression> scope_types,
      Scope scope): lexer(lexer),
      expressions(std::move(expressions)),
      prototypes(std::move(prototypes)),
      functions(std::move(functions)), 
      scope_types(std::move(scope_types)),
      scope(scope)
      
  {
    #ifndef PARSER_WHILE_CONDITIONAL
    #define PARSER_WHILE_CONDITIONAL lexer.isEoC()
    #endif

    while(PARSER_WHILE_CONDITIONAL) {
      parserInternal();
    }
  }

  #undef PARSER_WHILE_CONDITIONAL

  std::vector<ast::Expression> Parser::getExpressions() {
    return expressions;
  }

  std::vector<ast::Prototype> Parser::getPrototypes() {
    return prototypes;
  }

  std::vector<ast::Function> Parser::getFunctions() {
    return std::move(functions);
  }
  const Scope Parser::getScope() {
    return scope;
  }
}
