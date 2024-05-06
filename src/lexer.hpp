#ifndef NUKAC_LEXER_HPP
#define NUKAC_LEXER_HPP

#include <functional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "helper.hpp"

namespace nukac::lexer {
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

  using LiteralVariant = std::variant <
    std::string,
    Token
  >;

  struct Literal {
    usize character;
    usize line;
    LiteralVariant literal_variant;
  };

  std::ostream &operator<<(std::ostream &output, Literal &literal);
  bool operator==(const std::string &s, const LiteralVariant &l) noexcept;
  bool operator==(const Token &t, const LiteralVariant &l) noexcept;

  class Lexer {
    public:
      Lexer(std::istream &is);

      Literal next();
      bool next(LiteralVariant w);
      std::vector <Literal> next(usize n);

      Literal swallow();
      std::vector <Literal> swallow(usize n);
      bool swallow(LiteralVariant w);

      void swallowZ();

    private:
      std::vector <Literal> literals;
      std::istream &is;
      
      usize token_at;
      usize line_at;

      usize literals_vector_at;
  }; // Lexer

} // nukac::lexer 

#endif // NUKAC_LEXER_HPP
