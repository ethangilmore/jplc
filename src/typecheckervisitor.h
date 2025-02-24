#include "astvisitor.h"
#include "logger.h"
#include "context.h"
#include "astnodes.h"
#include <functional>
#include <iostream>
#include <memory>

class TypeCheckerVisitor : public ASTVisitor {
public:
  TypeCheckerVisitor(Logger& logger) : logger(logger) {}

  virtual void visit(const Program& program) override {
    ctx = std::make_shared<Context>();
    auto f = std::make_shared<Float>();
    auto i = std::make_shared<Int>();
    ctx->add(std::make_shared<StructInfo>( "rgba", 
      std::vector<std::pair<std::string, std::shared_ptr<ResolvedType>>>{
        {"r", f}, {"g", f}, {"b", f}, {"a", f},
      }
    ));
    ctx->add(std::make_shared<ValueInfo>("args", std::make_shared<Array>(i, 1)));
    ctx->add(std::make_shared<ValueInfo>("argnum", i));
    ctx->add(std::make_shared<FnInfo>("sin", std::vector<std::shared_ptr<ResolvedType>>{ f }, f));
    ctx->add(std::make_shared<FnInfo>("sqrt", std::vector<std::shared_ptr<ResolvedType>>{ f }, f));
    ctx->add(std::make_shared<FnInfo>("exp", std::vector<std::shared_ptr<ResolvedType>>{ f }, f));
    ctx->add(std::make_shared<FnInfo>("sin", std::vector<std::shared_ptr<ResolvedType>>{ f }, f));
    ctx->add(std::make_shared<FnInfo>("cos", std::vector<std::shared_ptr<ResolvedType>>{ f }, f));
    ctx->add(std::make_shared<FnInfo>("tan", std::vector<std::shared_ptr<ResolvedType>>{ f }, f));
    ctx->add(std::make_shared<FnInfo>("asin", std::vector<std::shared_ptr<ResolvedType>>{ f }, f));
    ctx->add(std::make_shared<FnInfo>("acos", std::vector<std::shared_ptr<ResolvedType>>{ f }, f));
    ctx->add(std::make_shared<FnInfo>("atan", std::vector<std::shared_ptr<ResolvedType>>{ f }, f));
    ctx->add(std::make_shared<FnInfo>("log", std::vector<std::shared_ptr<ResolvedType>>{ f }, f));
    ctx->add(std::make_shared<FnInfo>("pow", std::vector<std::shared_ptr<ResolvedType>>{ f, f }, f));
    ctx->add(std::make_shared<FnInfo>("atan2", std::vector<std::shared_ptr<ResolvedType>>{ f, f }, f));
    ctx->add(std::make_shared<FnInfo>("to_int", std::vector<std::shared_ptr<ResolvedType>>{ f }, i));
    ctx->add(std::make_shared<FnInfo>("to_float", std::vector<std::shared_ptr<ResolvedType>>{ i }, f));
    ASTVisitor::visit(program);
  };

  virtual void visit(const ReadCmd& cmd) override {
    ASTVisitor::visit(cmd);
    auto name = cmd.lvalue->identifier;
    if (ctx->lookup<NameInfo>(name)) {
      logger.log_error("Redeclaration of variable", 0);
    }
    if (auto array_lvalue = dynamic_cast<ArrayLValue*>(cmd.lvalue.get())) {
      if (array_lvalue->indices.size() != 2) {
        logger.log_error("Read cmd LValue must be of rank 2", 0);
      }
    }
    auto rgba = std::make_shared<Struct>("rgba");
    auto type = std::make_shared<Array>(rgba, 2);
    auto info = std::make_shared<ValueInfo>(name, type);
    ctx->add(info);
  }

  virtual void visit(const WriteCmd& cmd) override {
    ASTVisitor::visit(cmd);
    auto expr_type = cmd.expr->type;
    if (auto array = expr_type->as<Array>()) {
      if (auto element = array->element_type->as<Struct>()) {
        if (element->name != "rgba") {
          logger.log_error("Must be array of struct of type rgba", 0);
        }
      } else {
        logger.log_error("Must be array of struct", 0);
      }
    } else {
      logger.log_error("Must write array", 0);
    }
  }

