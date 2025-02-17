#include <string>

#include "context.h"

NameInfo::NameInfo(std::string name) : name(name) {}

NameInfo::~NameInfo() {}

ValueInfo::ValueInfo(std::string name, std::shared_ptr<ResolvedType> type) : NameInfo(name), type(type) {}

StructInfo::StructInfo(std::string name, std::vector<std::pair<std::string, std::shared_ptr<ResolvedType>>> fields) : NameInfo(name), fields(fields) {}

Context::Context() {};

Context::Context(Context* parent) : parent(parent) {}

void Context::add(std::shared_ptr<NameInfo> info) {
  table[info->name] = std::move(info);
};

