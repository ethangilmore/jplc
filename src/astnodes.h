#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "astvisitor.h"

class ASTNode {
 public:
  virtual void accept(ASTVisitor& visitor) = 0;
  virtual ~ASTNode() = default;
};

class Type : public ASTNode {};

class Cmd : public ASTNode {};

class Stmt : public ASTNode {};

class Expr : public ASTNode {};

class LValue : public ASTNode {};

class Program : public ASTNode {
 public:
  std::vector<std::unique_ptr<Cmd>> cmds;
  Program(std::vector<std::unique_ptr<Cmd>> cmds) : cmds(std::move(cmds)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

/* ========== Types ========== */
class IntType : public Type {
 public:
  IntType() {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class BoolType : public Type {
 public:
  BoolType() {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class FloatType : public Type {
 public:
  FloatType() {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayType : public Type {
 public:
  std::unique_ptr<Type> type;
  size_t rank;
  ArrayType(std::unique_ptr<Type> type, size_t rank)
      : type(std::move(type)), rank(rank) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class StructType : public Type {
 public:
  std::string identifier;
  StructType(std::string identifier) : identifier(std::move(identifier)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class VoidType : public Type {
 public:
  VoidType() {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

/* ========== Commands ========== */
class ReadCmd : public Cmd {
 public:
  std::string string;
  std::unique_ptr<VarLValue> lvalue;
  ReadCmd(std::string string, std::unique_ptr<VarLValue> lvalue)
      : string(std::move(string)), lvalue(std::move(lvalue)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class WriteCmd : public Cmd {
 public:
  std::unique_ptr<Expr> expr;
  std::string string;
  WriteCmd(std::unique_ptr<Expr> expr, std::string string)
      : expr(std::move(expr)), string(std::move(string)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class LetCmd : public Cmd {
 public:
  std::unique_ptr<LValue> lvalue;
  std::unique_ptr<Expr> expr;
  LetCmd(std::unique_ptr<LValue> lvalue, std::unique_ptr<Expr> expr)
      : lvalue(std::move(lvalue)), expr(std::move(expr)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class AssertCmd : public Cmd {
 public:
  std::unique_ptr<Expr> expr;
  std::string string;
  AssertCmd(std::unique_ptr<Expr> expr, std::string string)
      : expr(std::move(expr)), string(std::move(string)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class PrintCmd : public Cmd {
 public:
  std::string string;
  PrintCmd(std::string string) : string(std::move(string)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ShowCmd : public Cmd {
 public:
  std::unique_ptr<Expr> expr;
  ShowCmd(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class TimeCmd : public Cmd {
 public:
  std::unique_ptr<Cmd> cmd;
  TimeCmd(std::unique_ptr<Cmd> cmd) : cmd(std::move(cmd)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class FnCmd : public Cmd {
 public:
  std::string identifier;
  std::vector<std::unique_ptr<Binding>> params;
  std::unique_ptr<Type> return_type;
  std::vector<std::unique_ptr<Stmt>> stmts;
  FnCmd(std::string identifier,
        std::vector<std::unique_ptr<Binding>> params,
        std::unique_ptr<Type> return_type,
        std::vector<std::unique_ptr<Stmt>> stmts)
      : identifier(std::move(identifier)),
        params(std::move(params)),
        return_type(std::move(return_type)),
        stmts(std::move(stmts)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class StructCmd : public Cmd {
 public:
  std::string identifier;
  std::vector<std::pair<std::string, std::unique_ptr<Type>>> fields;
  StructCmd(std::string identifier,
            std::vector<std::pair<std::string, std::unique_ptr<Type>>> fields)
      : identifier(std::move(identifier)), fields(std::move(fields)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

/* ========== Statements ========== */
class LetStmt : public Stmt {
 public:
  std::unique_ptr<LValue> lvalue;
  std::unique_ptr<Expr> expr;
  LetStmt(std::unique_ptr<LValue> lvalue, std::unique_ptr<Expr> expr)
      : lvalue(std::move(lvalue)), expr(std::move(expr)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class AssertStmt : public Stmt {
 public:
  std::unique_ptr<Expr> expr;
  std::string string;
  AssertStmt(std::unique_ptr<Expr> expr, std::string string)
      : expr(std::move(expr)), string(std::move(string)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ReturnStmt : public Stmt {
 public:
  std::unique_ptr<Expr> expr;
  ReturnStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

/* ========== Expressions ========== */
class IntExpr : public Expr {
 public:
  int64_t value;
  IntExpr(int64_t value) : value(value) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class FloatExpr : public Expr {
 public:
  double value;
  FloatExpr(double value) : value(value) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class TrueExpr : public Expr {
 public:
  TrueExpr() {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class FalseExpr : public Expr {
 public:
  FalseExpr() {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class VarExpr : public Expr {
 public:
  std::string identifier;
  VarExpr(std::string identifier) : identifier(std::move(identifier)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class VoidExpr : public Expr {
 public:
  VoidExpr() {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayLiteralExpr : public Expr {
 public:
  std::vector<std::unique_ptr<Expr>> elements;
  ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>> elements)
      : elements(std::move(elements)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class StructLiteralExpr : public Expr {
 public:
  std::string identifier;
  std::vector<std::unique_ptr<Expr>> fields;
  StructLiteralExpr(std::string identifier,
                    std::vector<std::unique_ptr<Expr>> fields)
      : identifier(std::move(identifier)), fields(std::move(fields)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class DotExpr : public Expr {
 public:
  std::unique_ptr<Expr> expr;
  std::string field;
  DotExpr(std::unique_ptr<Expr> expr, std::string field)
      : expr(std::move(expr)), field(std::move(field)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayIndexExpr : public Expr {
 public:
  std::unique_ptr<Expr> expr;
  std::vector<std::unique_ptr<Expr>> indices;
  ArrayIndexExpr(std::unique_ptr<Expr> expr,
                 std::vector<std::unique_ptr<Expr>> indices)
      : expr(std::move(expr)), indices(std::move(indices)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class CallExpr : public Expr {
 public:
  std::string identifier;
  std::vector<std::unique_ptr<Expr>> args;
  CallExpr(std::string identifier, std::vector<std::unique_ptr<Expr>> args)
      : identifier(std::move(identifier)), args(std::move(args)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

/* ========== LValues ========== */
class VarLValue : public LValue {
 public:
  std::string identifier;
  VarLValue(std::string identifier) : identifier(std::move(identifier)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ArrayLValue : public LValue {
 public:
  std::string identifier;
  std::vector<std::string> indices;
  ArrayLValue(std::string identifier, std::vector<std::string> indices)
      : identifier(std::move(identifier)), indices(std::move(indices)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

/* ========== Bindings ========== */
class Binding : public ASTNode {
 public:
  std::unique_ptr<LValue> lvalue;
  std::unique_ptr<Type> type;
  Binding(std::unique_ptr<LValue> lvalue, std::unique_ptr<Type> type)
      : lvalue(std::move(lvalue)), type(std::move(type)) {}
  void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};
