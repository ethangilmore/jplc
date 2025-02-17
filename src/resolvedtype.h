#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <unordered_set>

class ResolvedType {
public:
  virtual ~ResolvedType() = 0;
  virtual std::string to_string() = 0;

  bool operator==(const ResolvedType& other) const {
    return typeid(*this) == typeid(other);
  }

  bool operator!=(const ResolvedType& other) const {
    return !(*this == other);
  }

  template<typename T>
  bool is() {
    return dynamic_cast<T*>(this) != nullptr;
  }

  template <typename T>
  std::shared_ptr<T> as() {
    if (auto ptr = dynamic_cast<T*>(this)) {
      return std::shared_ptr<T>(ptr, [](T*){}); // Managed externally
    }
    return nullptr;
  }
};

class Int : public ResolvedType {
  virtual std::string to_string() override;
};

class Float : public ResolvedType {
  virtual std::string to_string() override;
};

class Bool : public ResolvedType {
  virtual std::string to_string() override;
};

class Void : public ResolvedType {
  virtual std::string to_string() override;
};

class Struct : public ResolvedType {
public:
  virtual std::string to_string() override;
  Struct(std::string name);
  std::string name;
};

class Array : public ResolvedType {
public:
  virtual std::string to_string() override;
  Array(std::shared_ptr<ResolvedType> element_type, size_t rank);
  std::shared_ptr<ResolvedType> element_type;
  size_t rank;
};
