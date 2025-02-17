#pragma once

#include "astnodes.h"
#include "context.h"
#include "resolvedtype.h"
#include "logger.h"

class TypeChecker {
public:
  TypeChecker(Logger& logger);

  // std::shared_ptr<Context> typecheck(const Program& program);
  // std::shared_ptr<Context> typecheck(const ShowCmd& showCmd, Context& ctx);
  // std::shared_ptr<Context> typecheck(const StructCmd& structCmd, Context& ctx);
  // std::shared_ptr<Context> typecheck(const Cmd& cmd, Context& ctx);
  void typecheck(const Program& program);
  void typecheck(const ShowCmd& showCmd, Context& ctx);
  void typecheck(const StructCmd& structCmd, Context& ctx);
  void typecheck(const Cmd& cmd, Context& ctx);

  std::shared_ptr<ResolvedType> type_of(const Expr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const IntExpr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const FloatExpr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const TrueExpr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const FalseExpr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const VoidExpr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const VarExpr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const BinopExpr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const UnopExpr& expr, Context& ctx);
  // std::shared_ptr<ResolvedType> type_of(const StructCmd& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const ArrayLiteralExpr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const IfExpr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const DotExpr& expr, Context& ctx);
  std::shared_ptr<ResolvedType> type_of(const ArrayIndexExpr& expr, Context& ctx);

private:
  Logger& logger;
};
