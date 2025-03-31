#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include "asmgenvisitor.h"
#include "codegenvisitor.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "printervisitor.h"
#include "typecheckervisitor.h"
// #include "typedefvisitor.h"

struct Options {
  std::string input;
  bool lex;
  bool parse;
  bool c;
  bool assembly;
  bool typecheck;
};

int main(int argc, char *argv[]) {
  // Context ctx;
  // auto type = std::make_shared<Int>();
  // auto info = std::make_shared<ValueInfo>("asdf", type);
  // ctx.add(info);
  //
  // if (auto i = ctx.lookup<ValueInfo>("asdf")) {
  //   std::cout << "yay" << std::endl;
  // } else {
  //   std::cout << "uh oh" << std::endl;
  // }

  // std::cout << "Hello, World!" << std::endl;
  // exit(0);

  std::vector<std::string> args(argv + 1, argv + argc);
  Options options = {
      .input = args[0],
      .lex = std::find(args.begin(), args.end(), "-l") != args.end(),
      .parse = std::find(args.begin(), args.end(), "-p") != args.end(),
      .c = std::find(args.begin(), args.end(), "-i") != args.end(),
      .assembly = std::find(args.begin(), args.end(), "-s") != args.end(),
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
  std::unique_ptr<Program> program = parser.parse();
  TypeCheckerVisitor typechecker(logger);
  program->accept(typechecker);
  if (options.parse) {
    PrinterVisitor visitor;
    program->accept(visitor);
    std::cout << "\nCompilation succeeded" << std::endl;
    exit(0);
  }
  if (options.c) {
    CodeGenVisitor generator(typechecker.ctx, logger);
    program->accept(generator);
    std::cout << "\nCompilation succeeded" << std::endl;
    exit(0);
  } else if (options.assembly) {
    ASMGenVisitor generator(typechecker.ctx, logger);
    program->accept(generator);
    std::cout << "\nCompilation succeeded" << std::endl;
    exit(0);
  }
}
