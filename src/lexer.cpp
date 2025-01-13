#include "lexer.h"
#include "token.h"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

Lexer::Lexer(std::istream &stream) : stream(stream) {}

bool Lexer::eof() { return stream.eof(); }

Token Lexer::next() {
  std::vector<std::optional<Token> (Lexer::*)()> lexemes = {
      &Lexer::lex_whitespace,  &Lexer::lex_newline,  &Lexer::lex_comment,
      &Lexer::lex_punctuation, &Lexer::lex_operator, &Lexer::lex_number,
      &Lexer::lex_identifier};

  for (auto lexeme : lexemes) {
    auto token = (this->*lexeme)();
    if (token.has_value()) {
      return token.value();
    }
  }

  if (stream.eof()) {
    return Token(Token::Type::Eof, stream.tellg());
  }
  exit(1);
}

std::optional<Token> Lexer::lex_whitespace() {
  while (stream.peek() == ' ') {
    stream.ignore();
  }

  return std::nullopt;
}

std::optional<Token> Lexer::lex_comment() {
  int start = stream.tellg();

  if (stream.get() != '/') {
    stream.seekg(start);
    return std::nullopt;
  }

  switch (stream.get()) {
  case '/': // line comment
    while (stream.peek() != '\n') {
      stream.ignore(); // TODO: handle EOF
    }
    break;
  case '*': // block comment
    while (stream.get() != '*' || stream.peek() != '/') {
      stream.ignore(); // TODO: handle EOF
    }
    break;
  default: // not a comment
    stream.seekg(start);
  }

  return std::nullopt;
}

std::optional<Token> Lexer::lex_punctuation() {
  int start = stream.tellg();
  char value = stream.get();
  switch (value) {
  case ':':
    return Token{Token::Type::Colon, start, value};
  case '{':
    return Token{Token::Type::LCurly, start, value};
  case '}':
    return Token{Token::Type::RCurly, start, value};
  case '(':
    return Token{Token::Type::LParen, start, value};
  case ')':
    return Token{Token::Type::RParen, start, value};
  case ',':
    return Token{Token::Type::Comma, start, value};
  case '[':
    return Token{Token::Type::LSquare, start, value};
  case ']':
    return Token{Token::Type::RSquare, start, value};
  default:
    stream.seekg(start);
    return std::nullopt;
  }
}

std::optional<Token> Lexer::lex_operator() {
  std::unordered_set<std::string> operators = {
      "+", "-", "*", "/", "==", "!=", "<", ">", "<=", ">=", "="};

  char buffer[2];
  int start = stream.tellg();
  stream.read(buffer, 2);
  for (int len : {2, 1}) { // try 2-character operator first
    if (operators.count(std::string(buffer, len))) {
      stream.seekg(start + len);
      return Token{Token::Type::Op, start, std::string(buffer, len)};
    }
  }
  stream.seekg(start);
  return std::nullopt;
}

std::optional<Token> Lexer::lex_newline() {
  if (stream.peek() != '\n') {
    return std::nullopt;
  }

  int start = stream.tellg();
  while (stream.peek() == '\n') {
    stream.ignore();
  }
  return Token{Token::Type::Newline, start};
}

std::optional<Token> Lexer::lex_identifier() {
  if (!std::isalpha(stream.peek()) && stream.peek() != '_') {
    return std::nullopt;
  }

  int start = stream.tellg();
  std::string value;
  while (std::isalnum(stream.peek()) || stream.peek() == '_') {
    value += stream.get();
  }
  if (value.empty()) {
    return std::nullopt;
  }
  return Token{Token::Type::Variable, start, value};
}

std::optional<Token> Lexer::lex_number() {
  if (!std::isdigit(stream.peek())) {
    return std::nullopt;
  }

  int start = stream.tellg();
  std::string value;
  while (std::isdigit(stream.peek())) {
    value += stream.get();
  }
  if (stream.peek() != '.') {
    return Token{Token::Type::IntVal, start, value};
  }
  while (std::isdigit(stream.peek())) {
    value += stream.get();
  }
  return Token(Token::Type::FloatVal, start, value);
}
