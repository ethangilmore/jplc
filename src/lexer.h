#pragma once

#include "token.h"
#include <istream>
#include <optional>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Lexer {
public:
  Lexer(std::istream &stream);
  Token next();
  Token peek();

  std::optional<Token> lex_whitespace();
  std::optional<Token> lex_newline();
  std::optional<Token> lex_operator();
  std::optional<Token> lex_string();
  std::optional<Token> lex_number();
  std::optional<Token> lex_punctuation();
  std::optional<Token> lex_keyword();
  std::optional<Token> lex_identifier();
  std::optional<Token> lex_eof();

private:
  static const std::unordered_map<std::string, Token::Type> keywords;
  static const std::unordered_map<std::string, Token::Type> punctuations;
  static const std::unordered_set<std::string> operators;
  static const std::vector<std::optional<Token> (Lexer::*)()> lexemes;

  std::istream &stream;
};
