#pragma once

#include <memory>

#include "astnodes.h"
#include "astvisitor.h"
#include "context.h"
#include "functionvisitor.h"
#include "logger.h"
#include "typedefvisitor.h"

class CodeGenVisitor : public ASTVisitor {
 public:
  CodeGenVisitor(std::shared_ptr<Context> ctx, Logger& logger) : ctx(ctx), logger(logger) {
    type_def_generator = std::make_shared<TypeDefGenerator>(ctx, logger);
    function_generator = std::make_shared<FunctionGenerator>(this, ctx, logger);
  }

  virtual void visit(const Program& program) override {
    type_def_generator->visit(program);
    function_generator->visit(program);

    var_map.insert({"args", "args"});

    std::cout << "void jpl_main(struct args args) {\n";
    reset_name_ctr();
    ASTVisitor::visit(program);
    std::cout << "}";
  }

  virtual void visit(const IntExpr& expr) override {
    auto symbol = expr.symbol = gensym();
    auto c_type = expr.type->c_type();
    auto value = std::to_string(expr.value);
    println(c_type + " " + symbol + " = " + value + ";");
  }

  virtual void visit(const FloatExpr& expr) override {
    auto symbol = expr.symbol = gensym();
    auto c_type = expr.type->c_type();
    auto value = std::to_string(int(expr.value)) + ".0";
    println(c_type + " " + symbol + " = " + value + ";");
  }

  virtual void visit(const TrueExpr& expr) override {
    auto symbol = expr.symbol = gensym();
    auto c_type = expr.type->c_type();
    auto value = "true";
    println(c_type + " " + symbol + " = " + value + ";");
  }

  virtual void visit(const FalseExpr& expr) override {
    auto symbol = expr.symbol = gensym();
    auto c_type = expr.type->c_type();
    auto value = "false";
    println(c_type + " " + symbol + " = " + value + ";");
  }

  virtual void visit(const UnopExpr& expr) override {
    ASTVisitor::visit(expr);
    auto symbol = expr.symbol = gensym();
    auto c_type = expr.type->c_type();
    println(c_type + " " + symbol + " = " + expr.op + expr.expr->symbol + ";");
  }

  virtual void visit(const BinopExpr& expr) override {
    if (expr.op == "&&") {
      auto symbol = expr.symbol = gensym();
      expr.left->accept(*this);
      println("bool " + symbol + " = " + expr.left->symbol);
      auto label = genlabel();
      println("if (0 == " + expr.left->symbol + ")");
      println("goto " + label + ";");
      expr.right->accept(*this);
      println(symbol + " = " + expr.right->symbol + ";");
      println(label + ":;");
    } else if (expr.op == "||") {
      auto symbol = expr.symbol = gensym();
      expr.left->accept(*this);
      println("bool " + symbol + " = " + expr.left->symbol);
      println("if (0 != " + expr.left->symbol + ")");
      auto label = genlabel();
      println("goto " + label + ";");
      expr.right->accept(*this);
      println(symbol + " = " + expr.right->symbol + ";");
      println(label + ":;");
    } else {
      expr.left->accept(*this);
      expr.right->accept(*this);
      auto symbol = expr.symbol = gensym();
      auto c_type = expr.type->c_type();
      if (expr.op == "%" && expr.type->is<Float>()) {
        println(c_type + " " + symbol + " = fmod(" + expr.left->symbol + ", " + expr.right->symbol + ");");
      } else {
        println(c_type + " " + symbol + " = " + expr.left->symbol + " " + expr.op + " " + expr.right->symbol + ";");
      }
    }
  }

  virtual void visit(const VarExpr& expr) override {
    if (var_map.count(expr.identifier)) {
      expr.symbol = var_map.at(expr.identifier);
    } else {
      expr.symbol = "?";
    }
  }

  virtual void visit(const ArrayLiteralExpr& expr) override {
    ASTVisitor::visit(expr);
    auto symbol = expr.symbol = gensym();
    auto size = std::to_string(expr.elements.size());
    auto element_type = expr.type->as<Array>()->element_type->c_type();
    println(expr.type->c_type() + " " + symbol + ";");
    println(symbol + ".d0 = " + size + ";");
    println(symbol + ".data = jpl_alloc(sizeof(" + element_type + ") * " + size + ");");
    for (int i = 0; i < expr.elements.size(); i++) {
      println(symbol + ".data[" + std::to_string(i) + "] = " + expr.elements[i]->symbol + ";");
    }
  }

  virtual void visit(const VoidExpr& expr) override {
    auto symbol = expr.symbol = gensym();
    auto c_type = expr.type->c_type();
    println(c_type + " " + symbol + " = {};\n");
  }

