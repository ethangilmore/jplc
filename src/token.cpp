#include "token.h"

Token::Token(Type type, int start, std::string value)
    : type(type), start(start), value(value) {}

Token::Token(Type type, int start, char value)
    : type(type), start(start), value(std::string(1, value)) {}

std::string Token::toString() {
  switch (type) {
  case Type::Array:
    return "ARRAY '" + value + "'";
  case Type::Assert:
    return "ASSERT '" + value + "'";
  case Type::Bool:
    return "BOOL '" + value + "'";
  case Type::Else:
    return "ELSE '" + value + "'";
  case Type::False:
    return "FALSE '" + value + "'";
  case Type::Float:
    return "FLOAT '" + value + "'";
  case Type::Fn:
    return "FN '" + value + "'";
  case Type::If:
    return "IF '" + value + "'";
  case Type::Image:
    return "IMAGE '" + value + "'";
  case Type::Int:
    return "INT '" + value + "'";
  case Type::Let:
    return "LET '" + value + "'";
  case Type::Print:
    return "PRINT '" + value + "'";
  case Type::Read:
    return "READ '" + value + "'";
  case Type::Return:
    return "RETURN '" + value + "'";
  case Type::Show:
    return "SHOW '" + value + "'";
  case Type::Sum:
    return "SUM '" + value + "'";
  case Type::Then:
    return "THEN '" + value + "'";
  case Type::Time:
    return "TIME '" + value + "'";
  case Type::To:
    return "TO '" + value + "'";
  case Type::True:
    return "TRUE '" + value + "'";
  case Type::Type:
    return "TYPE '" + value + "'";
  case Type::Write:
    return "WRITE '" + value + "'";
  case Type::Colon:
    return "COLON '" + value + "'";
  case Type::LCurly:
    return "LCURLY '" + value + "'";
  case Type::RCurly:
    return "RCURLY '" + value + "'";
  case Type::LParen:
    return "LPAREN '" + value + "'";
  case Type::RParen:
    return "RPAREN '" + value + "'";
  case Type::Comma:
    return "COMMA '" + value + "'";
  case Type::LSquare:
    return "LSQUARE '" + value + "'";
  case Type::RSquare:
    return "RSQUARE '" + value + "'";
  case Type::Equals:
    return "EQUALS '" + value + "'";
  case Type::String:
    return "STRING '" + value + "'";
  case Type::IntVal:
    return "INTVAL '" + value + "'";
  case Type::FloatVal:
    return "FLOATVAL '" + value + "'";
  case Type::Variable:
    return "VARIABLE '" + value + "'";
  case Type::Op:
    return "OP '" + value + "'";
  case Type::Newline:
    return "NEWLINE";
  case Type::Eof:
    return "END_OF_FILE";
  }
};
