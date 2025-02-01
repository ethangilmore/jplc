#include <fstream>
#include <iostream>
#include <vector>
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "printervisitor.h"

struct Options {
  std::string input;
  bool lex;
  bool parse;
  bool typecheck;
};

int main(int argc, char* argv[]) {
  // std::cout << "Hello, World!" << std::endl;
  // exit(0);
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
  Logger logger(options.input);
  Lexer lexer(input, logger);
  if (options.lex) {
    Token token = lexer.next();
    std::cout << token.to_string() << std::endl;
    while (token.type != Token::Type::Eof) {
      token = lexer.next();
      std::cout << token.to_string() << std::endl;
    }
    std::cout << "Compilation succeeded" << std::endl;
    exit(0);
  }
  Parser parser(lexer, logger);
  if (options.parse) {
    std::unique_ptr<ASTNode> node = parser.parse();
    PrinterVisitor visitor;
    node->accept(visitor);
    std::cout << "\nCompilation succeeded" << std::endl;
    exit(0);
  }
}
