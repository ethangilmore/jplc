#pragma once

#include "asmfnvisitor.h"

#include "asmgenvisitor.h"
#include "astnodes.h"

void ASMFnVisitor::visit(const Program& program) {
  ASTVisitor::visit(program);
}

void ASMFnVisitor::visit(const FnCmd& fn) {
  // TODO: function stuff
  asm_visitor.visit(fn);
}