  virtual void visit(const StructLiteralExpr& expr) override {
    ASTVisitor::visit(expr);
    auto name = expr.identifier;
    auto symbol = expr.symbol = gensym();
    std::string args = "{ ";
    if (!expr.fields.empty()) {
      args += expr.fields.front()->symbol;
      for (int i = 1; i < expr.fields.size(); i++) {
        args += ", " + expr.fields[i]->symbol;
      }
    }
    args += " }";
    println(name + " " + symbol + " = " + args + ";");
  }

  virtual void visit(const DotExpr& expr) override {
    ASTVisitor::visit(expr);
    auto symbol = expr.symbol = gensym();
    auto c_type = expr.type->c_type();
    println(c_type + " " + symbol + " = " + expr.expr->symbol + "." + expr.field + ";");
  }

  virtual void visit(const IfExpr& expr) override {
    expr.condition->accept(*this);
    auto symbol = expr.symbol = gensym();
    auto l1 = genlabel();
    auto l2 = genlabel();
    println(expr.type->c_type() + " " + symbol + ";");
    println("if (!" + expr.condition->symbol + ")");
    println("goto " + l1 + ";");
    expr.if_expr->accept(*this);
    println(symbol + " = " + expr.if_expr->symbol + ";");
    println("goto " + l2 + ";");
    println(l1 + ":;");
    expr.else_expr->accept(*this);
    println(symbol + " = " + expr.else_expr->symbol + ";");
    println(l2 + ":;");
  }

  virtual void visit(const ArrayIndexExpr& expr) override {
    expr.expr->accept(*this);
    for (const auto& index : expr.indices) {
      index->accept(*this);
    }
    for (int i = 0; i < expr.indices.size(); i++) {
      auto& index = expr.indices[i];
      auto label = genlabel();
      println("if (" + index->symbol + " >= 0)");
      println("goto " + label + ";");
      println("fail_assertion(\"negative array index\");");
      println(label + ":;");
      label = genlabel();
      println("if (" + index->symbol + " < " + expr.expr->symbol + ".d" + std::to_string(i) + ")");
      println("goto " + label + ";");
      println("fail_assertion(\"index too large\");");
      println(label + ":;");
    }
    auto index = gensym();
    println("int64_t " + index + " = 0;");
    for (int i = 0; i < expr.indices.size(); i++) {
      println(index + " *= " + expr.expr->symbol + ".d" + std::to_string(i) + ";");
      println(index + " += " + expr.indices[i]->symbol + ";");
    }
    auto symbol = expr.symbol = gensym();
    auto type = expr.type->c_type();
    println(type + " " + symbol + " = " + expr.expr->symbol + ".data[" + index + "];");
  }

  virtual void visit(const CallExpr& expr) override {
    ASTVisitor::visit(expr);
    auto symbol = expr.symbol = gensym();
    auto info = ctx->lookup<FnInfo>(expr.identifier);
    auto type = info->return_type->c_type();
    std::string args = "(";
    bool first = true;
    for (const auto& arg : expr.args) {
      if (!first) {
        args += ", ";
      } else {
        first = false;
      }
      args += arg->symbol;
    }
    args += ")";
    println(type + " " + symbol + " = " + expr.identifier + args + ";");
  }

  virtual void visit(const ArrayLoopExpr& expr) override {
    auto symbol = expr.symbol = gensym();
    println(expr.type->c_type() + " " + symbol + ";");
    for (const auto& [name, limit] : expr.axis) {
      limit->accept(*this);
      println("if (" + limit->symbol + " > 0)");
      auto label = genlabel();
      println("goto " + label + ";");
      println("fail_assertion(\"non-positive loop bound\");");
      println(label + ":;");
    }
    auto size = gensym();
    println("int64_t " + size + " = 1;");
    println(size + " *= 1;");
    if (expr.expr->type->is<Int>()) {
      println(size + " *= sizeof(int64_t);");
    } else {
      println(size + " *= sizeof(double);");
    }
    println(symbol + ".data = jpl_alloc(" + size + ");");

    std::vector<std::string> symbols{};
    for (int i = expr.axis.size() - 1; i >= 0; --i) {
      auto symbol = gensym();
      symbols.insert(symbols.begin(), symbol);
      println("int64_t " + symbol + " = 0;");
      var_map.insert({expr.axis[i].first, symbol});
    }
    auto loop = genlabel();
    println(loop + ":; // loop start");
    expr.expr->accept(*this);
    println(symbol + " += " + expr.expr->symbol + ";");
    for (int i = expr.axis.size() - 1; i >= 0; --i) {
      println(symbols[i] + "++;");
      println("if (" + symbols[i] + " < " + expr.axis[i].second->symbol + ")");
      println("goto " + loop + ";");
      if (i > 0) {
        println(symbols[i] + " = 0;");
      }
    }
  }

