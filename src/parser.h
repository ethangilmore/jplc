#pragma once

#include "astnodes.h"
#include "lexer.h"
#include <memory>

class Parser {
public:
  Parser(Lexer &lexer);
  Token consume(Token::Type type);
  Token consume(Token::Type type, const std::string &value);
  std::unique_ptr<Program> parse();
  std::unique_ptr<Cmd> parse_cmd(Token token);
  std::unique_ptr<Expr> parse_expr(Token token);
  std::unique_ptr<LValue> parse_lvalue(Token token);
  std::unique_ptr<ReadCmd> parse_read_cmd(Token token);
  std::unique_ptr<WriteCmd> parse_write_cmd(Token token);
  std::unique_ptr<LetCmd> parse_let_cmd(Token token);
  std::unique_ptr<AssertCmd> parse_assert_cmd(Token token);
  std::unique_ptr<PrintCmd> parse_print_cmd(Token token);
  std::unique_ptr<ShowCmd> parse_show_cmd(Token token);
  std::unique_ptr<TimeCmd> parse_time_cmd(Token token);
  std::unique_ptr<IntExpr> parse_int_expr(Token token);
  std::unique_ptr<FloatExpr> parse_float_expr(Token token);
  std::unique_ptr<TrueExpr> parse_true_expr(Token token);
  std::unique_ptr<FalseExpr> parse_false_expr(Token token);
  std::unique_ptr<VarExpr> parse_var_expr(Token token);
  std::unique_ptr<ArrayLiteralExpr> parse_array_literal_expr(Token token);
  std::unique_ptr<VarLValue> parse_var_lvalue(Token token);

private:
  Lexer &lexer;
};