  virtual void visit(const LetCmd& cmd) override {
    // ASTVisitor::visit(cmd);
    cmd.expr->accept(*this);
    cmd.lvalue->accept(*this);
    auto name = cmd.lvalue->identifier;
    if (ctx->lookup<NameInfo>(name)) {
      logger.log_error("Redeclaration of variable", 0);
    }
    auto type = cmd.expr->type;
    if (auto array_lvalue = dynamic_cast<ArrayLValue*>(cmd.lvalue.get())) {
      if (auto array_type = type->as<Array>()) {
        if (array_lvalue->indices.size() != array_type->rank) {
          logger.log_error("Array LValue had incorrect rank", 0);
        }
      } else {
        logger.log_error("Array LValue had incorrect rank", 0);
      }
    }
    auto info = std::make_shared<ValueInfo>(name, type);
    ctx->add(info);
  }

  virtual void visit(const LetStmt& cmd) override {
    // ASTVisitor::visit(cmd);
    cmd.expr->accept(*this);
    cmd.lvalue->accept(*this);
    auto name = cmd.lvalue->identifier;
    if (ctx->lookup<NameInfo>(name)) {
      logger.log_error("Redeclaration of variable", 0);
    }
    auto type = cmd.expr->type;
    if (auto array_lvalue = dynamic_cast<ArrayLValue*>(cmd.lvalue.get())) {
      if (auto array_type = type->as<Array>()) {
        if (array_lvalue->indices.size() != array_type->rank) {
          logger.log_error("Array LValue had incorrect rank", 0);
        }
      } else {
        logger.log_error("Array LValue had incorrect rank", 0);
      }
    }
    auto info = std::make_shared<ValueInfo>(name, type);
    ctx->add(info);
  }

  virtual void visit(const AssertCmd& cmd) override {
    ASTVisitor::visit(cmd);
    if (!cmd.expr->type->is<Bool>()) {
      logger.log_error("Assert condition must be of type bool", 0);
    }
  }

  virtual void visit(const AssertStmt& cmd) override {
    ASTVisitor::visit(cmd);
    if (!cmd.expr->type->is<Bool>()) {
      logger.log_error("Assert condition must be of type bool", 0);
    }
  }

  virtual void visit(const ReturnStmt& stmt) override {
    ASTVisitor::visit(stmt);
    if (expected_return_type == nullptr || *stmt.expr->type != *expected_return_type) {
      logger.log_error("Bad return type", 0);
    }
    returned = true;
    // expected_return_type = nullptr;
  }

  virtual void visit(const FnCmd& cmd) override {
    if (ctx->lookup<NameInfo>(cmd.identifier)) {
      logger.log_error("Redeclaration of function", 0);
    }
    for (const auto &binding : cmd.params) {
      binding->type->accept(*this);
    }
    cmd.return_type->accept(*this);
    auto name = cmd.identifier;
    auto return_type = cmd.return_type->type;
    std::vector<std::shared_ptr<ResolvedType>> param_types;
    for (const auto& binding : cmd.params) {
      param_types.push_back(binding->type->type);
    }
    auto info = std::make_shared<FnInfo>(name, param_types, return_type);
    ctx->add(info);
    auto parent_ctx = ctx;
    ctx = std::make_shared<Context>(parent_ctx);
    for (const auto& binding : cmd.params) {
      binding->accept(*this);
    }
    expected_return_type = return_type;
    returned = false;
    for (const auto &stmt : cmd.stmts) {
      stmt->accept(*this);
    }
    if (!returned && !return_type->is<Void>()) {
      logger.log_error("Missing return type", 0);
    }
    ctx = parent_ctx;
  }

  virtual void visit(const StructCmd& cmd) override {
    ASTVisitor::visit(cmd);
    auto name = cmd.identifier;
    std::unordered_set<std::string> field_names;
    std::vector<std::pair<std::string, std::shared_ptr<ResolvedType>>> fields;
    for (const auto& [name, type] : cmd.fields) {
      if (field_names.count(name)) {
        logger.log_error("Redeclaration of struct field", 0);
      }
      field_names.insert(name);
      fields.emplace_back(std::make_pair(name, type->type));
    }
    auto info = std::make_shared<StructInfo>(name, fields);
    ctx->add(info);
  };

  virtual void visit(const IntType& int_type) override {
    int_type.type = std::make_shared<Int>();
  };

  virtual void visit(const FloatType& float_type) override {
    float_type.type = std::make_shared<Float>();
  };

  virtual void visit(const BoolType& bool_type) override {
    bool_type.type = std::make_shared<Bool>();
  };

  virtual void visit(const ArrayType& array_type) override {
    ASTVisitor::visit(array_type);
    array_type.type = std::make_shared<Array>(array_type.element_type->type, array_type.rank);
  };

