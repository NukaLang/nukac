#ifndef NUKAC_LEXER_HPP
#define NUKAC_LEXER_HPP

#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "helper.hpp"

namespace nukac {
  namespace lexer {
    class LexerException {
      public:
        LexerException(std::string what);
        LexerException(std::string what, usize character, usize line);
        const char *what();
      private:
        std::string what_did_i_do;
    }; // LexerException
    
    enum class Token {
      exclamation,
      question,
      pipe,
      ampersand,
      dollar,
      percent,
      slash,
      star,
      plus,
      dash,
      equals,
      backslash,
      dot,
      comma,
      colon,
      lparen,
      rparen,
      lsqbrace,
      rsqbrace,
      lcrbrace,
      rcrbrace,
      dquote,
      squote,
      left_inequality,
      right_inequality,

      arrow,
    }; // Token

    using Literal = std::variant <
      std::string,
      size,
      Token
    >;

    struct LiteralWithPosition {
      usize character;
      usize line;
      Literal literal;
    };

    std::ostream &operator<<(std::ostream &output, LiteralWithPosition &literal);

    class Lexer {
      public:
        Lexer(std::istream &is);

        LiteralWithPosition next();
        bool next(Literal w);
        std::vector <LiteralWithPosition> next(usize n);

        LiteralWithPosition swallow();
        std::vector <LiteralWithPosition> swallow(usize n);
        bool swallow(Literal w);

        void swallowZ();

      private:
        std::vector <LiteralWithPosition> literals;
        std::istream &is;
        
        usize token_at;
        usize line_at;

        usize literals_vector_at;
    }; // Lexer
  } // lexer
} // nukac 

#endif // NUKAC_LEXER_HPP
