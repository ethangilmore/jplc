#pragma once

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
  int start;
  std::string value;

  Token(Type type, int start, char value);
  Token(Type type, int start, std::string value = "");

  std::string toString();
};
