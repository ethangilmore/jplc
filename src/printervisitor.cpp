#include "printervisitor.h"
#include <iostream>

void PrinterVisitor::visit(const Program &node) {
  for (const auto &cmd : node.cmds) {
    cmd->accept(*this);
    std::cout << "\n";
  }
}

void PrinterVisitor::visit(const ReadCmd &node) {
  std::cout << "(ReadCmd " << node.string << " ";
  node.lvalue->accept(*this);
  std::cout << ")";
}

void PrinterVisitor::visit(const WriteCmd &node) {
  std::cout << "(WriteCmd ";
  node.expr->accept(*this);
  std::cout << " " << node.string << ")";
}

void PrinterVisitor::visit(const LetCmd &node) {
  std::cout << "(LetCmd ";
  node.lvalue->accept(*this);
  std::cout << " ";
  node.expr->accept(*this);
  std::cout << ")";
}

void PrinterVisitor::visit(const AssertCmd &node) {
  std::cout << "(AssertCmd ";
  node.expr->accept(*this);
  std::cout << " " << node.string << ")";
}

void PrinterVisitor::visit(const PrintCmd &node) {
  std::cout << "(PrintCmd " << node.string << ")";
}

void PrinterVisitor::visit(const ShowCmd &node) {
  std::cout << "(ShowCmd ";
  node.expr->accept(*this);
  std::cout << ")";
}

void PrinterVisitor::visit(const TimeCmd &node) {
  std::cout << "(TimeCmd ";
  node.cmd->accept(*this);
  std::cout << ")";
}

void PrinterVisitor::visit(const IntExpr &node) {
  std::cout << "(IntExpr " << node.value << ")";
}

void PrinterVisitor::visit(const FloatExpr &node) {
  std::cout << "(FloatExpr " << static_cast<long>(node.value) << ")";
}

void PrinterVisitor::visit(const TrueExpr &node) { std::cout << "(TrueExpr)"; }

void PrinterVisitor::visit(const FalseExpr &node) {
  std::cout << "(FalseExpr)";
}

void PrinterVisitor::visit(const VarExpr &node) {
  std::cout << "(VarExpr " << node.variable << ")";
}

void PrinterVisitor::visit(const ArrayLiteralExpr &node) {
  std::cout << "(ArrayLiteralExpr";
  for (const auto &expr : node.elements) {
    std::cout << " ";
    expr->accept(*this);
  }
  std::cout << ")";
}

void PrinterVisitor::visit(const VarLValue &node) {
  std::cout << "(VarLValue " << node.variable << ")";
}
