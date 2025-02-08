#pragma once

// #include "astnodes.h"

class ASTNode;
class Program;

class Type;
class IntType;
class BoolType;
class FloatType;
class ArrayType;
class StructType;
class VoidType;

class Cmd;
class ReadCmd;
class WriteCmd;
class LetCmd;
class AssertCmd;
class PrintCmd;
class ShowCmd;
class TimeCmd;
class FnCmd;
class StructCmd;

class Stmt;
class LetStmt;
class AssertStmt;
class ReturnStmt;

class Expr;
class IntExpr;
class FloatExpr;
class TrueExpr;
class FalseExpr;
class VarExpr;
class VoidExpr;
class ArrayLiteralExpr;
class StructLiteralExpr;
class DotExpr;
class ArrayIndexExpr;
class CallExpr;

class LValue;
class VarLValue;
class ArrayLValue;

class Binding;

class ASTVisitor {
 public:
  // Program
  virtual void visit(const Program &node);

  // Types
  virtual void visit(const IntType &node);
  virtual void visit(const BoolType &node);
  virtual void visit(const FloatType &node);
  virtual void visit(const ArrayType &node);
  virtual void visit(const StructType &node);
  virtual void visit(const VoidType &node);

  // Commands
  virtual void visit(const ReadCmd &node);
  virtual void visit(const WriteCmd &node);
  virtual void visit(const LetCmd &node);
  virtual void visit(const AssertCmd &node);
  virtual void visit(const PrintCmd &node);
  virtual void visit(const ShowCmd &node);
  virtual void visit(const TimeCmd &node);
  virtual void visit(const FnCmd &node);
  virtual void visit(const StructCmd &node);

  // Statements
  virtual void visit(const LetStmt &node);
  virtual void visit(const AssertStmt &node);
  virtual void visit(const ReturnStmt &node);

  // Expressions
  virtual void visit(const IntExpr &node);
  virtual void visit(const FloatExpr &node);
  virtual void visit(const TrueExpr &node);
  virtual void visit(const FalseExpr &node);
  virtual void visit(const VarExpr &node);
  virtual void visit(const VoidExpr &node);
  virtual void visit(const ArrayLiteralExpr &node);
  virtual void visit(const StructLiteralExpr &node);
  virtual void visit(const DotExpr &node);
  virtual void visit(const ArrayIndexExpr &node);
  virtual void visit(const CallExpr &node);

  // LValues
  virtual void visit(const VarLValue &node);
  virtual void visit(const ArrayLValue &node);

  // Bindings
  virtual void visit(const Binding &node);
};
