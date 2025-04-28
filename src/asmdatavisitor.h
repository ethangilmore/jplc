#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <variant>

#include "astnodes.h"
#include "astvisitor.h"
#include "context.h"

typedef std::variant<int64_t, double, std::string> asmval;

class ASMDataVisitor : public ASTVisitor {
 public:
  std::map<asmval, std::string> const_map;

  ASMDataVisitor(std::shared_ptr<Context> ctx, int opt) : ctx(ctx), opt(opt){};

  virtual void visit(const Program& program) override {
    std::cout << std::fixed << std::setprecision(15);
    std::cout << "section .data\n";
    ASTVisitor::visit(program);
    std::cout << "\nsection .text\n";
  }

  virtual void visit(const IntExpr& expr) override {
    if (opt > 0 && expr.value >= INT32_MIN && expr.value <= INT32_MAX) return;
    add_int(expr.value);
  }

  virtual void visit(const FloatExpr& expr) override {
    add_float(expr.value);
  }

  virtual void visit(const TrueExpr& expr) override {
    if (opt > 0) return;
    add_int(1);
  }

  virtual void visit(const FalseExpr& expr) override {
    if (opt > 0) return;
    add_int(0);
  }

  virtual void visit(const BinopExpr& expr) override {
    if (expr.op == "||" || expr.op == "&&") {
      expr.left->accept(*this);
      expr.right->accept(*this);
    } else {
      ASTVisitor::visit(expr);
    }
    if (expr.op == "/" && expr.type->is<Int>()) {
      add_string("divide by zero");
    } else if (expr.op == "%" && expr.type->is<Int>()) {
      add_string("mod by zero");
    }
  }

  virtual void visit(const ArrayIndexExpr& expr) override {
    expr.expr->accept(*this);
    for (int i = expr.indices.size() - 1; i >= 0; i--) {
      expr.indices[i]->accept(*this);
    }
    add_string("negative array index");
    add_string("index too large");
  }

  virtual void visit(const SumLoopExpr& expr) override {
    for (int i = expr.axis.size() - 1; i >= 0; i--) {
      expr.axis[i].second->accept(*this);
      add_string("non-positive loop bound");
    }
    expr.expr->accept(*this);
  }

  virtual void visit(const ArrayLoopExpr& expr) override {
    for (int i = expr.axis.size() - 1; i >= 0; i--) {
      expr.axis[i].second->accept(*this);
      add_string("non-positive loop bound");
    }
    add_string("overflow computing array size");
    expr.expr->accept(*this);
  }

  virtual void visit(const ShowCmd& cmd) override {
    ASTVisitor::visit(cmd);
    add_string(cmd.expr->type->show_type(ctx.get()));
  }

  virtual void visit(const ReadCmd& cmd) override {
    add_string(cmd.stripped_string());
    ASTVisitor::visit(cmd);
  }

  virtual void visit(const WriteCmd& cmd) override {
    add_string(cmd.stripped_string());
    ASTVisitor::visit(cmd);
  }

  void add_int(int64_t val) {
    if (const_map.count(val)) return;
    auto name = "const" + std::to_string(ctr++);
    std::cout << name + ": dq " + std::to_string(val) + "\n";
    const_map[val] = name;
  }

  void add_float(double val) {
    if (const_map.count(val)) return;
    auto name = "const" + std::to_string(ctr++);
    std::cout << name + ": dq " << val << "\n";
    const_map[val] = name;
  }

  void add_string(std::string str) {
    if (const_map.count(str)) return;
    auto name = "const" + std::to_string(ctr++);
    std::cout << name + ": db `" + str + "`, 0\n";
    const_map[str] = name;
  }

 private:
  int ctr = 0;
  int opt = 0;
  std::shared_ptr<Context> ctx;
};
