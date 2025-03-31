#include "resolvedtype.h"

#include "context.h"

ResolvedType::~ResolvedType() {}

std::string Int::to_string() {
  return "(IntType)";
}

std::string Int::c_type() {
  return "int64_t";
}

std::shared_ptr<Int> Int::shared = std::make_shared<Int>();

std::string Float::to_string() {
  return "(FloatType)";
}

std::string Float::c_type() {
  return "double";
}

std::shared_ptr<Float> Float::shared = std::make_shared<Float>();

std::string Bool::to_string() {
  return "(BoolType)";
}

std::string Bool::c_type() {
  return "bool";
}

std::shared_ptr<Bool> Bool::shared = std::make_shared<Bool>();

std::string Void::to_string() {
  return "(VoidType)";
}

std::string Void::c_type() {
  return "void_t";
}

std::shared_ptr<Void> Void::shared = std::make_shared<Void>();

Array::Array(std::shared_ptr<ResolvedType> element_type, size_t rank) : element_type(element_type), rank(rank) {}

std::string Array::to_string() {
  return "(ArrayType " + element_type->to_string() + " " + std::to_string(rank) + ")";
}

std::string Array::c_type() {
  return "_a" + std::to_string(rank) + "_" + element_type->c_type();
}

std::string Array::show_type(Context* ctx) {
  return "(ArrayType " + element_type->show_type(ctx) + " " + std::to_string(rank) + ")";
}

int Array::size() {
  return 8 + (rank * 8);
}

Struct::Struct(std::string name) : name(name) {}

std::string Struct::to_string() {
  return "(StructType " + name + ")";
}

std::string Struct::c_type() {
  return name;
}

std::string Struct::show_type(Context* ctx) {
  std::string result = "(TupleType ";
  auto info = ctx->lookup<StructInfo>(name);
  bool first = true;
  for (const auto& [name, type] : info->fields) {
    if (first) {
      result += type->show_type(ctx);
      first = false;
    } else {
      result += " " + type->show_type(ctx);
    }
  }
  result += ")";
  return result;
}
