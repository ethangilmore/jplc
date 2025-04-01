#pragma once

#include <cassert>
#include <map>
#include <optional>
#include <stack>
#include <variant>

#include "asmdatavisitor.h"
#include "asmfnvisitor.h"
#include "astvisitor.h"
#include "context.h"
#include "logger.h"
#include "resolvedtype.h"

class Stack {
 public:
  int size = 0;
  int local_var_size = 0;
  std::stack<std::optional<std::shared_ptr<ResolvedType>>> shadow;
  std::stack<int> padding;
  std::map<std::string, int> variables;

  std::shared_ptr<ResolvedType> top() {
    return *shadow.top();
  }

  int push(std::shared_ptr<ResolvedType> type) {
    auto s = type->size();
    size += s;
    shadow.push(type);
    // std::cout << "push: stack now has " << shadow.size() << " things on it\n";
    return s;
  }

  std::shared_ptr<ResolvedType> pop() {
    if (shadow.top()) {
      // std::cout << (*shadow.top())->to_string() << '\n';
    } else {
      std::cout << "no type on top... oops!\n";
    }
    auto type = *shadow.top();
    size -= type->size();
    shadow.pop();
    // std::cout << "pop: stack now has " << shadow.size() << " things on it\n";
    return type;
  }

  int align(int add) {
    if ((size + add) % 16 == 0) {
      padding.push(0);
      return 0;
    }
    auto leftovers = 16 - ((size + add) % 16);
    size += leftovers;
    padding.push(leftovers);
    shadow.push(std::nullopt);
    // std::cout << "align: stack now has " << shadow.size() << " things on it\n";
    return leftovers;
  }

  int unalign() {
    auto p = padding.top();
    padding.pop();
    size -= p;
    if (p != 0) {
      if (shadow.top()) {
        std::cout << "uh oh we're trying to unalign but theres data instead of padding\n";
      }
      shadow.pop();
    }
    // std::cout << "unalign: stack now has " << shadow.size() << " things on it\n";
    return p;
  }

  void recharacterize(int n, std::shared_ptr<ResolvedType> type) {
    for (int i = 0; i < n; i++) {
      auto type = shadow.top();
      shadow.pop();
    }
    shadow.push(type);
  }

  void add_lvalue(LValue* lvalue) {
    auto base = size - 8;  // rbp is 8 below the size of our stack so we subract 8 to get the offset from rbp
    variables[lvalue->identifier] = base;
    if (auto array_lvalue = dynamic_cast<ArrayLValue*>(lvalue)) {
      for (const auto& name : array_lvalue->indices) {
        variables[name] = base;
        base -= 8;
      }
    }
  }
};

struct StackArg {
  // int size;
  int offset;
  std::shared_ptr<ResolvedType> type;
};

class CallingConvention {
 public:
  inline static const std::vector<std::string> all_int_regs = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  inline static const std::vector<std::string> all_float_regs = {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8"};

  std::vector<std::variant<std::string, int>> args{};
  std::variant<std::string, int> ret;
  std::vector<std::string> int_regs{};
  std::vector<std::string> float_regs{};
  std::vector<StackArg> stack_args{};
  std::variant<std::string, StackArg> return_position;
  int total_stack = 0;

  CallingConvention(const FnInfo& fn) : fn(fn) {
    auto int_count = 0, float_count = 0, offset = 0;

    // params
    for (const auto& type : fn.param_types) {
      if (type->is<Int>()) {
        if (int_count < all_int_regs.size()) {
          args.push_back(all_int_regs[int_count++]);
          int_regs.push_back(all_int_regs[int_count++]);
        } else {
          args.push_back(offset);
          stack_args.push_back({offset, type});
          offset += type->size();
          total_stack += type->size();
        }
      } else if (type->is<Float>()) {
        if (float_count < all_float_regs.size()) {
          args.push_back(all_float_regs[float_count++]);
          float_regs.push_back(all_float_regs[float_count++]);
        } else {
          args.push_back(offset);
          stack_args.push_back({offset, type});
          offset += type->size();
          total_stack += type->size();
        }
      }
    }

    // return
    if (fn.return_type->is<Int>() || fn.return_type->is<Bool>()) {
      ret = "rax";
      return_position = "rax";
    } else if (fn.return_type->is<Float>()) {
      ret = "xmm0";
      return_position = "xmm0";
    } else if (fn.return_type->is<Array>()) {
      ret = 0;                       // also idk
      return_position = StackArg();  // idk
    }
  }

 private:
  const FnInfo& fn;
};

class ASMGenVisitor : public ASTVisitor {
 public:
  ASMGenVisitor(std::shared_ptr<Context> ctx, Logger& logger) : ctx(ctx), logger(logger), data_visitor(ctx), fn_visitor(*this) {
  }

