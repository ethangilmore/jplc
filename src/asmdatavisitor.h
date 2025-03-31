#pragma once

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

  ASMDataVisitor(std::shared_ptr<Context> ctx) : ctx(ctx){};

  virtual void visit(const Program& program) override {
    std::cout << std::fixed << std::setprecision(15);
    std::cout << "section .data\n";
    ASTVisitor::visit(program);
  }

  virtual void visit(const IntExpr& expr) override {
    add_int(expr.value);
  }

  virtual void visit(const FloatExpr& expr) override {
    add_float(expr.value);
  }

  virtual void visit(const TrueExpr& expr) override {
    add_int(1);
  }

  virtual void visit(const FalseExpr& expr) override {
    add_int(0);
  }

  virtual void visit(const BinopExpr& expr) override {
    ASTVisitor::visit(expr);
    if (expr.op == "/" && expr.type->is<Int>()) {
      add_string("divide by zero");
    } else if (expr.op == "%" && expr.type->is<Int>()) {
      add_string("mod by zero");
    }
  }

  virtual void visit(const ShowCmd& cmd) override {
    ASTVisitor::visit(cmd);
    add_string(cmd.expr->type->show_type(ctx.get()));
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
  int ctr;
  std::shared_ptr<Context> ctx;
};
