#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "context.h"
#include "resolvedtype.h"

class Context;

struct NameInfo {
public:
  NameInfo(std::string name);
  virtual ~NameInfo() = 0;
  std::string name;
};

struct ValueInfo : public NameInfo {
public:
  ValueInfo(std::string name, std::shared_ptr<ResolvedType> type);
  std::shared_ptr<ResolvedType> type;
};

struct StructInfo : public NameInfo {
public:
  StructInfo(std::string name, std::vector<std::pair<std::string, std::shared_ptr<ResolvedType>>>);
  std::vector<std::pair<std::string, std::shared_ptr<ResolvedType>>> fields;
};

class Context {
public:
  Context();
  Context(Context* parent);

  void add(std::shared_ptr<NameInfo> info);

  template <typename T>
  std::optional<T> lookup(std::string identifier) const {
    if (auto info = table.find(identifier); info != table.end()) {
      auto casted = std::dynamic_pointer_cast<T>(info->second);
      if (casted) {
        return *casted;
      }
    }
    return std::nullopt;
  }
  
private:
  std::shared_ptr<Context> parent;
  std::unordered_map<std::string, std::shared_ptr<NameInfo>> table;
};