  void align(int size) {
    int padding = stack.align(size);
    if (padding) {
      print("sub rsp, ", padding, " ; add padding for alignment");
    }
  }

  void unalign() {
    int padding = stack.unalign();
    if (padding) {
      print("add rsp, ", padding, " ; remove padding for alignment");
    }
  }

  void push(std::string reg, std::shared_ptr<ResolvedType> type, std::string comment = "") {
    print("; pushing ", type->to_string(), " to stack");
    if (type->is<Float>()) {
      print("sub rsp, 8");
      print("movsd [rsp], ", reg);
    } else {
      if (comment.empty()) {
        print("push ", reg);
      } else {
        print("push ", reg, " ; ", comment);
      }
    }
    stack.push(type);
  }

  void pop(std::string reg, std::string comment = "") {
    auto type = stack.pop();
    if (type->is<Float>()) {
      print("movsd ", reg, ", [rsp]");
      print("add rsp, 8");
    } else {
      if (comment.empty()) {
        print("pop ", reg);
      } else {
        print("pop ", reg, " ; ", comment);
      }
    }
  }

  void push_const(asmval val, std::shared_ptr<ResolvedType> type) {
    print("; pushing const ", type->to_string(), " to stack");
    stack.push(type);
    print("mov rax, [rel ", const_map[val], "]");
    print("push rax");
  }

  void read_const(std::string reg, asmval val) {
    print("lea ", reg, ", [rel ", const_map[val], "]");
  }

  virtual void visit(const Program& program) override {
    std::cout << header;
    data_visitor.visit(program);
    const_map = data_visitor.const_map;
    fn_visitor.visit(program);
    std::cout << "jpl_main:\n_jpl_main:\n";
    // print("push rbp");
    push("rbp", Int::shared);
    print("mov rbp, rsp");
    // print("push r12");
    push("r12", Int::shared);
    print("mov r12, rbp");
    stack.size = 16;
    print("; === END OF PRELUDE ===\n");
    ASTVisitor::visit(program);
    if (stack.local_var_size) {
      print("add rsp, ", stack.local_var_size, " ; local vars");
    }
    print("\n    ; === START OF POSTLUDE ===");
    // pop("r12");
    // pop("rbp");
    print("pop r12");
    print("pop rbp");
    print("ret");
  }

  virtual void visit(const FnCmd& fn) override {}

  void fn(const FnCmd& fn) {
    auto calling_convention = CallingConvention(*ctx->lookup<FnInfo>(fn.identifier));
    auto ret_reg = std::get_if<int>(&calling_convention.ret);
    auto prev_stack = stack;
    stack = Stack();

    std::cout << fn.identifier << ":\n";
    std::cout << "_" << fn.identifier << ":\n";
    push("rbp", Int::shared);
    print("mov rbp, rsp");
    print("; === END OF PRELUDE ===\n");

    if (ret_reg) {
      push("rdi", Int::shared);
      stack.variables["$return"] = stack.size - 8;
    }

    ASTVisitor::visit(fn);

    print("add rsp, ", stack.size - 8, " ; local variables");
    pop("rbp");
    print("ret");
    std::cout << "\n";

    stack = prev_stack;
  }

  virtual void visit(const ReturnStmt& stmt) override {
    ASTVisitor::visit(stmt);
    auto type = stmt.expr->type;
    if (type->is<Int>() || type->is<Bool>()) {
      pop("rax");
    } else if (type->is<Float>()) {
      pop("xmm0");
    } else {
      auto offset = stack.variables["$return"];
      print("mov rax, [rbp - ", offset, "]");
      // copy data from rsp to rax
      for (int i = type->size() - 8; i >= 0; i -= 8) {
        print("mov r10, [rsp + ", i, "]");
        print("mov [rax + ", i, "], r10");
      }
    }
  }

  virtual void visit(const IntExpr& expr) override {
    push_const(expr.value, Int::shared);
  }

