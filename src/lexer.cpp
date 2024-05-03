#include <sstream>
#include <format>

#include "lexer.hpp"

using namespace nukac::lexer;

LexerException::LexerException(std::string what) {
  what_did_i_do = what;
}

LexerException::LexerException(std::string what, usize character, usize line) {
  what_did_i_do = std::format("{}:{}: {}", line, character, what);
}

const char *LexerException::what() {
  return what_did_i_do.c_str();
}

#define LEXER_PUSH_BACK(what) \
  if(!literals.empty()){ \
    literals.push_back({.character = token_at, .line = line_at, .literal = what}); \
  } \
  if(after_slash) { literals.push_back({ .character = token_at - 1, .line = line_at, .literal = Token::slash }); after_slash = false; } \
  if(after_dash) { literals.push_back({ .character = token_at - 1, .line = line_at, .literal = Token::dash }); after_dash = false; } 

#define LEXER_SWITCH(token) \
  LEXER_PUSH_BACK(reconstructed_string_literal); \
  literals.push_back({ .character = token_at, .line = line_at, .literal = token }); \
  continue; 

Lexer::Lexer(std::istream &is): is(is) {
  std::string reconstructed_string_literal;
  std::string line;
  literals_vector_at = 0;

  bool after_star_comment = false;
  while(std::getline(is, line)) {
    for(char c: line) {
      bool after_star = false;
      bool after_slash = false; 
      bool after_dash = false;
      token_at++;
      switch (c) {
        case '/': LEXER_PUSH_BACK(reconstructed_string_literal);
                  if(after_slash) {
                    after_slash = false;
                    break;
                  } else if(after_star) {
                    after_star = false;
                  } else {
                    after_slash = true;
                    continue;
                  }

        case '*': if (after_slash) {
                    after_slash = false;
                    after_star_comment = true;
                  } else if (after_star_comment) {
                    after_star = true;
                  } else {
                    LEXER_SWITCH(Token::star);
                  }

        case '>': LEXER_PUSH_BACK(reconstructed_string_literal) 
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
        case '"': LEXER_SWITCH(Token::dquote); 
        case '\'':  LEXER_SWITCH(Token::backslash); 
        case '<': LEXER_SWITCH(Token::left_inequality); 
        case ' ': case '\t': case ';':
                  token_at++;
        case '\n': line_at++; token_at = 0;
        
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
        case 't': case 'T':
        case 'u': case 'U':
        case 'v': case 'V':
        case 'w': case 'W':
        case 'x': case 'X':
        case 'y': case 'Y':
        case 'z': case 'Z':
        reconstructed_string_literal.push_back(c);

        default: throw LexerException(std::format("Invalid character {}", c), token_at, line_at);
      }
    }
  }
}

LiteralWithPosition Lexer::next(){
  return literals[literals_vector_at + 1];  
}

bool Lexer::next(Literal w){
  return (w == literals[literals_vector_at + 1].literal);
}

std::vector <LiteralWithPosition> Lexer::next(usize n) {
  std::vector <LiteralWithPosition> ls;
  for(usize m = 0; m < n; m++) {
    ls.push_back(literals[literals_vector_at + m]);
  }

  return ls;
}

LiteralWithPosition Lexer::swallow(){
  const LiteralWithPosition l = next();
  literals_vector_at++;
  return l;
}

bool Lexer::swallow(Literal w){
  const bool l = next(w);
  literals_vector_at++;
  return l;
}

std::vector <LiteralWithPosition> Lexer::swallow(usize n){
  const std::vector <LiteralWithPosition> ls = next(n);
  literals_vector_at += n;
  return ls;
}

void Lexer::swallowZ() {
  literals_vector_at++;
}

std::ostream &operator<<(std::ostream &output, LiteralWithPosition &literal) {
  output << std::format("{}:{}: {}", literal.character, literal.line, std::get<std::string>(literal.literal));
  return output;
}
