#pragma once

#include "astnodes.h"

class ASTVisitor {
public:
  virtual void visit(const Program &node) = 0;
  virtual void visit(const ReadCmd &node) = 0;
  virtual void visit(const WriteCmd &node) = 0;
  virtual void visit(const LetCmd &node) = 0;
  virtual void visit(const AssertCmd &node) = 0;
  virtual void visit(const PrintCmd &node) = 0;
  virtual void visit(const ShowCmd &node) = 0;
  virtual void visit(const TimeCmd &node) = 0;
  virtual void visit(const IntExpr &node) = 0;
  virtual void visit(const FloatExpr &node) = 0;
  virtual void visit(const TrueExpr &node) = 0;
  virtual void visit(const FalseExpr &node) = 0;
  virtual void visit(const VarExpr &node) = 0;
  virtual void visit(const ArrayLiteralExpr &node) = 0;
  virtual void visit(const VarLValue &node) = 0;
};
