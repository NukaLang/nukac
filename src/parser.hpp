#ifndef NUKAC_PARSER_HPP
#define NUKAC_PARSER_HPP

#include <any>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

#include "lexer.hpp"

namespace nukac::parser {
  namespace ast {
    class Expression {
      public:
        virtual ~Expression() = default;
    };
    
    class NumberExpression: public Expression {
      public:
        NumberExpression(double val);
      private:
        double val;
    };

    class TypeExpression: public Expression {
      public:
        TypeExpression(const std::string &name, Expression &of_other_type);

        std::string getName();
        Expression &referencingType();
      private:
        std::string name;
        Expression &of_other_type;
    };

    class VariableExpression: public Expression {
      public:
        VariableExpression(std::string &name, ast::TypeExpression &type);
        VariableExpression(std::string &name, 
            ast::TypeExpression &type, 
            std::vector<ast::Expression> &stored);

        void store(std::vector<Expression> stored);
        std::vector<Expression> getStored();

      private:
        std::string name;
        TypeExpression &type;
        std::vector<Expression> stored;
    };
    
    class BinaryExpression: public Expression {
      public:
        enum class Operand {
          oand,
          oor,
          oxor,
          onot,
          oplus,
          ominus,
          otimes,
          odivide,
          omodulo,

        };
        BinaryExpression(Operand operand, 
            Expression &lhs, Expression &rhs);
      private:
        Operand operand;
        Expression & lhs, rhs;
    };

    class StructExpression: public Expression {
      public:
        StructExpression(const std::string &name, std::vector<Expression> contents);
      private:
        std::string name;
        std::vector<Expression> contents;
    };

    class CallExpression: public Expression {
      public:
        CallExpression(const std::string &callee, std::vector<Expression> args);
      private:
        std::string callee;
        std::vector<Expression> args;
    };

    class Prototype {
      public:
        Prototype(const std::string &name, ast::TypeExpression &return_type, 
            std::vector<ast::VariableExpression> args);
        const std::string &getName();
        const std::vector<ast::VariableExpression> getVariables;
      private:
        std::string name;
        ast::TypeExpression &return_type;
        std::vector<ast::VariableExpression> args;
    };
    
    class Function {
      public:
        Function(Prototype &proto, std::vector<Expression> body);
      private:
        Prototype &proto;
        std::vector<Expression> body;
    };

  } // ast 


  class ParserException {
    public:
      ParserException(std::string what, std::string faulty_expression);
      ParserException(std::string what, nukac::lexer::Literal literal);
      const char *what();
    private:
      std::string what_did_i_do;
  };

  enum class Scope {
    function,
    structure, // even a file is considered a structure,
               // ziglike.
    variable,
  };

  class Parser {
    public:
      // optional macro
      // PARSER_WHILE_CONDITIONAL
      Parser(lexer::Lexer &lexer);
      Parser(lexer::Lexer &lexer,
          std::unordered_map<std::string, ast::VariableExpression> variables,
          std::vector<ast::Expression> expressions, 
          std::vector<ast::Prototype> prototypes, 
          std::vector<ast::Function> functions, 
          std::unordered_map<std::string, ast::TypeExpression> parent_types,
          Scope scope);

      std::vector<ast::Expression> getExpressions();
      std::vector<ast::Prototype> getPrototypes();
      std::vector<ast::Function> getFunctions();
      const Scope getScope();

    private:
      lexer::Lexer &lexer;
      std::unordered_map<std::string, ast::VariableExpression> variables;
      std::vector<ast::Expression> expressions;
      std::vector<ast::Prototype> prototypes;
      std::vector<ast::Function> functions;

      std::unordered_map<std::string, ast::TypeExpression> scope_types;

      inline void parserInternal();
      inline void parserPFunction();
      inline void parserPStructure();
      inline void parserPReturn();
      inline void parserPVariable(std::string name);
      inline void parserPVariableAssign(std::string name);

      Scope scope;
  };

} // nukac::parser

#endif
