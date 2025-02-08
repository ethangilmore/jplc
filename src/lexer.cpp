#include "lexer.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "token.h"

const std::unordered_map<std::string, Token::Type> Lexer::keywords{
    {"array", Token::Type::Array},
    {"assert", Token::Type::Assert},
    {"bool", Token::Type::Bool},
    {"else", Token::Type::Else},
    {"false", Token::Type::False},
    {"float", Token::Type::Float},
    {"fn", Token::Type::Fn},
    {"if", Token::Type::If},
    {"image", Token::Type::Image},
    {"int", Token::Type::Int},
    {"let", Token::Type::Let},
    {"print", Token::Type::Print},
    {"read", Token::Type::Read},
    {"return", Token::Type::Return},
    {"show", Token::Type::Show},
    {"struct", Token::Type::Struct},
    {"sum", Token::Type::Sum},
    {"then", Token::Type::Then},
    {"time", Token::Type::Time},
    {"to", Token::Type::To},
    {"true", Token::Type::True},
    {"void", Token::Type::Void},
    {"write", Token::Type::Write},
};

const std::unordered_map<std::string, Token::Type> Lexer::punctuations{
    {":", Token::Type::Colon},
    {"{", Token::Type::LCurly},
    {"}", Token::Type::RCurly},
    {"(", Token::Type::LParen},
    {")", Token::Type::RParen},
    {",", Token::Type::Comma},
    {"[", Token::Type::LSquare},
    {"]", Token::Type::RSquare},
    {"=", Token::Type::Equals},
    {".", Token::Type::Dot}};

const std::unordered_set<std::string> Lexer::operators = {"+", "-", "*", "/", "<", ">", "%", "!", "&&", "==", "!=", "<=", ">="};

const std::vector<std::optional<Token> (Lexer::*)()> Lexer::lexemes = {
    &Lexer::lex_whitespace,
    &Lexer::lex_newline,
    &Lexer::lex_operator,
    &Lexer::lex_string,
    &Lexer::lex_number,
    &Lexer::lex_punctuation,
    &Lexer::lex_keyword,
    &Lexer::lex_identifier,
    &Lexer::lex_eof};

Lexer::Lexer(std::istream& stream, Logger& logger)
    : stream(stream), logger(logger) {}

Token Lexer::peek() {
  if (!peeked.has_value()) {
    peeked = next();
  }
  return peeked.value();
}

Token Lexer::next() {
  if (peeked.has_value()) {
    Token token = peeked.value();
    peeked = std::nullopt;
    return token;
  }
  for (auto lexeme : lexemes) {
    auto token = (this->*lexeme)();
    if (token.has_value()) {
      return token.value();
    }
  }
  logger.log_error("Unexpected character", stream.tellg());
}

std::optional<Token> Lexer::lex_whitespace() {
  std::optional<Token> token = std::nullopt;
  while (true) {
    bool consume = false;
    int64_t start = stream.tellg();
    char c = stream.get();
    if (c == ' ') {  // whitespace
      continue;
    } else if (c == '\n') {
      token = Token{Token::Type::NewLine, start};
      consume = true;
    } else if (c == '\\' && stream.peek() == '\n') {  // escaped newline
      stream.ignore();
    } else if (c == '/' && stream.peek() == '/') {  // line comment
      while (stream.peek() != '\n') {
        stream.ignore();  // TODO: handle EOF
      }
    } else if (c == '/' && stream.peek() == '*') {  // block comment
      stream.ignore();
      while (true) {
        if (stream.eof()) {
          logger.log_error("Unterminated block comment", start);
        }
        char c = stream.get();
        if ((c < 32 || c > 126) && c != '\n') {
          logger.log_error("Invalid character in block comment", start);
        }
        if (c != '*') {
          continue;
        }
        if (stream.get() == '/') {
          break;
        }
      }
    } else {
      if (!consume) {
        stream.seekg(start);
      }
      return token;
    }
  }
  return std::nullopt;
}

std::optional<Token> Lexer::lex_newline() {
  int64_t start = stream.tellg();
  if (stream.peek() == '\n') {
    while (stream.peek() == '\n') {
      stream.ignore();
    }
    return Token{Token::Type::NewLine, start};
  }
  return std::nullopt;
}

std::optional<Token> Lexer::lex_operator() {
  char buffer[2];
  int64_t start = stream.tellg();
  stream.read(buffer, 2);
  for (int len : {2, 1}) {  // try 2-character operator first
    if (operators.count(std::string(buffer, len))) {
      stream.seekg(start + len);
      return Token{Token::Type::Op, start, std::string(buffer, len)};
    }
  }
  stream.seekg(start);
  return std::nullopt;
}

std::optional<Token> Lexer::lex_string() {
  int64_t start = stream.tellg();
  std::string value = "";
  if (stream.peek() != '"') {
    return std::nullopt;
  }
  value += stream.get();
  while (stream.peek() != '"') {
    char c = stream.get();
    if (c < 32 || c > 126) {
      logger.log_error("Invalid character in string", start);
    } else if (stream.eof()) {
      logger.log_error("Unterminated string", start);
    }
    value += c;
  }
  value += stream.get();
  return Token{Token::Type::String, start, value};
}

std::optional<Token> Lexer::lex_number() {
  int64_t start = stream.tellg();
  std::string pre, post;
  while (std::isdigit(stream.peek())) {
    pre += stream.get();
  }
  if (stream.peek() == '.') {
    post += stream.get();
    while (std::isdigit(stream.peek())) {
      post += stream.get();
    }
    if (pre.empty() && post.size() == 1) {
      stream.seekg(start);
      return std::nullopt;
    }
    return Token{Token::Type::FloatVal, start, pre + post};
  } else if (pre.size() > 0) {
    return Token{Token::Type::IntVal, start, pre};
  }
  return std::nullopt;
}

std::optional<Token> Lexer::lex_punctuation() {
  int64_t start = stream.tellg();
  std::string value = std::string(1, stream.get());
  if (punctuations.count(value)) {
    return Token{punctuations.at(value), start, value};
  }
  stream.seekg(start);
  return std::nullopt;
}

std::optional<Token> Lexer::lex_keyword() {
  int64_t start = stream.tellg();
  std::string value = "";
  while (std::isalnum(stream.peek()) || stream.peek() == '_') {
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

  int64_t start = stream.tellg();
  std::string value;
  while (std::isalnum(stream.peek()) || stream.peek() == '_') {
    value += stream.get();
  }
  if (value.empty()) {
    return std::nullopt;
  }
  return Token{Token::Type::Variable, start, value};
}

std::optional<Token> Lexer::lex_eof() {
  if (stream.peek() == EOF || stream.eof()) {
    return Token{Token::Type::Eof, stream.tellg()};
  }
  return std::nullopt;
}
