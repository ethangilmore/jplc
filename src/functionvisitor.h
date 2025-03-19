#pragma once

#include <memory>

#include "astnodes.h"
#include "astvisitor.h"
#include "context.h"
#include "logger.h"

class CodeGenVisitor;

class FunctionGenerator : public ASTVisitor {
 public:
  FunctionGenerator(CodeGenVisitor* code_gen, std::shared_ptr<Context> ctx, Logger& logger);

  void visit(const Program& program) override;
  void visit(const FnCmd& fn) override;

 private:
  CodeGenVisitor* code_gen;
  std::shared_ptr<Context> ctx;
  Logger& logger;
};
