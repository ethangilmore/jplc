#include "astvisitor.h"

#include "astnodes.h"

void ASTVisitor::visit(const Program &node) {
  for (const auto &cmd : node.cmds) {
    cmd->accept(*this);
  }
}

void ASTVisitor::visit(const IntType &node) {}

void ASTVisitor::visit(const BoolType &node) {}

void ASTVisitor::visit(const FloatType &node) {}

void ASTVisitor::visit(const ArrayType &node) {
  node.element_type->accept(*this);
}

void ASTVisitor::visit(const StructType &node) {}

void ASTVisitor::visit(const VoidType &node) {}

void ASTVisitor::visit(const ReadCmd &node) { node.lvalue->accept(*this); }

void ASTVisitor::visit(const WriteCmd &node) { node.expr->accept(*this); }

void ASTVisitor::visit(const LetCmd &node) {
  node.lvalue->accept(*this);
  node.expr->accept(*this);
}

void ASTVisitor::visit(const AssertCmd &node) { node.expr->accept(*this); }

void ASTVisitor::visit(const PrintCmd &node) {}

void ASTVisitor::visit(const ShowCmd &node) { node.expr->accept(*this); }

void ASTVisitor::visit(const TimeCmd &node) { node.cmd->accept(*this); }

void ASTVisitor::visit(const FnCmd &node) {
  for (const auto &binding : node.params) {
    binding->accept(*this);
  }
  node.return_type->accept(*this);
  for (const auto &stmt : node.stmts) {
    stmt->accept(*this);
  }
}

void ASTVisitor::visit(const StructCmd &node) {
  for (const auto &[string, type] : node.fields) {
    type->accept(*this);
  }
}

void ASTVisitor::visit(const LetStmt &node) {
  node.lvalue->accept(*this);
  node.expr->accept(*this);
}

void ASTVisitor::visit(const AssertStmt &node) { node.expr->accept(*this); }

void ASTVisitor::visit(const ReturnStmt &node) { node.expr->accept(*this); }

void ASTVisitor::visit(const IntExpr &node) {}

void ASTVisitor::visit(const FloatExpr &node) {}

void ASTVisitor::visit(const TrueExpr &node) {}

void ASTVisitor::visit(const FalseExpr &node) {}

void ASTVisitor::visit(const VarExpr &node) {}

void ASTVisitor::visit(const VoidExpr &node) {}

void ASTVisitor::visit(const ArrayLiteralExpr &node) {
  for (int i = node.elements.size() - 1; i >= 0; i--) {
    node.elements[i]->accept(*this);
  }
}

void ASTVisitor::visit(const StructLiteralExpr &node) {
  for (int i = node.fields.size() - 1; i >= 0; i--) {
    node.fields[i]->accept(*this);
  }
}

void ASTVisitor::visit(const DotExpr &node) { node.expr->accept(*this); }

void ASTVisitor::visit(const ArrayIndexExpr &node) {
  node.expr->accept(*this);
  for (const auto &index : node.indices) {
    index->accept(*this);
  }
}

void ASTVisitor::visit(const CallExpr &node) {
  for (const auto &arg : node.args) {
    arg->accept(*this);
  }
}

void ASTVisitor::visit(const SumLoopExpr &node) {
  for (const auto &[variable, expr] : node.axis) {
    expr->accept(*this);
  }
  node.expr->accept(*this);
}

void ASTVisitor::visit(const ArrayLoopExpr &node) {
  for (const auto &[variable, expr] : node.axis) {
    expr->accept(*this);
  }
  node.expr->accept(*this);
}

void ASTVisitor::visit(const IfExpr &node) {
  node.condition->accept(*this);
  node.if_expr->accept(*this);
  node.else_expr->accept(*this);
}

void ASTVisitor::visit(const BinopExpr &node) {
  node.right->accept(*this);
  node.left->accept(*this);
}

void ASTVisitor::visit(const UnopExpr &node) {
  node.expr->accept(*this);
}

void ASTVisitor::visit(const VarLValue &node) {}

void ASTVisitor::visit(const ArrayLValue &node) {}

void ASTVisitor::visit(const Binding &node) {
  node.lvalue->accept(*this);
  node.type->accept(*this);
}
