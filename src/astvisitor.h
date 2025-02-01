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
  virtual void visit(const Program &node) = 0;

  // Types
  virtual void visit(const IntType &node) = 0;
  virtual void visit(const BoolType &node) = 0;
  virtual void visit(const FloatType &node) = 0;
  virtual void visit(const ArrayType &node) = 0;
  virtual void visit(const StructType &node) = 0;
  virtual void visit(const VoidType &node) = 0;

  // Commands
  virtual void visit(const ReadCmd &node) = 0;
  virtual void visit(const WriteCmd &node) = 0;
  virtual void visit(const LetCmd &node) = 0;
  virtual void visit(const AssertCmd &node) = 0;
  virtual void visit(const PrintCmd &node) = 0;
  virtual void visit(const ShowCmd &node) = 0;
  virtual void visit(const TimeCmd &node) = 0;
  virtual void visit(const FnCmd &node) = 0;
  virtual void visit(const StructCmd &node) = 0;

  // Statements
  virtual void visit(const LetStmt &node) = 0;
  virtual void visit(const AssertStmt &node) = 0;
  virtual void visit(const ReturnStmt &node) = 0;

  // Expressions
  virtual void visit(const IntExpr &node) = 0;
  virtual void visit(const FloatExpr &node) = 0;
  virtual void visit(const TrueExpr &node) = 0;
  virtual void visit(const FalseExpr &node) = 0;
  virtual void visit(const VarExpr &node) = 0;
  virtual void visit(const VoidExpr &node) = 0;
  virtual void visit(const ArrayLiteralExpr &node) = 0;
  virtual void visit(const StructLiteralExpr &node) = 0;
  virtual void visit(const DotExpr &node) = 0;
  virtual void visit(const ArrayIndexExpr &node) = 0;
  virtual void visit(const CallExpr &node) = 0;

  // LValues
  virtual void visit(const VarLValue &node) = 0;
  virtual void visit(const ArrayLValue &node) = 0;

  // Bindings
  virtual void visit(const Binding &node) = 0;
};
