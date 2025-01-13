#include "lexer.h"
#include <fstream>
#include <iostream>
#include <vector>

struct Options {
  std::string input;
  bool lex;
  bool parse;
  bool typecheck;
};

int main(int argc, char *argv[]) {
  std::vector<std::string> args(argv + 1, argv + argc);
  Options options = {
      .input = args[0],
      .lex = std::find(args.begin(), args.end(), "-l") != args.end(),
      .parse = std::find(args.begin(), args.end(), "-p") != args.end(),
      .typecheck = std::find(args.begin(), args.end(), "-t") != args.end(),
  };

  if (options.lex + options.parse + options.typecheck > 1) {
    std::cerr << "Error: only one of -l, -p, -t can be specified" << std::endl;
    return 1;
  }

  std::ifstream input(options.input);
  Lexer lexer(input);
  while (!lexer.eof()) {
    Token token = lexer.next();
    std::cout << token.toString() << std::endl;
  }
}
