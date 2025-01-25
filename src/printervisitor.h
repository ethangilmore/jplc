#pragma once

#include "astvisitor.h"

class PrinterVisitor : public ASTVisitor {
public:
  void visit(const Program &node) override;
  void visit(const ReadCmd &node) override;
  void visit(const WriteCmd &node) override;
  void visit(const LetCmd &node) override;
  void visit(const AssertCmd &node) override;
  void visit(const PrintCmd &node) override;
  void visit(const ShowCmd &node) override;
  void visit(const TimeCmd &node) override;
  void visit(const IntExpr &node) override;
  void visit(const FloatExpr &node) override;
  void visit(const TrueExpr &node) override;
  void visit(const FalseExpr &node) override;
  void visit(const VarExpr &node) override;
  void visit(const ArrayLiteralExpr &node) override;
  void visit(const VarLValue &node) override;
};
