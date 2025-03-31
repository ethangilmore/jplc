#pragma once

#include "astvisitor.h"

class ASMGenVisitor;

class ASMFnVisitor : public ASTVisitor {
 public:
  ASMFnVisitor(ASMGenVisitor& asm_visitor) : asm_visitor(asm_visitor) {}

  virtual void visit(const Program& program) override;
  virtual void visit(const FnCmd& fn) override;

 private:
  ASMGenVisitor& asm_visitor;
};
