#include <format>
#include <iostream>

#include "lexer.hpp"

namespace nukac::lexer {
  LexerException::LexerException(std::string what) {
    what_did_i_do = what;
  }

  LexerException::LexerException(std::string what, usize character, usize line) {
    what_did_i_do = std::format("{}:{}: {}", line, character, what);
  }

  const char *LexerException::what() {
    return what_did_i_do.c_str();
  }

#define LEXER_PUSH_BACK() \
    if(after_star) continue;  \
    if(string_literal) { \
      reconstructed_string_literal.push_back(c); continue; \
    } \
    if(!reconstructed_string_literal.empty()){ \
      literals.push_back({.character = token_at, .line = line_at, .literal_variant = reconstructed_string_literal}); \
      literals.clear(); \
    } \
    if(after_slash) { literals.push_back({ .character = token_at - 1, .line = line_at, .literal_variant = Token::slash }); after_slash = false; } \
    if(after_dash) { literals.push_back({ .character = token_at - 1, .line = line_at, .literal_variant = Token::dash }); after_dash = false; } \

#define LEXER_SWITCH(token) \
    LEXER_PUSH_BACK(); \
    literals.push_back({ .character = token_at, .line = line_at, .literal_variant = token }); \
    continue; 

  Lexer::Lexer(std::istream &is): is(is) {
    std::string reconstructed_string_literal;
    std::string line;
    literals_vector_at = 0;
    token_at = 0;
    line_at = 0;

    bool after_star_comment = false;
    while(std::getline(is, line)) {
      for(char c: line) {
        bool after_star = false;
        bool after_slash = false; 
        bool after_dash = false;
        bool string_literal = false;
        token_at++;
        switch (c) {
          case '/': LEXER_PUSH_BACK();
                    if(after_slash) {
                      after_slash = false;
                      break;
                    } else if(after_star) {
                      after_star = false;
                      continue;
                    } else {
                      after_slash = true;
                      continue;
                    }

          case '*': if (after_slash) {
                      after_slash = false;
                      after_star_comment = true;
                      continue;
                    } else if (after_star_comment) {
                      after_star = true;
                      continue;
                    } else {
                      LEXER_SWITCH(Token::star);
                      continue;
                    }

          case '>': LEXER_PUSH_BACK() 
                    if(after_dash == true) {
                      LEXER_SWITCH(Token::arrow);
                      after_dash = false;
                    } else LEXER_SWITCH(Token::right_inequality);
                    continue;

          case '!': LEXER_SWITCH(Token::exclamation); 
          case '?': LEXER_SWITCH(Token::question);
          case '|': LEXER_SWITCH(Token::pipe); 
          case '&': LEXER_SWITCH(Token::ampersand); 
          case '$': LEXER_SWITCH(Token::dollar);
          case '%': LEXER_SWITCH(Token::percent); 
          case '+': LEXER_SWITCH(Token::plus);
          case '-': after_dash = true; continue;
          case '=': LEXER_SWITCH(Token::equals);
          case '\\': LEXER_SWITCH(Token::backslash);
          case '.': LEXER_SWITCH(Token::dot); 
          case ',': LEXER_SWITCH(Token::comma); 
          case ':': LEXER_SWITCH(Token::comma)
          case '(': LEXER_SWITCH(Token::lparen); 
          case ')': LEXER_SWITCH(Token::rparen);
          case '[': LEXER_SWITCH(Token::lsqbrace); 
          case ']': LEXER_SWITCH(Token::rsqbrace); 
          case '{': LEXER_SWITCH(Token::lcrbrace); 
          case '}': LEXER_SWITCH(Token::rcrbrace); 
          case '"': LEXER_PUSH_BACK();
                    if(!string_literal) string_literal = true;
                    else if(string_literal) string_literal = false;
                     
          case '\'':  LEXER_SWITCH(Token::backslash); 
          case '<': LEXER_SWITCH(Token::left_inequality); 
          case ' ': case '\t': case ';':
                    token_at++; continue;
          case '\n': line_at++; token_at = 0; continue;
          
          case 'a': case 'A':
          case 'b': case 'B':
          case 'c': case 'C':
          case 'd': case 'D':
          case 'e': case 'E':
          case 'f': case 'F':
          case 'g': case 'G':
          case 'h': case 'H':
          case 'i': case 'I':
          case 'j': case 'J':
          case 'k': case 'K':
          case 'l': case 'L':
          case 'm': case 'M':
          case 'n': case 'N':
          case 'o': case 'O':
          case 'p': case 'P':
          case 'r': case 'R':
          case 's': case 'S':
          case 't': case 'T':
          case 'u': case 'U':
          case 'v': case 'V':
          case 'w': case 'W':
          case 'x': case 'X':
          case 'y': case 'Y':
          case 'z': case 'Z':
          reconstructed_string_literal.push_back(c); continue;
          
          default: throw LexerException(std::format("Invalid character {}", c), token_at, line_at);
        }
      }
    }
  }

  Literal Lexer::next(){
    return literals[literals_vector_at + 1];  
  }

  bool Lexer::next(LiteralVariant w){
    return (w == literals[literals_vector_at + 1].literal_variant);
  }

  std::vector <Literal> Lexer::next(usize n) {
    std::vector <Literal> ls;
    for(usize m = 0; m < n; m++) {
      ls.push_back(literals[literals_vector_at + m]);
    }

    return ls;
  }
  
  Literal Lexer::swallow(){
    const Literal l = next();
    literals_vector_at++;
    return l;
  }
  
  bool Lexer::swallow(LiteralVariant w){
    const bool l = next(w);
    literals_vector_at++;
    return l;
  }

  std::vector <Literal> Lexer::swallow(usize n){
    const std::vector <Literal> ls = next(n);
    literals_vector_at += n;
    return ls;
  }

  void Lexer::swallowZ() {
    literals_vector_at++;
  }

  bool Lexer::isEoC() {
    if(literals_vector_at == literals.size()) return true;
    else return false;
  }

  std::ostream &operator<<(std::ostream &output, Literal &literal) {
    output << std::format("{}:{}: {}", literal.character, literal.line, std::get<std::string>(literal.literal_variant));
    return output;
  }

  bool operator==(const std::string &s, const LiteralVariant &l) noexcept {
    try {
      const std::string &sr = std::get<std::string>(l);
      return s == sr;
    } catch(...) {
      return false;
    }
  }
  
  bool operator==(const Token &t, const LiteralVariant &l) noexcept {
    try {
      const Token &tr = std::get<Token>(l);
      return t == tr;
    } catch(...) {
      return false;
    }
  }
} // nukac::lexer
