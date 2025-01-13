#pragma once

#include <string>

class Token {
public:
  enum class Type {
    Array,
    Assert,
    Bool,
    Else,
    False,
    Float,
    Fn,
    If,
    Image,
    Int,
    Let,
    Print,
    Read,
    Return,
    Show,
    Sum,
    Then,
    Time,
    To,
    True,
    Type,
    Write,
    Colon,
    LCurly,
    RCurly,
    LParen,
    RParen,
    Comma,
    LSquare,
    RSquare,
    Equals,
    String,
    IntVal,
    FloatVal,
    Variable,
    Op,
    Newline,
    Eof
  };

  Type type;
  int start;
  std::string value;

  Token(Type type, int start, char value);
  Token(Type type, int start, std::string value = "");

  std::string toString();
};
