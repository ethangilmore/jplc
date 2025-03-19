#include "functionvisitor.h"

#include <iostream>

#include "codegenvisitor.h"

FunctionGenerator::FunctionGenerator(CodeGenVisitor* code_gen, std::shared_ptr<Context> ctx, Logger& logger)
    : code_gen(code_gen), ctx(ctx), logger(logger) {}

void FunctionGenerator::visit(const Program& program) {
  ASTVisitor::visit(program);
}

void FunctionGenerator::visit(const FnCmd& fn) {
  std::cout << fn.return_type->type->c_type() + " " + fn.identifier + "(";
  if (!fn.params.empty()) {
    std::cout << fn.params[0]->type->type->c_type() + " " + fn.params[0]->lvalue->identifier;
    for (size_t i = 1; i < fn.params.size(); i++) {
      std::cout << ", " + fn.params[i]->type->type->c_type() + " " + fn.params[i]->lvalue->identifier;
    }
  }
  std::cout << ") {\n";

  code_gen->reset_name_ctr();
  for (const auto& stmt : fn.stmts) {
    stmt->accept(*code_gen);
  }

  std::cout << "}\n\n";
}
