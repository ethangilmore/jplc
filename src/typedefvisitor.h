#pragma once

#include "astnodes.h"
#include "astvisitor.h"
#include "context.h"
#include "logger.h"

class TypeDefGenerator : public ASTVisitor {
 public:
  TypeDefGenerator(std::shared_ptr<Context> ctx, Logger& logger) : ctx(ctx), logger(logger) {}

  virtual void visit(const Program& program) override {
    // print out header
    std::cout << "#include <math.h>\n"
                 "#include <stdbool.h>\n"
                 "#include <stdint.h>\n"
                 "#include <stdio.h>\n"
                 "#include \"rt/runtime.h\"\n\n"
                 "typedef struct { } void_t;\n\n";
    created_types.insert("rgba");
    ASTVisitor::visit(program);
  }

  virtual void visit(const StructCmd& st) override {
    ASTVisitor::visit(st);
    if (!created_types.count(st.identifier)) {
      std::cout << "typedef struct {\n";
      for (const auto& [name, type] : st.fields) {
        std::cout << "  " + type->type->c_type() + " " + name + ";\n";
      }
      std::cout << "} " + st.identifier + ";\n\n";
    }
    created_types.insert(st.identifier);
  }

  virtual void visit(const ArrayType& array) override {
    ASTVisitor::visit(array);
    if (!created_types.count(array.type->c_type())) {
      std::cout << "typedef struct {\n";
      for (size_t i = 0; i < array.rank; i++) {
        std::cout << "  int64_t d" + std::to_string(i) + ";\n";
      }
      std::cout << "  " + array.element_type->type->c_type() + " *data;\n";
      std::cout << "} " + array.type->c_type() + ";\n\n";
    }
    created_types.insert(array.type->c_type());
  }

  virtual void visit(const ArrayLiteralExpr& array) override {
    ASTVisitor::visit(array);
    auto element_type = array.type->as<Array>()->element_type->c_type();
    auto type = "_a1_" + element_type;
    if (!created_types.count(type)) {
      std::cout << "typedef struct {\n";
      std::cout << "int64_t d0;\n";
      std::cout << element_type + " *data;\n";
      std::cout << "} " + type + ";\n\n";
    }
    created_types.insert(type);
  }

 private:
  std::unordered_set<std::string> created_types;
  std::shared_ptr<Context> ctx;
  Logger& logger;
};
