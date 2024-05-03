#ifndef NUKAC_PARSER_HPP
#define NUKAC_PARSER_HPP

#include <memory>
#include <string>
#include <vector>

#include "lexer.hpp"

namespace nukac::parser {
  class ParserException {
    public:
      ParserException(std::string what, std::string faulty_expression);
      ParserException(std::string what, nukac::lexer::LiteralWithPosition literal);
      const char *what();
    private:
      std::string what_did_i_do;
  };

  void parser();
  
  namespace comptime_macros {
    void println();
  }

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
        Prototype(const std::string &name, std::vector<std::string> args);
        const std::string &getName();
      private:
        std::string name;
        std::vector<std::string> args;
    };
    
    class Function {
      public:
        Function(std::unique_ptr<Prototype> proto, std::unique_ptr<Expression> body);
      private:
        std::unique_ptr<Prototype> proto;
        std::unique_ptr<Expression> body;
    };
  } // namespace ast 
} // namespace nukac::parser

#endif
