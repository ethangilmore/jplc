#include "lexer.h"
#include "token.h"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

const std::unordered_map<std::string, Token::Type> Lexer::keywords{
    {"array", Token::Type::Array}, {"assert", Token::Type::Assert},
    {"bool", Token::Type::Bool},   {"else", Token::Type::Else},
    {"false", Token::Type::False}, {"float", Token::Type::Float},
    {"fn", Token::Type::Fn},       {"if", Token::Type::If},
    {"image", Token::Type::Image}, {"int", Token::Type::Int},
    {"let", Token::Type::Let},     {"print", Token::Type::Print},
    {"read", Token::Type::Read},   {"return", Token::Type::Return},
    {"show", Token::Type::Show},   {"struct", Token::Type::Struct},
    {"sum", Token::Type::Sum},     {"then", Token::Type::Then},
    {"time", Token::Type::Time},   {"to", Token::Type::To},
    {"true", Token::Type::True},   {"void", Token::Type::Void},
    {"write", Token::Type::Write},
};

const std::unordered_map<std::string, Token::Type> Lexer::punctuations{
    {":", Token::Type::Colon},   {"{", Token::Type::LCurly},
    {"}", Token::Type::RCurly},  {"(", Token::Type::LParen},
    {")", Token::Type::RParen},  {",", Token::Type::Comma},
    {"[", Token::Type::LSquare}, {"]", Token::Type::RSquare},
    {"=", Token::Type::Equals}};

const std::unordered_set<std::string> Lexer::operators = {
    "+", "-", "*", "/", "==", "!=", "<", ">", "<=", ">=", "="};

const std::vector<std::optional<Token> (Lexer::*)()> Lexer::lexemes = {
    &Lexer::lex_whitespace, &Lexer::lex_newline, &Lexer::lex_punctuation,
    &Lexer::lex_operator,   &Lexer::lex_number,  &Lexer::lex_keyword,
    &Lexer::lex_identifier, &Lexer::lex_string};

Lexer::Lexer(std::istream &stream) : stream(stream) {}

bool Lexer::eof() { return stream.eof(); }

Token Lexer::next() {
  for (auto lexeme : lexemes) {
    auto token = (this->*lexeme)();
    if (token.has_value()) {
      return token.value();
    }
  }

  if (stream.eof()) {
    return Token(Token::Type::Eof, stream.tellg());
  }
  std::cout << "Compilation failed" << std::endl;
  exit(1);
}

std::optional<Token> Lexer::lex_whitespace() {
  while (true) {
    int start = stream.tellg();
    int c = stream.get();
    if (c == ' ') { // whitespace
      continue;
    } else if (c == '\\' && stream.peek() == '\n') { // escaped newline
      stream.ignore();
    } else if (c == '/' && stream.peek() == '/') { // line comment
      while (stream.get() != '\n') {
        // stream.ignore(); // TODO: handle EOF
      }
    } else if (c == '/' && stream.peek() == '*') { // block comment
      stream.ignore();
      while (true) {
        if (stream.eof()) {
          std::cout << "Compilation failed" << std::endl;
          exit(1);
        }
        if (stream.get() != '*') {
          continue;
        }
        if (stream.get() == '/') {
          break;
        }
      }
    } else {
      stream.seekg(start);
      return std::nullopt;
    }
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
    while (true) {
      if (stream.get() != '*') {
        continue;
      }
      if (stream.get() == '/') {
        break;
      }
    }
    break;
  default: // not a comment
    stream.seekg(start);
  }

  return std::nullopt;
}

std::optional<Token> Lexer::lex_punctuation() {
  int start = stream.tellg();
  std::string value = std::string(1, stream.get());
  if (punctuations.count(value)) {
    return Token{punctuations.at(value), start, value};
  }
  stream.seekg(start);
  return std::nullopt;
}

std::optional<Token> Lexer::lex_string() {
  int start = stream.tellg();
  std::string value = "";
  if (stream.peek() != '"') {
    return std::nullopt;
  }
  value += stream.get();
  while (stream.peek() != '"') {
    char c = stream.get();
    if (c < 32 || c > 126 || stream.eof()) {
      std::cout << "Compilation failed" << std::endl;
      exit(1);
    }
    value += c;
  }
  value += stream.get();
  return Token{Token::Type::String, start, value};
}

std::optional<Token> Lexer::lex_operator() {
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
  int start = stream.tellg();
  switch (stream.get()) {
  case '\n':
    while (stream.peek() == '\n') {
      stream.ignore();
    }
    return Token{Token::Type::NewLine, start};
  case '\\':
    if (stream.get() == '\n') {
      return std::nullopt;
    }
  default:
    stream.seekg(start);
    return std::nullopt;
  }
}

std::optional<Token> Lexer::lex_keyword() {
  int start = stream.tellg();
  std::string value;
  while (std::isalpha(stream.peek())) {
    value += stream.get();
  }
  if (keywords.count(value)) {
    return Token{keywords.at(value), start, value};
  }
  stream.seekg(start);
  return std::nullopt;
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
  value += stream.get();
  while (std::isdigit(stream.peek())) {
    value += stream.get();
  }
  return Token(Token::Type::FloatVal, start, value);
}