  virtual void visit(const StructType& struct_type) override {
    auto name = struct_type.identifier;
    if (!ctx->lookup<StructInfo>(name)) {
      logger.log_error("Use of undeclared struct", 0);
    }
    struct_type.type = std::make_shared<Struct>(struct_type.identifier);
  };

  virtual void visit(const VoidType& void_type) override {
    void_type.type = std::make_shared<Void>();
  };

  virtual void visit(const IntExpr& expr) override {
    expr.type = std::make_shared<Int>();
  };

  virtual void visit(const FloatExpr& expr) override {
    expr.type = std::make_shared<Float>();
  };

  virtual void visit(const TrueExpr& expr) override {
    expr.type = std::make_shared<Bool>();
  };

  virtual void visit(const FalseExpr& expr) override {
    expr.type = std::make_shared<Bool>();
  };

  virtual void visit(const VarExpr &expr) override {
    auto info = ctx->lookup<ValueInfo>(expr.identifier);
    if (auto info = ctx->lookup<ValueInfo>(expr.identifier)) {
      expr.type = info->type;
    } else {
      logger.log_error("Use of undeclared variable", 0);
    }
  };

  virtual void visit(const VoidExpr &expr) override {
    expr.type = std::make_shared<Void>();
  }

  virtual void visit(const BinopExpr& expr) override {
    ASTVisitor::visit(expr);
    if (*expr.left->type != *expr.right->type) {
      logger.log_error("left and right must match!", 0);
    }
    if (std::unordered_set<std::string>{"==", "!="}.count(expr.op)) {
      expr.type = std::make_shared<Bool>();
    } else if (std::unordered_set<std::string>{"&&", "||"}.count(expr.op)) {
      if (expr.left->type->is<Bool>()) {
        expr.type = expr.left->type;
      } else {
        logger.log_error("Operands must be bool", 0);
      }
    } else if (std::unordered_set<std::string>{"<", ">", "<=", ">="}.count(expr.op)) {
      if (expr.left->type->is<Int>() || expr.left->type->is<Float>()) {
        expr.type = std::make_shared<Bool>();
      } else {
        logger.log_error("Operands must be of a numerical type", 0);
      }
    } else if (std::unordered_set<std::string>{"+", "-", "*", "/", "%"}.count(expr.op)) {
      if (expr.left->type->is<Int>() || expr.left->type->is<Float>()) {
        expr.type = expr.left->type;
      } else {
        logger.log_error("Operands must be of a numerical type", 0);
      }
    }
  };

  virtual void visit(const UnopExpr& expr) override {
    ASTVisitor::visit(expr);
    expr.type = expr.expr->type;
  };

  virtual void visit(const StructLiteralExpr& expr) override {
    ASTVisitor::visit(expr);
    if (auto info = ctx->lookup<StructInfo>(expr.identifier)) {
      if (expr.fields.size() != info->fields.size()) {
        logger.log_error("Wrong number of fields", 0);
      }
      for (int i = 0; i < expr.fields.size(); i++) {
        auto expr_type = expr.fields[i]->type;
        auto info_type = info->fields[i].second;
        if (*expr_type != *info_type) {
          logger.log_error("Wrong type in struct field", 0);
        }
      }
    } else {
      logger.log_error("Use of undeclared struct", 0);
    }
    expr.type = std::make_shared<Struct>(expr.identifier);
  };

  virtual void visit(const ArrayLiteralExpr& expr) override {
    ASTVisitor::visit(expr);
    auto element_type = expr.elements.empty() ? std::make_shared<Void>() : expr.elements.front()->type;
    for (const auto& element : expr.elements) {
      if (*element_type != *element->type) {
        logger.log_error("All elements in array literal must be of the same type", 0);
      }
    }
    expr.type = std::make_shared<Array>(element_type, 1);
  };

  virtual void visit(const IfExpr& expr) override {
    ASTVisitor::visit(expr);
    if (!expr.condition->type->is<Bool>()) {
      logger.log_error("Condition on ternary must be of type boolean", 0);
    }
    if (*expr.if_expr->type != *expr.else_expr->type) {
      logger.log_error("Both branches of ternary must be of same type", 0);
    }
    expr.type = expr.if_expr->type;
  };

  virtual void visit(const DotExpr& expr) override {
    ASTVisitor::visit(expr);
    if (auto struct_type = expr.expr->type->as<Struct>()) {
      if (auto info = ctx->lookup<StructInfo>(struct_type->name)) {
        for (const auto& [name, type] : info->fields) {
          if (name == expr.field) {
            expr.type = type;
          }
        }
      } else {
        logger.log_error("Somehow has type of undeclared struct", 0);
      }
    } else {
      logger.log_error("Can only access fields of struct objects", 0);
    }
  };

