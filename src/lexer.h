#pragma once

#include "token.h"
#include <istream>
#include <optional>
#include <regex>
#include <vector>

class Lexer {
public:
  Lexer(std::istream &stream);
  bool eof();
  Token next();
  Token peek();

  std::optional<Token> lex_whitespace();
  std::optional<Token> lex_comment();
  std::optional<Token> lex_number();
  std::optional<Token> lex_newline();
  std::optional<Token> lex_identifier();
  std::optional<Token> lex_operator();
  std::optional<Token> lex_punctuation();

private:
  std::istream &stream;
};
