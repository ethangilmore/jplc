#include "astvisitor.h"
#include <iostream>

class PrinterVisitor : public ASTVisitor {
public:
  /* ========== Program ========== */
  void visit(const Program &node) override {
    for (const auto &cmd : node.cmds) {
      cmd->accept(*this);
      std::cout << "\n";
    }
  }

  /* ========== Types ========== */
  void visit(const IntType &node) override { std::cout << "(IntType)"; }

  void visit(const BoolType &node) override { std::cout << "(BoolType)"; }

  void visit(const FloatType &node) override { std::cout << "(FloatType)"; }

  void visit(const ArrayType &node) override {
    std::cout << "(ArrayType ";
    node.type->accept(*this);
    std::cout << " " << node.rank << ")";
  }

  void visit(const StructType &node) override {
    std::cout << "(StructType " << node.identifier << ")";
  }

  void visit(const VoidType &node) override { std::cout << "(VoidType)"; }

  /* ========== Commands ========== */
  void visit(const ReadCmd &node) override {
    std::cout << "(ReadCmd " << node.string << " ";
    node.lvalue->accept(*this);
    std::cout << ")";
  }

  void visit(const WriteCmd &node) override {
    std::cout << "(WriteCmd ";
    node.expr->accept(*this);
    std::cout << " " << node.string << ")";
  }

  void visit(const LetCmd &node) override {
    std::cout << "(LetCmd ";
    node.lvalue->accept(*this);
    std::cout << " ";
    node.expr->accept(*this);
    std::cout << ")";
  }

  void visit(const AssertCmd &node) override {
    std::cout << "(AssertCmd ";
    node.expr->accept(*this);
    std::cout << " " << node.string << ")";
  }

  void visit(const PrintCmd &node) override {
    std::cout << "(PrintCmd " << node.string << ")";
  }

  void visit(const ShowCmd &node) override {
    std::cout << "(ShowCmd ";
    node.expr->accept(*this);
    std::cout << ")";
  }

  void visit(const TimeCmd &node) override {
    std::cout << "(TimeCmd ";
    node.cmd->accept(*this);
    std::cout << ")";
  }

  void visit(const FnCmd &node) override {
    std::cout << "(FnCmd " << node.identifier << " ((";
    for (const auto &param : node.params) {
      param->accept(*this);
      if (&param != &node.params.back()) {
        std::cout << " ";
      }
    }
    std::cout << ")) ";
    node.return_type->accept(*this);
    std::cout << " ";
    for (const auto &stmt : node.stmts) {
      stmt->accept(*this);
      if (&stmt != &node.stmts.back()) {
        std::cout << " ";
      }
    }
    std::cout << ")";
  }

  void visit(const StructCmd &node) override {
    std::cout << "(StructCmd " << node.identifier;
    for (const auto &field : node.fields) {
      std::cout << " " << field.first << " ";
      field.second->accept(*this);
    }
    std::cout << ")";
  }

  /* ========== Statements ========== */
  void visit(const LetStmt &node) override {
    std::cout << "(LetStmt ";
    node.lvalue->accept(*this);
    std::cout << " ";
    node.expr->accept(*this);
    std::cout << ")";
  }

  void visit(const AssertStmt &node) override {
    std::cout << "(AssertStmt ";
    node.expr->accept(*this);
    std::cout << " " << node.string << ")";
  }

  void visit(const ReturnStmt &node) override {
    std::cout << "(ReturnStmt ";
    node.expr->accept(*this);
    std::cout << ")";
  }

  /* ========== Expressions ========== */
  void visit(const IntExpr &node) override {
    std::cout << "(IntExpr " << node.value << ")";
  }

  void visit(const FloatExpr &node) override {
    std::cout << "(FloatExpr " << static_cast<long>(node.value) << ")";
  }

  void visit(const TrueExpr &node) override { std::cout << "(TrueExpr)"; }

  void visit(const FalseExpr &node) override { std::cout << "(FalseExpr)"; }

  void visit(const VarExpr &node) override {
    std::cout << "(VarExpr " << node.identifier << ")";
  }

  void visit(const VoidExpr &node) override { std::cout << "(VoidExpr)"; }

  void visit(const ArrayLiteralExpr &node) override {
    std::cout << "(ArrayLiteralExpr";
    for (const auto &expr : node.elements) {
      std::cout << " ";
      expr->accept(*this);
    }
    std::cout << ")";
  }

  void visit(const StructLiteralExpr &node) override {
    std::cout << "(StructLiteralExpr " << node.identifier;
    for (const auto &expr : node.fields) {
      std::cout << " ";
      expr->accept(*this);
    }
    std::cout << ")";
  }

  void visit(const DotExpr &node) override {
    std::cout << "(DotExpr ";
    node.expr->accept(*this);
    std::cout << " " << node.field << ")";
  }

  void visit(const ArrayIndexExpr &node) override {
    std::cout << "(ArrayIndexExpr ";
    node.expr->accept(*this);
    for (const auto &index : node.indices) {
      std::cout << " ";
      index->accept(*this);
    }
    std::cout << ")";
  }

  void visit(const CallExpr &node) override {
    std::cout << "(CallExpr " << node.identifier;
    for (const auto &arg : node.args) {
      std::cout << " ";
      arg->accept(*this);
    }
    std::cout << ")";
  }

  /* ========== LValues ========== */
  void visit(const VarLValue &node) override {
    std::cout << "(VarLValue " << node.identifier << ")";
  }

  void visit(const ArrayLValue &node) override {
    std::cout << "(ArrayLValue ";
    std::cout << node.identifier;
    for (const auto &index : node.indices) {
      std::cout << " " << index;
    }
    std::cout << ")";
  }

  /* ========== Bindings ========== */
  void visit(const Binding &node) override {
    node.lvalue->accept(*this);
    std::cout << " ";
    node.type->accept(*this);
  }
};
