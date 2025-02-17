#pragma once

#include <memory>
#include <string>

class ResolvedType {
public:
  virtual ~ResolvedType() = 0;

  template<typename T>
  bool is() {
    return dynamic_cast<T*>(this) != nullptr;
  }

  template<typename T>
  std::shared_ptr<T> as() {
    return dynamic_cast<T*>(this);
  }
};

class Int : public ResolvedType {
};

class Float : public ResolvedType {};

class Bool : public ResolvedType {};

class Void : public ResolvedType {};

class Struct : public ResolvedType {
public:
  Struct(std::string name);
  std::string name;
};

class Array : public ResolvedType {
public:
  std::shared_ptr<ResolvedType> element_type;
  size_t rank;
};