  virtual void visit(const ArrayIndexExpr& expr) override {
    ASTVisitor::visit(expr);
    if (auto array_type = expr.expr->type->as<Array>()) {
      if (expr.indices.size() != array_type->rank) {
        logger.log_error("Index is of incorrect rank", 0);
      }
      for (const auto& index : expr.indices) {
        if (!index->type->is<Int>()) {
          logger.log_error("Only ints can be used to index arrays", 0);
        }
      }
      expr.type = array_type->element_type;
    } else {
      logger.log_error("Can only index array objects", 0);
    }
  };

  virtual void visit(const CallExpr& expr) override {
    ASTVisitor::visit(expr);
    // TODO: typecheck params
    if (auto info = ctx->lookup<FnInfo>(expr.identifier)) {
      if (expr.args.size() != info->param_types.size()) {
        logger.log_error("Incorrect number of parameters", 0);
      }
      for (int i = 0; i < expr.args.size(); i++) {
        auto call_type = expr.args[i]->type;
        auto info_type = info->param_types[i];
        if (*call_type != *info_type) {
          logger.log_error("Wrong parameter type", 0);
        }
      }
      expr.type = info->return_type;
    } else {
      logger.log_error("Trying to call undeclared function", 0);
    }
  };

  virtual void visit(const ArrayLoopExpr& expr) override {
    auto parent_ctx = ctx;
    ctx = std::make_shared<Context>(parent_ctx);
    if (expr.axis.empty()) {
      logger.log_error("Array loop expression cannot be empty", 0);
    }
    for (const auto& [axis_name, axis_expr] : expr.axis) {
      axis_expr->accept(*this);
      if (!axis_expr->type->is<Int>()) {
        logger.log_error("Bounds of sum loop expression must be of type integer", 0);
      }
    }
    for (const auto& [axis_name, axis_expr] : expr.axis) {
      auto info = std::make_shared<ValueInfo>(axis_name, axis_expr->type);
      ctx->add(info);
    }
    expr.expr->accept(*this);
    expr.type = std::make_shared<Array>(expr.expr->type, expr.axis.size());
    ctx = parent_ctx;
  };

  virtual void visit(const SumLoopExpr& expr) override {
    auto parent_ctx = ctx;
    ctx = std::make_shared<Context>(parent_ctx);
    if (expr.axis.empty()) {
      logger.log_error("Array loop expression cannot be empty", 0);
    }
    for (const auto& [axis_name, axis_expr] : expr.axis) {
      axis_expr->accept(*this);
      if (!axis_expr->type->is<Int>()) {
        logger.log_error("Bounds of sum loop expression must be of type integer", 0);
      }
    }
    for (const auto& [axis_name, axis_expr] : expr.axis) {
      auto info = std::make_shared<ValueInfo>(axis_name, axis_expr->type);
      ctx->add(info);
    }
    expr.expr->accept(*this);
    if (expr.expr->type->is<Int>()) {
    }
    if (!(expr.expr->type->is<Int>() || expr.expr->type->is<Float>())) {
      logger.log_error("Sum loop expression must be of numeric type", 0);
    }
    expr.type = expr.expr->type;
    ctx = parent_ctx;
  };

  virtual void visit(const ArrayLValue& lvalue) override {
    for (const auto& index : lvalue.indices) {
      if (ctx->lookup<NameInfo>(lvalue.identifier)) {
        logger.log_error("Redeclaration of identifier", 0);
      }
      auto info = std::make_shared<ValueInfo>(index, std::make_shared<Int>());
      ctx->add(info);
      // std::cout << "added 
    }
    // auto info = std::make_shared<ValueInfo>(lvalue.identifier, std::shared_ptr<
  };

  virtual void visit(const Binding& binding) override {
    ASTVisitor::visit(binding);
    if (ctx->lookup<NameInfo>(binding.lvalue->identifier)) {
      logger.log_error("Redeclaration of identifier", 0);
    }
    auto name = binding.lvalue->identifier;
    auto type = binding.type->type;
    auto info = std::make_shared<ValueInfo>(name, type);
    ctx->add(info);
  };

private:
  Logger& logger;
  std::shared_ptr<Context> ctx;
  bool returned;
  std::shared_ptr<ResolvedType> expected_return_type;
};
