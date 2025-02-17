#include "resolvedtype.h"

ResolvedType::~ResolvedType() {}

std::string Int::to_string() {
  return "(IntType)";
}

std::string Float::to_string() {
  return "(FloatType)";
}

std::string Bool::to_string() {
  return "(BoolType)";
}

std::string Void::to_string() {
  return "(VoidType)";
}

Array::Array(std::shared_ptr<ResolvedType> element_type, size_t rank) : element_type(element_type), rank(rank) {}

std::string Array::to_string() {
  return "(ArrayType " + element_type->to_string() + " " + std::to_string(rank) + ")";
}

Struct::Struct(std::string name) : name(name) {}

std::string Struct::to_string() {
  return "(StructType " + name + ")"; 
}