  virtual void visit(const FloatExpr& expr) override {
    push_const(expr.value, Float::shared);
  }

  virtual void visit(const TrueExpr& expr) override {
    push_const(true, Bool::shared);
  }

  virtual void visit(const FalseExpr& expr) override {
    push_const(false, Bool::shared);
  }

  virtual void visit(const UnopExpr& expr) override {
    ASTVisitor::visit(expr);
    auto type = stack.top();
    print("; unop on ", type->to_string());
    if (type->is<Bool>()) {
      pop("rax");
      print("xor rax, 1");
      push("rax", type);
    } else if (type->is<Int>()) {
      pop("rax");
      print("neg rax");
      push("rax", type);
    } else if (type->is<Float>()) {
      pop("xmm1");
      print("pxor xmm0, xmm0");
      print("subsd xmm0, xmm1");
      push("xmm0", type);
    }
  }

  void int_binop(const BinopExpr& expr) {
    std::map<std::string, std::string> bool_ops = {{"<", "setl"}, {">", "setg"}, {"<=", "setle"}, {">=", "setge"}, {"==", "sete"}, {"!=", "setne"}};
    ASTVisitor::visit(expr);
    pop("rax");
    pop("r10");
    if (bool_ops.find(expr.op) != bool_ops.end()) {
      print("cmp rax, r10");
      print(bool_ops[expr.op], " al");
      print("and rax, 1");
    } else if (expr.op == "+") {
      print("add rax, r10");
    } else if (expr.op == "-") {
      print("sub rax, r10");
    } else if (expr.op == "*") {
      print("imul rax, r10");
    } else if (expr.op == "/") {
      print("cmp r10, 0");
      print("; begin assert call");
      auto label = genlabel();
      print("jne ", label);
      align(8);  // idk
      read_const("rdi", "divide by zero");
      print("call _fail_assertion");
      unalign();
      print(label, ":");
      print("; end assert call");
      print("cqo");
      print("idiv r10");
    } else if (expr.op == "%") {
      print("cmp r10, 0");
      print("; begin assert call");
      auto label = genlabel();
      print("jne ", label);
      align(8);  // idk
      read_const("rdi", "mod by zero");
      print("call _fail_assertion");
      unalign();
      print(label, ":");
      print("; end assert call");
      print("cqo");
      print("idiv r10");
      print("mov rax, rdx");
    }
    push("rax", expr.type);
  }

  void float_binop(const BinopExpr& expr) {
    if (expr.op == "%") {
      align(8);
    }
    ASTVisitor::visit(expr);
    pop("xmm0");
    pop("xmm1");
    if (expr.op == "+") {
      print("addsd xmm0, xmm1");
    } else if (expr.op == "-") {
      print("subsd xmm0, xmm1");
    } else if (expr.op == "*") {
      print("mulsd xmm0, xmm1");
    } else if (expr.op == "/") {
      print("divsd xmm0, xmm1");
    } else if (expr.op == "%") {
      print("call _fmod");
      unalign();
    } else {  // boolean ops
      auto reg1 = "xmm0", reg2 = "xmm1";
      if (expr.op == ">" || expr.op == ">=") {
        reg1 = "xmm1", reg2 = "xmm0";
      }
      auto cmd = "";
      if (expr.op == "<" || expr.op == ">") {
        cmd = "cmpltsd";
      } else if (expr.op == "<=" || expr.op == ">=") {
        cmd = "cmplesd";
      } else if (expr.op == "==") {
        cmd = "cmpeqsd";
      } else if (expr.op == "!=") {
        cmd = "cmpneqsd";
      }
      print(cmd, " ", reg1, ", ", reg2);
      print("movq rax, ", reg1);
      print("and rax, 1");
      push("rax", Bool::shared);
      return;
    }
    push("xmm0", Float::shared);
  }

  virtual void visit(const BinopExpr& expr) override {
    print("; binop ", expr.type->to_string());
    if (expr.left->type->is<Float>()) {
      float_binop(expr);
    } else {
      int_binop(expr);
    }
  }

