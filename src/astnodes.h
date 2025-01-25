#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

class ASTVisitor;
class ASTNode;
class Program;
class Cmd;
class ReadCmd;
class WriteCmd;
class LetCmd;
class AssertCmd;
class PrintCmd;
class ShowCmd;
class TimeCmd;
class Expr;
class IntExpr;
class FloatExpr;
class TrueExpr;
class FalseExpr;
class VarExpr;
class ArrayLiteralExpr;
class VarLValue;

class ASTNode {
public:
  virtual void accept(ASTVisitor &visitor) = 0;
  virtual ~ASTNode() = default;
};

class Cmd : public ASTNode {};

class Expr : public ASTNode {};

class LValue : public ASTNode {};

class Program : public ASTNode {
public:
  std::vector<std::unique_ptr<Cmd>> cmds;
  Program(std::vector<std::unique_ptr<Cmd>> cmds);
  void accept(ASTVisitor &visitor) override;
};

class ReadCmd : public Cmd {
public:
  std::string string;
  std::unique_ptr<VarLValue> lvalue;
  ReadCmd(std::string string, std::unique_ptr<VarLValue> lvalue);
  void accept(ASTVisitor &visitor) override;
};

class WriteCmd : public Cmd {
public:
  std::unique_ptr<Expr> expr;
  std::string string;
  WriteCmd(std::unique_ptr<Expr> expr, std::string string);
  void accept(ASTVisitor &visitor) override;
};

class LetCmd : public Cmd {
public:
  std::unique_ptr<LValue> lvalue;
  std::unique_ptr<Expr> expr;
  LetCmd(std::unique_ptr<LValue> lvalue, std::unique_ptr<Expr> expr);
  void accept(ASTVisitor &visitor) override;
};

class AssertCmd : public Cmd {
public:
  std::unique_ptr<Expr> expr;
  std::string string;
  AssertCmd(std::unique_ptr<Expr> expr, std::string string);
  void accept(ASTVisitor &visitor) override;
};

class PrintCmd : public Cmd {
public:
  std::string string;
  PrintCmd(std::string string);
  void accept(ASTVisitor &visitor) override;
};

class ShowCmd : public Cmd {
public:
  std::unique_ptr<Expr> expr;
  ShowCmd(std::unique_ptr<Expr> expr);
  void accept(ASTVisitor &visitor) override;
};

class TimeCmd : public Cmd {
public:
  std::unique_ptr<Cmd> cmd;
  TimeCmd(std::unique_ptr<Cmd> cmd);
  void accept(ASTVisitor &visitor) override;
};

class IntExpr : public Expr {
public:
  int64_t value;
  IntExpr(int64_t value);
  void accept(ASTVisitor &visitor) override;
};

class FloatExpr : public Expr {
public:
  double value;
  FloatExpr(double value);
  void accept(ASTVisitor &visitor) override;
};

class TrueExpr : public Expr {
public:
  TrueExpr();
  void accept(ASTVisitor &visitor) override;
};

class FalseExpr : public Expr {
public:
  FalseExpr();
  void accept(ASTVisitor &visitor) override;
};

class VarExpr : public Expr {
public:
  std::string variable;
  VarExpr(std::string variable);
  void accept(ASTVisitor &visitor) override;
};

class ArrayLiteralExpr : public Expr {
public:
  std::vector<std::unique_ptr<Expr>> elements;
  ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>> elements);
  void accept(ASTVisitor &visitor) override;
};

class VarLValue : public LValue {
public:
  std::string variable;
  VarLValue(std::string variable);
  void accept(ASTVisitor &visitor) override;
};