  virtual void visit(const SumLoopExpr& expr) override {
    auto symbol = expr.symbol = gensym();
    if (expr.expr->type->is<Int>()) {
      println("int64_t " + symbol + ";");
    } else {
      println("double " + symbol + ";");
    }
    for (const auto& [name, limit] : expr.axis) {
      limit->accept(*this);
      println("if (" + limit->symbol + " > 0)");
      auto label = genlabel();
      println("goto " + label + ";");
      println("fail_assertion(\"non-positive loop bound\");");
      println(label + ":;");
    }
    println(symbol + " = 0;");
    std::vector<std::string> symbols{};
    for (int i = expr.axis.size() - 1; i >= 0; --i) {
      auto symbol = gensym();
      symbols.insert(symbols.begin(), symbol);
      println("int64_t " + symbol + " = 0;");
      var_map.insert({expr.axis[i].first, symbol});
    }
    auto loop = genlabel();
    println(loop + ":; // loop start");
    expr.expr->accept(*this);
    println(symbol + " += " + expr.expr->symbol + ";");
    for (int i = expr.axis.size() - 1; i >= 0; --i) {
      println(symbols[i] + "++;");
      println("if (" + symbols[i] + " < " + expr.axis[i].second->symbol + ")");
      println("goto " + loop + ";");
      if (i > 0) {
        println(symbols[i] + " = 0;");
      }
    }
  }

  // expr here

  virtual void
  visit(const AssertCmd& expr) override {
    ASTVisitor::visit(expr);
    auto label = genlabel();
    println("if (0 != " + expr.expr->symbol + ")");
    println("goto " + label + ";");
    println("fail_assertion(" + expr.string + ");");
    println(label + ":;");
  }

  virtual void visit(const ReadCmd& cmd) override {
    std::string symbol = gensym();
    println("_a2_rgba " + symbol + " = read_image(" + cmd.string + ");");
    if (auto array_lvalue = dynamic_cast<ArrayLValue*>(cmd.lvalue.get())) {
      println("int64_t " + array_lvalue->indices[0] + " = " + symbol + ".d0;");
      println("int64_t " + array_lvalue->indices[1] + " = " + symbol + ".d1;");
    }
    last_symbol = symbol;
    cmd.lvalue->accept(*this);
  }

  virtual void visit(const AssertStmt& expr) override {
    ASTVisitor::visit(expr);
    auto label = genlabel();
    println("if (0 != " + expr.expr->symbol + ")");
    println("goto " + label + ";");
    println("fail_assertion(" + expr.string + ");");
    println(label + ":;");
  }

  virtual void visit(const ShowCmd& cmd) override {
    ASTVisitor::visit(cmd);
    auto type = cmd.expr->type->show_type(ctx.get());
    auto symbol = cmd.expr->symbol;
    println("show(\"" + type + "\", &" + symbol + ");");
  }

  virtual void visit(const LetCmd& cmd) override {
    cmd.expr->accept(*this);
    last_symbol = cmd.expr->symbol;
    cmd.lvalue->accept(*this);
  }

  virtual void visit(const VarLValue& lvalue) override {
    var_map.insert({lvalue.identifier, last_symbol});
  }

  virtual void visit(const ArrayLValue& lvalue) override {
    var_map.insert({lvalue.identifier, last_symbol});
    for (int i = 0; i < lvalue.indices.size(); i++) {
      auto index = lvalue.indices[i];
      var_map.insert({index, last_symbol + ".d" + std::to_string(i)});
    }
  }

  virtual void visit(const LetStmt& cmd) override {
    ASTVisitor::visit(cmd);
    var_map.insert({cmd.lvalue->identifier, cmd.expr->symbol});
  }

  virtual void visit(const PrintCmd& cmd) override {
    ASTVisitor::visit(cmd);
    println("print(" + cmd.string + ");");
  }

  virtual void visit(const FnCmd& fn) override {}

  void reset_name_ctr() {
    name_ctr = 0;
  }

 private:
  int name_ctr = 0;
  int jump_ctr = 1;
  std::shared_ptr<TypeDefGenerator> type_def_generator;
  std::shared_ptr<FunctionGenerator> function_generator;
  std::shared_ptr<Context> ctx;
  std::unordered_map<std::string, std::string> var_map;
  std::string last_symbol = "";
  Logger& logger;

  std::string gensym() {
    return "_" + std::to_string(name_ctr++);
  }

  std::string genlabel() {
    return "_jump" + std::to_string(jump_ctr++);
  }

  void println(std::string str) {
    std::cout << "  " << str << '\n';
  }
};
