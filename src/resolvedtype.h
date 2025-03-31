#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>

class Context;

class ResolvedType {
 public:
  virtual ~ResolvedType() = 0;
  virtual std::string to_string() = 0;
  virtual std::string c_type() = 0;
  virtual std::string show_type(Context* ctx) { return to_string(); };
  virtual int size() { return 8; }

  bool operator==(const ResolvedType& other) const {
    return typeid(*this) == typeid(other);
  }

  bool operator!=(const ResolvedType& other) const {
    return !(*this == other);
  }

  template <typename T>
  bool is() {
    return dynamic_cast<T*>(this) != nullptr;
  }

  template <typename T>
  std::shared_ptr<T> as() {
    if (auto ptr = dynamic_cast<T*>(this)) {
      return std::shared_ptr<T>(ptr, [](T*) {});  // Managed externally
    }
    return nullptr;
  }
};

class Int : public ResolvedType {
 public:
  static std::shared_ptr<Int> shared;
  virtual std::string to_string() override;
  virtual std::string c_type() override;
};

class Float : public ResolvedType {
 public:
  static std::shared_ptr<Float> shared;
  virtual std::string to_string() override;
  virtual std::string c_type() override;
};

class Bool : public ResolvedType {
 public:
  static std::shared_ptr<Bool> shared;
  virtual std::string to_string() override;
  virtual std::string c_type() override;
};

class Void : public ResolvedType {
 public:
  static std::shared_ptr<Void> shared;
  virtual std::string to_string() override;
  virtual std::string c_type() override;
};

class Struct : public ResolvedType {
 public:
  virtual std::string to_string() override;
  virtual std::string c_type() override;
  virtual std::string show_type(Context* ctx) override;
  Struct(std::string name);
  std::string name;
};

class Array : public ResolvedType {
 public:
  virtual std::string to_string() override;
  virtual std::string c_type() override;
  virtual std::string show_type(Context* ctx) override;
  virtual int size() override;
  Array(std::shared_ptr<ResolvedType> element_type, size_t rank);
  std::shared_ptr<ResolvedType> element_type;
  size_t rank;
};
