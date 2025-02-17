#include "astvisitor.h"
#include "logger.h"
#include "context.h"
#include "astnodes.h"
#include <iostream>
#include <memory>

class TypeCheckerVisitor : public ASTVisitor {
public:
  TypeCheckerVisitor(Logger& logger) : logger(logger) {}

  virtual void visit(const Program& program) override {
    ctx = std::make_shared<Context>();
    auto f = std::make_shared<Float>();
    auto rgba = std::make_shared<StructInfo>( "rgba", 
      std::vector<std::pair<std::string, std::shared_ptr<ResolvedType>>>{
        {"r", f}, {"g", f}, {"b", f}, {"a", f},
      }
    );
    ctx->add(rgba);
    ASTVisitor::visit(program);
  };

  virtual void visit(const StructCmd& cmd) override {
    ASTVisitor::visit(cmd);
    auto name = cmd.identifier;
    std::vector<std::pair<std::string, std::shared_ptr<ResolvedType>>> fields;
    for (const auto& [name, type] : cmd.fields) {
      fields.emplace_back(std::make_pair(name, type->type));
    }
    auto info = std::make_shared<StructInfo>(name, fields);
    ctx->add(info);
  };

  virtual void visit(const IntType& int_type) override {
    int_type.type = std::make_unique<Int>();
  };

  virtual void visit(const FloatType& float_type) override {
    float_type.type = std::make_unique<Float>();
  };

  virtual void visit(const BoolType& bool_type) override {
    bool_type.type = std::make_unique<Bool>();
  };

  virtual void visit(const ArrayType& array_type) override {
    ASTVisitor::visit(array_type);
    array_type.type = std::make_shared<Array>(array_type.element_type->type, array_type.rank);
  };

  virtual void visit(const StructType& struct_type) override {
    struct_type.type = std::make_shared<Struct>(struct_type.identifier);
  };

  virtual void visit(const VoidType& void_type) override {
    void_type.type = std::make_shared<Void>();
  };

  virtual void visit(const IntExpr& expr) override {
    expr.type = std::make_unique<Int>();
  };

  virtual void visit(const FloatExpr& expr) override {
    expr.type = std::make_unique<Float>();
  };

  virtual void visit(const TrueExpr& expr) override {
    expr.type = std::make_unique<Bool>();
  };

  virtual void visit(const FalseExpr& expr) override {
    expr.type = std::make_unique<Bool>();
  };

  virtual void visit(const BinopExpr& expr) override {
    expr.left->accept(*this);
    expr.right->accept(*this);
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
    expr.expr->accept(*this);
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
    expr.type = std::make_unique<Struct>(expr.identifier);
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
  }

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
  }

private:
  Logger& logger;
  std::shared_ptr<Context> ctx;
};
