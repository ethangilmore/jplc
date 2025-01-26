#include "parser.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

Parser::Parser(Lexer &lexer, Logger &logger) : lexer(lexer), logger(logger) {}

Token Parser::consume(Token::Type type) {
  Token token = lexer.next();
  if (token.type != type) {
    logger.log_error("Unexpected token: " + token.value, token.start);
  }
  return token;
}

Token Parser::consume(Token::Type type, const std::string &value) {
  Token token = lexer.next();
  if (token.type != type || token.value != value) {
    logger.log_error("Unexpected token: " + token.value, token.start);
  }
  return token;
}

std::unique_ptr<Program> Parser::parse() {
  std::vector<std::unique_ptr<Cmd>> cmds;
  Token token = lexer.next();
  while (token.type != Token::Type::Eof) {
    if (token.type == Token::Type::NewLine) {
      token = lexer.next();
      continue;
    }
    cmds.push_back(parse_cmd(token));
    consume(Token::Type::NewLine);
    token = lexer.next();
  }
  return std::make_unique<Program>(std::move(cmds));
}

std::unique_ptr<Cmd> Parser::parse_cmd(Token token) {
  switch (token.type) {
  case Token::Type::Read:
    return parse_read_cmd(token);
  case Token::Type::Write:
    return parse_write_cmd(token);
  case Token::Type::Let:
    return parse_let_cmd(token);
  case Token::Type::Assert:
    return parse_assert_cmd(token);
  case Token::Type::Print:
    return parse_print_cmd(token);
  case Token::Type::Show:
    return parse_show_cmd(token);
  case Token::Type::Time:
    return parse_time_cmd(token);
  default:
    logger.log_error("Unexpected token: " + token.value, token.start);
  }
}

std::unique_ptr<LValue> Parser::parse_lvalue(Token token) {
  switch (token.type) {
  case Token::Type::Variable:
    return parse_var_lvalue(token);
  default:
    logger.log_error("Unexpected token: " + token.value, token.start);
  }
}

std::unique_ptr<Expr> Parser::parse_expr(Token token) {
  switch (token.type) {
  case Token::Type::IntVal:
    return parse_int_expr(token);
  case Token::Type::FloatVal:
    return parse_float_expr(token);
  case Token::Type::True:
    return parse_true_expr(token);
  case Token::Type::False:
    return parse_false_expr(token);
  case Token::Type::Variable:
    return parse_var_expr(token);
  case Token::Type::LSquare:
    return parse_array_literal_expr(token);
  default:
    logger.log_error("Unexpected token: " + token.value, token.start);
  }
}

std::unique_ptr<ReadCmd> Parser::parse_read_cmd(Token token) {
  consume(Token::Type::Image);
  std::string string = consume(Token::Type::String).value;
  consume(Token::Type::To);
  std::unique_ptr<VarLValue> lvalue = parse_var_lvalue(lexer.next());
  return std::make_unique<ReadCmd>(std::string(string), std::move(lvalue));
}

std::unique_ptr<WriteCmd> Parser::parse_write_cmd(Token token) {
  consume(Token::Type::Image);
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  consume(Token::Type::To);
  std::string string = consume(Token::Type::String).value;
  return std::make_unique<WriteCmd>(std::move(expr), std::move(string));
}

std::unique_ptr<LetCmd> Parser::parse_let_cmd(Token token) {
  std::unique_ptr<LValue> lvalue = parse_lvalue(lexer.next());
  consume(Token::Type::Equals);
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  return std::make_unique<LetCmd>(std::move(lvalue), std::move(expr));
}

std::unique_ptr<AssertCmd> Parser::parse_assert_cmd(Token token) {
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  consume(Token::Type::Comma);
  std::string string = consume(Token::Type::String).value;
  return std::make_unique<AssertCmd>(std::move(expr), std::move(string));
}

std::unique_ptr<PrintCmd> Parser::parse_print_cmd(Token token) {
  std::string string = consume(Token::Type::String).value;
  return std::make_unique<PrintCmd>(std::move(string));
}

std::unique_ptr<ShowCmd> Parser::parse_show_cmd(Token token) {
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  return std::make_unique<ShowCmd>(std::move(expr));
}

std::unique_ptr<TimeCmd> Parser::parse_time_cmd(Token token) {
  std::unique_ptr<Cmd> cmd = parse_cmd(lexer.next());
  return std::make_unique<TimeCmd>(std::move(cmd));
}

std::unique_ptr<IntExpr> Parser::parse_int_expr(Token token) {
  try {
    return std::make_unique<IntExpr>(std::stoll(token.value));
  } catch (std::out_of_range) {
    logger.log_error("Integer literal out of range: " + token.value,
                     token.start);
  }
}

std::unique_ptr<FloatExpr> Parser::parse_float_expr(Token token) {
  try {
    return std::make_unique<FloatExpr>(std::stod(token.value));
  } catch (std::out_of_range) {
    logger.log_error("Float literal out of range: " + token.value, token.start);
  }
}

std::unique_ptr<TrueExpr> Parser::parse_true_expr(Token token) {
  return std::make_unique<TrueExpr>();
}

std::unique_ptr<FalseExpr> Parser::parse_false_expr(Token token) {
  return std::make_unique<FalseExpr>();
}

std::unique_ptr<VarExpr> Parser::parse_var_expr(Token token) {
  return std::make_unique<VarExpr>(std::move(token.value));
}

std::unique_ptr<ArrayLiteralExpr>
Parser::parse_array_literal_expr(Token token) {
  std::vector<std::unique_ptr<Expr>> elements;
  while (true) {
    Token next = lexer.next();
    if (next.type == Token::Type::RSquare) {
      return std::make_unique<ArrayLiteralExpr>(std::move(elements));
    }
    elements.push_back(parse_expr(next));
    next = lexer.next();
    if (next.type == Token::Type::RSquare) {
      return std::make_unique<ArrayLiteralExpr>(std::move(elements));
    } else if (next.type != Token::Type::Comma) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
  }
}

std::unique_ptr<VarLValue> Parser::parse_var_lvalue(Token token) {
  return std::make_unique<VarLValue>(std::move(token.value));
}
