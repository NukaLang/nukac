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
  ast::VariableExpression::VariableExpression(std::string &name): name(name) {}
  ast::BinaryExpression::BinaryExpression(ast::BinaryExpression::Operand operand, std::unique_ptr<ast::Expression> lhs,
      std::unique_ptr<Expression> rhs): operand(operand), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  ast::CallExpression::CallExpression(const std::string &callee, 
      std::vector<std::unique_ptr<ast::Expression>> args): callee(callee), args(std::move(args)) {}
  ast::TypeExpression::TypeExpression(const std::string &name, std::unique_ptr<ast::Expression> of_other_type): 
    name(name), of_other_type(std::move(of_other_type)) {}
  ast::StructExpression::StructExpression(const std::string &name, std::vector<std::unique_ptr<ast::Expression>> contents):
    name(name), contents(std::move(contents)) {}
  ast::Prototype::Prototype(const std::string &name, ast::TypeExpression &return_type, std::map<std::unique_ptr<ast::VariableExpression>, ast::TypeExpression &> args):
    name(name), return_type(return_type), args(std::move(args)) {}
  ast::Function::Function(std::unique_ptr<Prototype> proto, std::unique_ptr<ast::Expression> body):
    proto(std::move(proto)), body(std::move(body)) {}


  constexpr const std::string function_kw = "fn"; 

#define GET_PRSR(var, literal, msg) \
  try { \
    var = std::get<typeof(var)>(literal.literal_variant); \
  } catch(...) { \
    throw ParserException(msg, literal); \
  }

  inline ast::ExpressionsAndFunctions parserFunction(nukac::lexer::Lexer &lexer,
                 std::map<std::string, ast::TypeExpression &> &types) {
    using namespace nukac::lexer;
    Literal return_type_l = lexer.swallow();
    std::string return_type_n;
    GET_PRSR(return_type_n, return_type_l, "Invalid token in function declaration");
    ast::TypeExpression &return_type = types[return_type_n];

    Literal fun_name_l = lexer.swallow();
    std::string fun_name;
    GET_PRSR(fun_name, fun_name_l, "Invalid token in function declaration.");

    if(!lexer.next(Token::lparen)) {
      Literal l = lexer.swallow();
      throw ParserException("Invalid token in function declaration.", l);
    }
    lexer.swallowZ();
    std::map<std::unique_ptr<ast::VariableExpression>, ast::TypeExpression &> arguments;
    
    while(!lexer.next(Token::rparen)) {
      Literal name_l = lexer.swallow();
      std::string name_n;
      GET_PRSR(name_n, name_l, "Invalid token in function declaration.");
      std::unique_ptr<ast::VariableExpression> name = std::make_unique<ast::VariableExpression>(ast::VariableExpression(name_n));
      
      if(!lexer.next(Token::colon)) {
        Literal l = lexer.swallow();
        throw ParserException("Invalid token in function declaration.", l);
      }
      lexer.swallowZ();

      Literal type_l = lexer.swallow();
      std::string type_n;
      GET_PRSR(type_n, type_l, "Invalid token in function declaration.");
      ast::TypeExpression &type = types[type_n];

      arguments[name] = std::move(type);

      if(!lexer.next(Token::comma) && !lexer.next(Token::rparen)) {
        Literal l = lexer.swallow();
        throw ParserException("Invalid token in function declaration.", l);
      }
    }
    
    // TODO: implement function definitions lol
    if(lexer.next(Token::lcrbrace)){
      throw ParserException("Unimplemented", "Function definition.");
    } else {
      ast::Prototype proto(fun_name, return_type, arguments);
      return proto; 
    }
  }

  void parser(nukac::lexer::Lexer &lexer) {
    std::vector<ast::ExpressionsAndFunctions> expressions_and_functions;

    std::map<std::string, ast::TypeExpression &> types;
    while(true) {
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
        expressions_and_functions.push_back(parserFunction(lexer, types));
      }
    }
  }
}
