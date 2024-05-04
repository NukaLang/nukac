#include <format>
#include <iostream>
#include <map>
#include <sstream>
#include <variant>
#include <memory>

#include "lexer.hpp"
#include "parser.hpp"

using namespace nukac::parser;

ParserException::ParserException(std::string what, std::string faulty_expression) {
  what_did_i_do = std::format("{}...\n{}", what, faulty_expression);
}

ParserException::ParserException(std::string what, nukac::lexer::LiteralWithPosition literal) {
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

/* inline void parserFunction(nukac::lexer::Lexer &lexer,
               std::map<std::string, ast::TypeExpression &> &types,
               std::vector<ast::ExpressionsAndFunctions> &expressions) {
  using namespace nukac::lexer;
  LiteralWithPosition func_name_l = lexer.swallow();
  std::string func_name;
  try {
    func_name = std::get<std::string>(func_name_l.literal);
  } catch (std::exception &e) {
    throw ParserException("Invalid character in a function definition.", func_name_l);
  }

  LiteralWithPosition func_return_type_name_l = lexer.swallow();
  std::string func_return_type_name;
  try {
    func_return_type_name = std::get<std::string>(func_return_type_name_l.literal);
  } catch (std::exception &e) {
    throw ParserException("Invalid character in a function defintion.", func_return_type_name_l);
  }

  ast::TypeExpression &func_return_type = types[func_return_type_name]; 

  if (!lexer.next(Token::lparen)) {
    LiteralWithPosition inv = lexer.swallow();
    throw ParserException("Invalid character in a function definition.", inv);
  }

  lexer.swallowZ();
  std::map<std::unique_ptr<ast::VariableExpression>, ast::TypeExpression &> arguments;
  while (lexer.next(Token::lparen)) {
    LiteralWithPosition name = lexer.swallow();
    ast::VariableExpression lnv = *std::get_if<std::string>(&name.literal);
    std::unique_ptr<ast::VariableExpression> lv(&lnv);
    if (!lexer.next(Token::colon)) {
      LiteralWithPosition inv = lexer.swallow();
      throw ParserException("Invalid character in a function definition.", inv);
    }

    lexer.swallowZ();

    std::string rnv;
    LiteralWithPosition type;
    try {
      type = lexer.swallow();
      rnv = std::get<std::string>(name.literal);
    } catch (std::exception &e) {
      throw ParserException("Invalid token in a function definition.", type);
    }

    ast::TypeExpression &rv = types[rnv]; 
    arguments.insert({ lv, rv });
  }
  
  if(!lexer.next(Token::lcrbrace)) {
    ast::Prototype(func_name, func_return_type, arguments);
  } 
} */
void parser(nukac::lexer::Lexer &lexer) {
  std::vector<ast::ExpressionsAndFunctions> expressions_and_functions;

  std::map<std::string, ast::TypeExpression &> types;
  while(true) {
    nukac::lexer::LiteralWithPosition literal = lexer.swallow();
    using namespace nukac::lexer;
    if(*std::get_if<Token>(&literal.literal) == Token::dollar) {
      if(lexer.next("println")) {
        lexer.swallowZ();
        LiteralWithPosition p = lexer.swallow();
        std::cout << p << "\n";
      } else if(lexer.next("error")) {
        lexer.swallowZ();
        LiteralWithPosition what = lexer.swallow();
        throw ParserException("Custom compile error.", what);
      }
    }/* else if(*std::get_if<std::string>(&literal.literal) == function_kw) {
      parserFunction(lexer, types, expressions_and_functions);
    }*/
  }
}
