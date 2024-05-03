#include <format>
#include <iostream>
#include <sstream>
#include <variant>

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
ast::Prototype::Prototype(const std::string &name, std::vector<std::string> args):
  name(name), args(std::move(args)) {}
ast::Function::Function(std::unique_ptr<Prototype> proto, std::unique_ptr<ast::Expression> body):
  proto(std::move(proto)), body(std::move(body)) {}


constexpr const std::string function_kw = "fn";

void parser(nukac::lexer::Lexer &lexer) {
  std::vector<ast::Expression> expressions;
  while(true) {
    nukac::lexer::LiteralWithPosition literal = lexer.swallow();
    using namespace nukac::lexer;
    if(*std::get_if<Token>(&literal.literal) == Token::dollar) {
      if(lexer.next("println")) {
        lexer.swallowZ();
        LiteralWithPosition p = lexer.swallow();
        std::cout << p << "\n";
      }
    } else if(*std::get_if<std::string>(&literal.literal) == function_kw) {
      LiteralWithPosition func_name = lexer.swallow();
      LiteralWithPosition func_return_type = lexer.swallow();
      if(!lexer.next(Token::lparen)){
        LiteralWithPosition inv = lexer.swallow();
        throw ParserException("Invalid character in a function definition.", inv);
      }
      
      lexer.swallowZ();
    }
  }

}
