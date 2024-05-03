#include <fstream>

#include "lexer.hpp"
#include "helper.hpp"

int main(int argc, char *argv[]){
  try {
    std::ifstream ifs(argv[0]);
    nukac::lexer::Lexer ll(ifs);
  } catch (nukac::lexer::LexerException &e) {
    nukac::helper::exceptionHandler(e.what());
  }
}
