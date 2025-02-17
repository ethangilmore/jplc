#include "typechecker.h"
#include <memory>
#include <unordered_set>
#include "context.h"

TypeChecker::TypeChecker(Logger& logger) : logger(logger) {}

void TypeChecker::typecheck(const Program& program) {
  auto ctx = std::make_shared<Context>();
  for (const auto& cmd : program.cmds) {
    typecheck(*cmd, *ctx);
  }
}

void TypeChecker::typecheck(const ShowCmd& showCmd, Context& ctx) {
  type_of(*showCmd.expr, ctx);
}

void TypeChecker::typecheck(const StructCmd& structCmd, Context& ctx) {
  auto info = std::make_shared<StructInfo>(structCmd.identifier, structCmd.fields);
  ctx.add(info);
}

void TypeChecker::typecheck(const Cmd& cmd, Context& ctx) {
  throw std::runtime_error("something is wrong");
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const Expr& expr, Context& ctx) {
  throw std::runtime_error("something is wrong");
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const IntExpr& expr, Context& ctx) {
  expr.type = std::make_shared<Int>();
  return expr.type;
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const FloatExpr& expr, Context& ctx) {
  expr.type = std::make_shared<Float>();
  return expr.type;
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const TrueExpr& expr, Context& ctx) {
  expr.type = std::make_shared<Bool>();
  return expr.type;
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const FalseExpr& expr, Context& ctx) {
  expr.type = std::make_shared<Bool>();
  return expr.type;
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const VoidExpr& expr, Context& ctx) {
  expr.type = std::make_shared<Void>();
  return expr.type;
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const VarExpr& expr, Context& ctx) {
  if (auto info = ctx.lookup<ValueInfo>(expr.identifier)) {
    expr.type = info->type;
    return expr.type;
  }
  logger.log_error("Use of undefined value", 0);
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const BinopExpr& expr, Context& ctx) {
  std::unordered_set<std::string> bool_ops {"&&", "||", "==", "!="};
  std::unordered_set<std::string> number_ops {"+", "-", "*", "/", "%", "<", ">", "<=", ">="};
  auto left_type = type_of(*expr.left, ctx);
  auto right_type = type_of(*expr.right, ctx);
  if (left_type != right_type) {
    logger.log_error("Both operands must be of the same type", 0);
  }
  if (bool_ops.count(expr.op) && !left_type->is<Bool>())
  {
    logger.log_error("Must be bool", 0);
  }
  if (number_ops.count(expr.op) && !(left_type->is<Int>() || left_type->is<Float>())) {
    logger.log_error("Numbers must both be int or both be floats", 0);
  }
  expr.type = left_type;
  return expr.type;
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const UnopExpr& expr, Context& ctx) {
  auto type = type_of(*expr.expr, ctx);
  if (expr.op == "-" && !(type->is<Int>() || type->is<Float>())) {
    logger.log_error("Negation operator can only be applied to ints and floats", 0);
  } else if (expr.op == "!" && !type->is<Bool>()) {
    logger.log_error("Not operator can only be applied to bools", 0);
  }
  expr.type = type;
  return expr.type;
}

// std::shared_ptr<ResolvedType> TypeChecker::type_of(const StructCmd& expr, Context& ctx) {
//   // add to ctx
//   return std::make_shared<Struct>(expr.identifier);
// }

std::shared_ptr<ResolvedType> TypeChecker::type_of(const ArrayLiteralExpr& expr, Context& ctx) {
  auto type = expr.elements.empty() ? std::make_shared<Void>() : expr.elements[0]->type;
  expr.type = std::make_shared<Array>(type, expr.elements.size());
  return expr.type;
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const IfExpr& expr, Context& ctx) {
  auto condition = type_of(*expr.condition, ctx);
  if (!condition->is<Bool>()) {
    logger.log_error("Condition of ternary must be a bool", 0);
  }
  auto if_expr = type_of(*expr.if_expr, ctx);
  auto else_expr = type_of(*expr.else_expr, ctx);
  if (if_expr != else_expr) {
    logger.log_error("Both branches of ternary must be equal types", 0);
  }
  expr.type = if_expr;
  return expr.type;
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const DotExpr& expr, Context& ctx) {
  auto type = type_of(*expr.expr, ctx);
  std::shared_ptr<Struct> struct_type = std::dynamic_pointer_cast<Struct>(type);
  if (auto struct_type = type_of(*expr.expr, ctx)->as<Struct>())
  {
    auto info = ctx.lookup<StructInfo>(struct_type->name);
    for (const auto& [name, type] : info->fields) {
      if (name == expr.field) {
        expr.type = type;
        return expr.type;
      }
    }
    logger.log_error("Trying to access field which does not exist on struct object", 0);
  } else {
    logger.log_error("Trying to access field of non-struct object", 0);
  }
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(const ArrayIndexExpr& expr, Context& ctx) {
  if (auto type = type_of(*expr.expr, ctx)->as<Array>()) {
    if (type->rank != expr.indices.size()) {
      logger.log_error("Index has incorrect rank", 0);
    }
    expr.type = type->element_type;
    return expr.type;
  } else {
    logger.log_error("Trying to access index of non array object", 0);
  }
}
