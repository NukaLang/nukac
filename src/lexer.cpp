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
    if(after_star) continue; \
    if(!reconstructed_string_literal.empty()){ \
      literals.push_back({.where_character = token_at, .where_line = line_at, .literal_token = Token::string, .literal_string = reconstructed_string_literal}); \
      literals.clear(); \
    } \
    if(after_slash) { literals.push_back({ .where_character = token_at - 1, .where_line = line_at, .literal_token = Token::slash }); after_slash = false; } \
    if(after_dash) { literals.push_back({ .where_character = token_at - 1, .where_line = line_at, .literal_token = Token::dash }); after_dash = false; } \
    if(after_backslash) { after_backslash = false; } \

#define LEXER_SWITCH(token) \
    LEXER_PUSH_BACK(); \
    literals.push_back({ .where_character = token_at, .where_line = line_at, .literal_token = token }); \
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
        bool after_backslash = false;
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
          case '\\': after_backslash = true; continue;
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
                    else if(string_literal && !after_slash) { 
                      string_literal = false;
                      literals.push_back({
                          .where_character = token_at,
                          .where_line = line_at,
                          .literal_token = Token::quoted,
                          .literal_string = reconstructed_string_literal
                      });
                    }
                    else if(string_literal && after_slash) {
                      after_slash = false;
                      string_literal = false;
                      reconstructed_string_literal.push_back(c);
                    }
                    continue;
          case '\'':  LEXER_SWITCH(Token::backslash); 
          case '<': LEXER_SWITCH(Token::left_inequality); 
          case ' ': case '\t':
                    token_at++; continue;
          case ';': LEXER_SWITCH(Token::semicolon);
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

  bool Lexer::next(Token w){
    return (w == literals[literals_vector_at + 1].literal_token);
  }
  
  bool Lexer::next(std::string w){
    return (literals[literals_vector_at + 1].isString() && w == literals[literals_vector_at + 1].literal_string);
  }
    
  Literal Lexer::swallow(){
    const Literal l = next();
    literals_vector_at++;
    return l;
  }
  
  bool Lexer::swallow(Token w){
    const bool l = next(w);
    literals_vector_at++;
    return l;
  }
  
  bool Lexer::swallow(std::string w) {
    const bool l = next(w);
    literals_vector_at++;
    return l;

  }
  
  void Lexer::swallowZ() {
    literals_vector_at++;
  }

  bool Lexer::isEoC() {
    if(literals_vector_at == literals.size()) return true;
    else return false;
  }


  bool operator==(const std::string &s, Literal &l) noexcept {
    return (l.isString() && s == l.literal_string); 
  }
  
  bool operator==(const Token &t, Literal &l) noexcept {
    return t == l.literal_token;  
  }

  const bool Literal::isString() noexcept {
    return literal_token == Token::string; 
  }

  const bool Literal::isQuoted() noexcept {
    return literal_token == Token::quoted;
  }
} // nukac::lexer
