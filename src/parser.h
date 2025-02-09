#pragma once

#include <memory>

#include "astnodes.h"
#include "lexer.h"
#include "logger.h"

class Parser {
 public:
  Parser(Lexer& lexer, Logger& logger);

  Token consume(Token::Type type);
  Token consume(Token::Type type, const std::string& value);

  // Program
  std::unique_ptr<Program> parse();

  // Type
  std::unique_ptr<Type> parse_type(Token token);
  std::unique_ptr<Type> parse_base_type(Token token);
  std::unique_ptr<Type> parse_array_type(std::unique_ptr<Type> base_type);

  // Cmd
  std::unique_ptr<Cmd> parse_cmd(Token token);
  std::unique_ptr<ReadCmd> parse_read_cmd(Token token);
  std::unique_ptr<WriteCmd> parse_write_cmd(Token token);
  std::unique_ptr<LetCmd> parse_let_cmd(Token token);
  std::unique_ptr<AssertCmd> parse_assert_cmd(Token token);
  std::unique_ptr<PrintCmd> parse_print_cmd(Token token);
  std::unique_ptr<ShowCmd> parse_show_cmd(Token token);
  std::unique_ptr<TimeCmd> parse_time_cmd(Token token);
  std::unique_ptr<FnCmd> parse_fn_cmd(Token token);
  std::unique_ptr<StructCmd> parse_struct_cmd(Token token);

  // Expr
  std::unique_ptr<Expr> parse_expr(Token token);
  std::unique_ptr<Expr> parse_base_expr(Token token);
  // std::unique_ptr<Expr> parse_cont_expr(Token token);
  std::unique_ptr<IntExpr> parse_int_expr(Token token);
  std::unique_ptr<FloatExpr> parse_float_expr(Token token);
  std::unique_ptr<TrueExpr> parse_true_expr(Token token);
  std::unique_ptr<FalseExpr> parse_false_expr(Token token);
  std::unique_ptr<Expr> parse_var_expr(Token token);
  std::unique_ptr<VoidExpr> parse_void_expr(Token token);
  std::unique_ptr<ArrayLiteralExpr> parse_array_literal_expr(Token token);
  std::unique_ptr<StructLiteralExpr> parse_struct_literal_expr(Token token);
  std::unique_ptr<Expr> parse_paren_expr(Token token);
  std::unique_ptr<DotExpr> parse_dot_expr(std::unique_ptr<Expr> base_expr);
  std::unique_ptr<ArrayIndexExpr> parse_array_index_expr(
      std::unique_ptr<Expr> base_expr);
  std::unique_ptr<CallExpr> parse_call_expr(Token token);

  std::unique_ptr<IfExpr> parse_if_expr(Token token);
  std::unique_ptr<ArrayLoopExpr> parse_array_loop_expr(Token token);
  std::unique_ptr<SumLoopExpr> parse_sum_loop_expr(Token token);
  std::unique_ptr<Expr> parse_index_expr(Token token);
  std::unique_ptr<Expr> parse_add_expr(Token token);
  std::unique_ptr<Expr> parse_mult_expr(Token token);
  std::unique_ptr<Expr> parse_compare_expr(Token token);
  std::unique_ptr<Expr> parse_boolean_expr(Token token);
  std::unique_ptr<Expr> parse_unop_expr(Token token);

  // Stmt
  std::unique_ptr<Stmt> parse_stmt(Token token);
  std::unique_ptr<LetStmt> parse_let_stmt(Token token);
  std::unique_ptr<AssertStmt> parse_assert_stmt(Token token);
  std::unique_ptr<ReturnStmt> parse_return_stmt(Token token);

  // LValue
  std::unique_ptr<LValue> parse_lvalue(Token token);
  std::unique_ptr<VarLValue> parse_var_lvalue(Token token);
  std::unique_ptr<ArrayLValue> parse_array_lvalue(Token token);

  // Binding
  std::unique_ptr<Binding> parse_binding(Token token);

 private:
  Logger& logger;
  Lexer& lexer;
};
