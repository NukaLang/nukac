#ifndef NUKAC_PARSER_HPP
#define NUKAC_PARSER_HPP

#include <any>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "lexer.hpp"

namespace nukac::parser {
  namespace ast {
    class Expression {
      public:
        virtual ~Expression() = default;
    };
    
    class NumberExpression: Expression {
      public:
        NumberExpression(double val);
      private:
        double val;
    };

    class VariableExpression: Expression {
      public:
        VariableExpression(std::string &name);
      private:
        std::string name;
    };
    
    class BinaryExpression: Expression {
      public:
        enum class Operand {
          oand,
          oor,
          oxor,
          onot,
        };
        BinaryExpression(Operand operand, 
            std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs);
      private:
        Operand operand;
        std::unique_ptr<Expression> lhs, rhs;
    };

    class TypeExpression: Expression {
      public:
        TypeExpression(const std::string &name, std::unique_ptr<Expression> of_other_type);
      private:
        std::string name;
        std::unique_ptr<Expression> of_other_type;
    };

    class TypedVariableExpression: Expression {
      public:
        TypedVariableExpression(const std::string &name, TypeExpression &of_type);
      private:
        std::string name;
        TypeExpression of_type;
    };

    class StructExpression: Expression {
      public:
        StructExpression(const std::string &name, std::vector<std::unique_ptr<Expression>> contents);
      private:
        std::string name;
        std::vector<std::unique_ptr<Expression>> contents;
    };

    class CallExpression: Expression {
      public:
        CallExpression(const std::string &calle, std::vector<std::unique_ptr<Expression>> args);
      private:
        std::string callee;
        std::vector<std::unique_ptr<Expression>> args;
    };

    class Prototype {
      public:
        Prototype(const std::string &name, ast::TypeExpression &return_type, std::map<std::unique_ptr<ast::VariableExpression>, ast::TypeExpression &> args);
        const std::string &getName();
      private:
        std::string name;
        ast::TypeExpression &return_type;
        std::map<std::unique_ptr<ast::VariableExpression>, ast::TypeExpression &> args;
    };
    
    class Function {
      public:
        Function(std::unique_ptr<Prototype> proto, std::unique_ptr<std::vector<Expression>> body);
      private:
        std::unique_ptr<Prototype> proto;
        std::unique_ptr<std::vector<Expression>> body;
    };

    using ExpressionsAndFunctions = std::variant <
        Expression,
        Prototype,
        Function
      >;
  } // ast 


  class ParserException {
    public:
      ParserException(std::string what, std::string faulty_expression);
      ParserException(std::string what, nukac::lexer::Literal literal);
      const char *what();
    private:
      std::string what_did_i_do;
  };

  class Parser {
    public:
      // optional macro
      // PARSER_WHILE_CONDITIONAL
      Parser(lexer::Lexer &lexer);
      Parser(lexer::Lexer &lexer, std::vector<ast::Expression> expressions, 
          std::vector<ast::Prototype> prototypes, 
          std::vector<ast::Function> functions, 
          std::map<std::string, ast::TypeExpression &> parent_types);

      std::vector<ast::Expression> getExpressions();
      std::vector<ast::Prototype> getPrototypes();
      std::vector<ast::Function> getFunctions();

    private:
      lexer::Lexer &lexer;
      std::vector<ast::Expression> expressions;
      std::vector<ast::Prototype> prototypes;
      std::vector<ast::Function> functions;

      std::map<std::string, ast::TypeExpression &> scope_types;

      inline void parserInternal();
      inline void parserFunction();
  };

} // nukac::parser

#endif
