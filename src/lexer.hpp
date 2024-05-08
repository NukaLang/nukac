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
    semicolon,

    arrow,

    string,
    quoted,
  }; // Token

  struct Literal {
    usize       where_character;
    usize       where_line;
    Token       literal_token;
    std::string literal_string;

    const bool isString() noexcept;
    const bool isQuoted() noexcept;
  };

  std::ostream &operator<<(std::ostream &output, Literal &literal);

  class Lexer {
    public:
      Lexer(std::istream &is);

      Literal next();
      bool next(Token w);
      bool next(std::string w);

      Literal swallow();
      bool swallow(Token w);
      bool swallow(std::string w);
      void swallowZ();

      bool isEoC();

    private:
      std::vector <Literal> literals;
      std::istream &is;
      
      usize token_at;
      usize line_at;

      usize literals_vector_at;
  }; // Lexer

} // nukac::lexer 

#endif // NUKAC_LEXER_HPP