  virtual void visit(const ArrayLiteralExpr& expr) override {
    print("; in reverse order, generate code for EXPRs");
    ASTVisitor::visit(expr);
    auto element_size = expr.type->as<Array>()->element_type->size();
    auto size = expr.elements.size() * element_size;
    print("mov rdi, ", size);
    align(8);
    print("call _jpl_alloc");
    unalign();
    print("; copy data from rsp to rax");
    for (int i = size - 8; i >= 0; i -= 8) {
      print("mov r10, [rsp + ", i, "]");
      print("mov [rax + ", i, "], r10");
    }
    print("; free EXPRs from stack");
    print("add rsp, ", element_size * expr.elements.size());
    for (const auto& _ : expr.elements) {
      stack.pop();
    }
    push("rax", Int::shared);
    print("mov rax, ", expr.elements.size());
    push("rax", Int::shared);
    stack.recharacterize(2, expr.type);
  }

  virtual void visit(const LetCmd& cmd) override {
    ASTVisitor::visit(cmd);
    stack.local_var_size += cmd.expr->type->size();
    stack.add_lvalue(cmd.lvalue.get());
  }

  virtual void visit(const LetStmt& cmd) override {
    ASTVisitor::visit(cmd);
    stack.local_var_size += cmd.expr->type->size();
    stack.add_lvalue(cmd.lvalue.get());
  }

  virtual void visit(const VarExpr& expr) override {
    auto start = stack.variables[expr.identifier];
    // allocate type on stack
    stack.shadow.push(expr.type);
    stack.size += expr.type->size();
    print("sub rsp, ", expr.type->size(), " ; make space to copy var expr");

    // copy data to rsp from rsp - [position on stack]
    for (int i = expr.type->size() - 8; i >= 0; i -= 8) {
      print("mov r10, [rbp - ", start, " + ", i, "]");
      print("mov [rsp + ", i, "], r10");
    }
  }

  virtual void visit(const CallExpr& expr) override {
    auto info = ctx->lookup<FnInfo>(expr.identifier);

    // make calling convention
    auto convention = CallingConvention(*info);
    auto ret_reg = std::get_if<std::string>(&convention.ret);

    // prepare stack
    if (!ret_reg) {
      print("sub rsp, ", info->return_type->size());
      auto ret_offset = std::get<int>(convention.ret);
      auto offset = stack.size - ret_offset + stack.padding.top();
      print("lea rdi, [rsp + ", offset, "]");
    }

    // generate code for args

    // do call
    if (ret_reg) {
      align(8);
    } else {
      align(info->return_type->size() + 8);
      stack.shadow.push(info->return_type);
    }
    print("call _", info->name);

    // free stack args one-by-one

    // unalign stack
    unalign();

    // if return val is in a register, push to a stack
    if (ret_reg) {
      push(*ret_reg, info->return_type);
    }
  }

  virtual void visit(const ShowCmd& cmd) override {
    auto type = cmd.expr->type;
    align(type->size() + 8);  // call pushes return address
    ASTVisitor::visit(cmd);
    read_const("rdi", type->show_type(ctx.get()));
    print("lea rsi, [rsp]");
    print("call _show");
    // free self.stack by sizeof EXPR_TYPE
    print("add rsp, ", type->size(), " ; free stack by sizeof expr");
    // stack.size -= type->size();
    stack.pop();
    unalign();
  }

 private:
  int jump_ctr = 0;
  const std::shared_ptr<Context> ctx;
  const Logger& logger;
  ASMDataVisitor data_visitor;
  ASMFnVisitor fn_visitor;
  std::map<asmval, std::string> const_map;
  Stack stack;

  std::string genlabel() {
    return ".jump" + std::to_string(++jump_ctr);
  }

  template <typename... Args>
  void print(Args&&... args) {
    std::cout << "    ";
    (std::cout << ... << args);
    std::cout << '\n';
  }

  std::string header =
      "global jpl_main\n"
      "global _jpl_main\n"
      "extern _fail_assertion\n"
      "extern _jpl_alloc\n"
      "extern _get_time\n"
      "extern _show\n"
      "extern _print\n"
      "extern _print_time\n"
      "extern _read_image\n"
      "extern _write_image\n"
      "extern _fmod\n"
      "extern _sqrt\n"
      "extern _exp\n"
      "extern _sin\n"
      "extern _cos\n"
      "extern _tan\n"
      "extern _asin\n"
      "extern _acos\n"
      "extern _atan\n"
      "extern _log\n"
      "extern _pow\n"
      "extern _atan2\n"
      "extern _to_int\n"
      "extern _to_float\n\n";
};
