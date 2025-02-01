#pragma once

#include <cstdint>
#include <string>

class Token {
 public:
  enum class Type {
    Array,
    Assert,
    Bool,
    Colon,
    Comma,
    Dot,
    Else,
    Eof,
    Equals,
    False,
    Float,
    FloatVal,
    Fn,
    If,
    Image,
    Int,
    IntVal,
    LCurly,
    Let,
    LParen,
    LSquare,
    NewLine,
    Op,
    Print,
    RCurly,
    Read,
    Return,
    RParen,
    RSquare,
    Show,
    String,
    Struct,
    Sum,
    Then,
    Time,
    To,
    True,
    Variable,
    Void,
    Write,
  };

  Type type;
  int64_t start;
  std::string value;

  Token(Type type, int64_t start, char value);
  Token(Type type, int64_t start, std::string value = "");

  std::string to_string() const;
};
