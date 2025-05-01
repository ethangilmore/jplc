#pragma once

#include <math.h>

#include <cassert>
#include <climits>
#include <map>
#include <optional>
#include <stack>
#include <variant>

#include "asmdatavisitor.h"
#include "asmfnvisitor.h"
#include "astnodes.h"
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

  Stack(Context* ctx) : ctx(ctx) {}

  std::shared_ptr<ResolvedType> top() {
    return *shadow.top();
  }

  int push(std::shared_ptr<ResolvedType> type) {
    auto s = type->size(ctx);
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
    size -= type->size(ctx);
    shadow.pop();
    // std::cout << "pop: stack now has " << shadow.size() << " things on it\n";
    return type;
  }

  std::shared_ptr<ResolvedType> pop(std::shared_ptr<ResolvedType> expected) {
    auto type = pop();
    if (*type != *expected) {
      throw std::runtime_error("Expected to pop " + expected->to_string() + ", but got " + type->to_string());
    }
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
        std::cout << "; uh oh we're trying to unalign but theres data instead of padding\n";
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

  void add_lvalue(LValue* lvalue, int offset) {
    auto base = offset;
    variables[lvalue->identifier] = base;
    if (auto array_lvalue = dynamic_cast<ArrayLValue*>(lvalue)) {
      for (const auto& name : array_lvalue->indices) {
        variables[name] = base;
        base -= 8;
      }
    }
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

  void add_lvalue(std::string identifier) {
    auto base = size - 8;  // rbp is 8 below the size of our stack so we subract 8 to get the offset from rbp
    variables[identifier] = base;
  }

 private:
  Context* ctx;
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

  CallingConvention(const FnInfo& fn, Context* ctx) : fn(fn) {
    auto int_count = 0, float_count = 0, offset = 0;

    // params
    for (const auto& type : fn.param_types) {
      if (type->is<Int>() || type->is<Bool>()) {
        if (int_count < all_int_regs.size()) {
          args.push_back(all_int_regs[int_count]);
          int_regs.push_back(all_int_regs[int_count++]);
        } else {
          args.push_back(offset);
          stack_args.push_back({offset, type});
          offset += type->size(ctx);
          total_stack += type->size(ctx);
        }
      } else if (type->is<Float>()) {
        if (float_count < all_float_regs.size()) {
          args.push_back(all_float_regs[float_count]);
          float_regs.push_back(all_float_regs[float_count++]);
        } else {
          args.push_back(offset);
          stack_args.push_back({offset, type});
          offset += type->size(ctx);
          total_stack += type->size(ctx);
        }
      } else if (type->is<Array>()) {
        args.push_back(offset);
        stack_args.push_back({offset, type});
        offset += type->size(ctx);
        total_stack += type->size(ctx);
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
  ASMGenVisitor(std::shared_ptr<Context> ctx, Logger& logger, int opt) : ctx(ctx), stack(ctx.get()), logger(logger), data_visitor(ctx, opt), fn_visitor(*this), opt(opt) {
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
    if (auto int_const = std::get_if<int64_t>(&val)) {
      // if (opt > 0 && (*int_const & (1l << 31) - 1) == *int_const) {
      if (opt > 0 && *int_const >= INT32_MIN && *int_const <= INT32_MAX) {
        print("push qword ", *int_const);
        return;
      }
    }
    print("mov rax, [rel ", const_map[val], "]");
    print("push rax");
  }

  void read_const(std::string reg, asmval val) {
    print("lea ", reg, ", [rel ", const_map[val], "]");
  }

  void copy(int size, std::string from, std::string to) {
    for (int i = size - 8; i >= 0; i -= 8) {
      print("mov r10, [", from, " + ", i, "]");
      print("mov [", to, " + ", i, "], r10");
    }
  }

  void asm_alloc(std::shared_ptr<ResolvedType> type) {
    print("sub rsp, ", type->size(ctx.get()));
    stack.push(type);
  }

  void asm_free(std::shared_ptr<ResolvedType> type) {
    print("add rsp, ", type->size(ctx.get()));
    stack.pop(type);
  }

  void asm_free(int n, std::shared_ptr<ResolvedType> type) {
    print("add rsp, ", n * type->size(ctx.get()));
    for (int i = 0; i < n; i++) {
      stack.pop(type);
    }
  }

  void asm_assert(std::string cmd, std::string msg) {
    print("; begin assert call for '", msg, "'");
    auto label = genlabel();
    print(cmd, " ", label);
    align(8);
    read_const("rdi", msg);
    print("call _fail_assertion");
    unalign();
    print(label, ":");
    print("; end assert call");
  }

  virtual void visit(const Program& program) override {
    stack.variables["argnum"] = -16;
    stack.variables["args"] = -16;
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
    // stack.size = 16;
    print("; === END OF PRELUDE ===\n");
    ASTVisitor::visit(program);
    print("; local var size ", stack.local_var_size);
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
    auto calling_convention = CallingConvention(*ctx->lookup<FnInfo>(fn.identifier), ctx.get());
    auto ret_reg = std::get_if<int>(&calling_convention.ret);
    auto prev_stack = stack;
    stack = Stack(ctx.get());

    // make a new function
    std::cout << fn.identifier << ":\n";
    std::cout << "_" << fn.identifier << ":\n";

    // save and udpate rbp
    push("rbp", Int::shared);
    print("mov rbp, rsp");
    print("; === END OF PRELUDE ===\n");

    // if return val goes on stack
    std::cout << "; ret reg " << ret_reg << '\n';
    std::cout << "; doing return val\n";
    if (ret_reg) {
      push("rdi", Int::shared);
      stack.variables["$return"] = stack.size - 8;
    }

    // recieve args
    std::cout << "; recieve args\n";
    for (int i = 0; i < fn.params.size(); i++) {
      auto identifier = fn.params[i]->lvalue->identifier;
      print("; identifier ", identifier);
      auto type = fn.params[i]->type->type;
      print("; type ", type->to_string());
      auto position = calling_convention.args[i];
      if (auto reg = std::get_if<std::string>(&position)) {
        print("; position ", reg);
      } else if (auto pos = std::get_if<int>(&position)) {
        print("; position ", pos);
      }
      if (auto arg_offset = std::get_if<int>(&position)) {
        // stack arg
        // TODO: this doesn't work
        auto ret_size = fn.return_type->type->size(ctx.get());
        auto offset = stack.size - *arg_offset + 16 + ret_size;
        stack.add_lvalue(fn.params[i]->lvalue.get(), -offset);
      } else if (auto reg = std::get_if<std::string>(&position)) {
        // register
        push(*reg, type);
        stack.add_lvalue(fn.params[i]->lvalue.get());
      }
    }

    // process stmts
    std::cout << "; process stmts\n";
    ASTVisitor::visit(fn);

    // add implicit return, if needed

    // this is some return stuff, idk if it should go here
    print("add rsp, ", stack.size - 8, " ; local variables");
    // pop("rbp");
    stack.pop();
    print("pop rbp");
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
      for (int i = type->size(ctx.get()) - 8; i >= 0; i -= 8) {
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
    if (expr.op == "&&" || expr.op == "||") {
      expr.left->accept(*this);
      pop("rax");
      print("cmp rax, 0");
      auto label = genlabel();
      auto cmd = expr.op == "&&" ? "je" : "jne";
      print(cmd, " ", label);
      expr.right->accept(*this);
      pop("rax");
      print(label, ":");
      push("rax", Bool::shared);
      return;
    }

    std::map<std::string, std::string> bool_ops = {{"<", "setl"}, {">", "setg"}, {"<=", "setle"}, {">=", "setge"}, {"==", "sete"}, {"!=", "setne"}};
    auto left_int_const = dynamic_cast<const IntExpr*>(expr.left.get());
    auto right_int_const = dynamic_cast<const IntExpr*>(expr.right.get());
    auto left_shl_opt = opt > 0 && expr.op == "*" && left_int_const && log_2(left_int_const->value) >= 0;
    auto right_shl_opt = opt > 0 && expr.op == "*" && right_int_const && log_2(right_int_const->value) >= 0;

    if (left_shl_opt) {
      print("; optimizing left constant into shift");
      expr.right->accept(*this);
      if (log_2(left_int_const->value) > 0) {
        pop("rax");
        print("shl rax, ", log_2(left_int_const->value));
        push("rax", expr.right->type);
      }
      return;
    } else if (right_shl_opt) {
      print("; optimizing right constant into shift");
      expr.left->accept(*this);
      if (log_2(right_int_const->value) > 0) {
        pop("rax");
        print("shl rax, ", log_2(right_int_const->value));
        push("rax", expr.left->type);
      }
      return;
    } else {
      ASTVisitor::visit(expr);
      pop("rax");
      pop("r10");
    }

    if (bool_ops.find(expr.op) != bool_ops.end()) {
      print("cmp rax, r10");
      print(bool_ops[expr.op], " al");
      print("and rax, 1");
    } else if (expr.op == "+") {
      print("add rax, r10");
    } else if (expr.op == "-") {
      print("sub rax, r10");
    } else if (expr.op == "*") {
      if (left_shl_opt) {
        print("shl rax, ", log_2(left_int_const->value));
      } else {
        print("imul rax, r10");
      }
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
    auto element_size = expr.type->as<Array>()->element_type->size(ctx.get());
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
    stack.local_var_size += cmd.expr->type->size(ctx.get());
    stack.add_lvalue(cmd.lvalue.get());
  }

  virtual void visit(const LetStmt& cmd) override {
    ASTVisitor::visit(cmd);
    stack.local_var_size += cmd.expr->type->size(ctx.get());
    stack.add_lvalue(cmd.lvalue.get());
  }

  virtual void visit(const VarExpr& expr) override {
    auto start = stack.variables[expr.identifier];
    // allocate type on stack
    stack.shadow.push(expr.type);
    stack.size += expr.type->size(ctx.get());
    print("sub rsp, ", expr.type->size(ctx.get()), " ; make space to copy var expr");

    // copy data to rsp from rsp - [position on stack]
    for (int i = expr.type->size(ctx.get()) - 8; i >= 0; i -= 8) {
      print("mov r10, [rbp - ", start, " + ", i, "]");
      print("mov [rsp + ", i, "], r10");
    }
  }

  virtual void visit(const CallExpr& expr) override {
    std::cout << "; stack size is " << stack.size << '\n';
    auto info = ctx->lookup<FnInfo>(expr.identifier);
    // print("; calling convention for ", info->name);
    // for (auto arg : info->param_types) {
    //   print("; ", arg->to_string());
    // }

    // make calling convention
    auto convention = CallingConvention(*info, ctx.get());
    auto ret_reg = std::get_if<std::string>(&convention.ret);

    // if (!ret_reg) {  // return val goes on stack, alloc space
    //   asm_alloc(info->return_type);
    // } else {  // else add return type to stack shadow
    //   stack.push(info->return_type);
    // }
    // align(stack.size - info->return_type->size(ctx.get()));  // either way, align by: (total_satck - return_val_sapce)

    // prepare stack
    if (!ret_reg) {
      asm_alloc(info->return_type);
      auto ret_offset = std::get<int>(convention.ret);
      print("lea rdi, [rsp + 0]");  //, offset, "]");
      // auto offset = stack.size - ret_offset + stack.padding.top();
      // print("lea rdi, [rsp + ", offset, "]");
    } else {
      stack.shadow.push(info->return_type);
    }
    std::cout << "; stack size is " << stack.size << '\n';
    align(stack.size - info->return_type->size(ctx.get()));

    // generate code for args
    for (int i = expr.args.size() - 1; i >= 0; i--) {
      // stack args right to left
      if (std::get_if<int>(&convention.args[i])) {
        print("; generating expr for stack arg");
        expr.args[i]->accept(*this);
        stack.size += expr.args[i]->type->size(ctx.get());
      }
    }
    for (int i = expr.args.size() - 1; i >= 0; i--) {
      // register args right to left
      print("; generating expr for register arg");
      if (std::get_if<std::string>(&convention.args[i])) {
        expr.args[i]->accept(*this);
      }
    }
    for (const auto& arg : convention.args) {
      // pop register args to their registers
      print("; popping register arg to register");
      if (auto reg = std::get_if<std::string>(&arg)) {
        pop(*reg);
      }
    }

    // do call
    // auto ret_offset = std::get<int>(convention.ret);
    // auto offset = stack.size - ret_offset + stack.padding.top();
    // print("lea rdi, [rsp + ", offset, "]");
    print("call _", info->name);

    // free stack args one-by-one
    for (auto arg : convention.stack_args) {
      asm_free(arg.type);
    }

    // unalign stack
    unalign();

    // if return val is in a register, push to a stack
    if (ret_reg) {
      push(*ret_reg, info->return_type);
    }
  }

  virtual void visit(const IfExpr& expr) override {
    expr.condition->accept(*this);
    if (opt > 0) {
      auto l = dynamic_cast<const IntExpr*>(expr.if_expr.get());
      auto r = dynamic_cast<const IntExpr*>(expr.else_expr.get());
      if (l && r && l->value == 1 && r->value == 0) return;
    }
    pop("rax");
    print("cmp rax, 0");
    auto else_label = genlabel();
    auto end_label = genlabel();
    print("je ", else_label);
    expr.if_expr->accept(*this);
    stack.pop();
    print("jmp ", end_label);
    print(else_label, ":");
    expr.else_expr->accept(*this);
    print(end_label, ":");
  }

  virtual void visit(const ArrayIndexExpr& expr) override {
    print();
    print("; begin array index expr");
    auto type = expr.expr->type->as<Array>();

    auto gap = 0;
    auto var_expr = dynamic_cast<const VarExpr*>(expr.expr.get());
    if (opt > 0 && var_expr) {
      auto offset = stack.variables[var_expr->identifier];
      gap = stack.size - offset + type->rank * 8 - 8;
    } else {
      expr.expr->accept(*this);
      gap = type->rank * 8;
    }

    // generate code for IDX, reversed
    for (int i = expr.indices.size() - 1; i >= 0; i--) {
      expr.indices[i]->accept(*this);
    }

    // for each IDX_K
    for (int k = 0; k < type->rank; k++) {
      print("mov rax, [rsp + ", k * 8, "] ; here");
      print("cmp rax, 0");
      asm_assert("jge", "negative array index");
      print("cmp rax, [rsp + ", k * 8 + gap, "] ; here");
      asm_assert("jl", "index too large");
    }

    // genereate indexing code
    auto offset = 0;
    if (opt == 0) {
      print("mov rax, 0");
    } else {
      print("mov rax, [rsp + ", offset, "]");
    }
    for (int i = opt > 0; i < expr.indices.size(); i++) {
      print("imul rax, [rsp + ", offset + i * 8 + gap, "]");
      print("add rax, [rsp + ", offset + i * 8, "]");
    }
    auto element_size = type->element_type->size(ctx.get());
    print("; element type ", type->element_type->to_string());
    print("; element size ", element_size);
    if (opt > 0 && log_2(element_size) > 0) {
      print("shl rax, ", log_2(element_size));
    } else {
      print("imul rax, ", element_size);
    }

    print("add rax, [rsp + ", offset + expr.indices.size() * 8 + gap, "]");

    // stack cleanup stuff
    if (opt > 0) {
      for (const auto& index : expr.indices) {
        // asm_free(index->type);
        stack.pop(index->type);  // free
      }
      print("add rsp, ", expr.indices.size() * 8);
    } else {
      for (const auto& index : expr.indices) {
        asm_free(index->type);
        // stack.pop(index->type);  // free
      }
    }
    if (!(opt > 0 && var_expr)) {
      asm_free(expr.expr->type);
    }
    // stack.pop(expr.expr->type);  // free
    asm_alloc(type->element_type);
    copy(type->element_type->size(ctx.get()), "rax", "rsp");
    print("; stack.alloc(ELEM_TYPE)");
  }

  virtual void visit(const SumLoopExpr& expr) override {
    print();
    print("; begin sum loop expr");

    // 1/4
    auto num_e = expr.axis.size();
    asm_alloc(expr.type);
    for (int i = num_e - 1; i >= 0; i--) {
      expr.axis[i].second->accept(*this);
      // check bounds step-by-step
      print("mov rax, [rsp]");
      print("cmp rax, 0");
      asm_assert("jg", "non-positive loop bound");
    }
    print("mov rax, 0 ; init sum");
    print("mov [rsp + ", num_e * 8, "], rax ; move to pre-alloc");
    for (int i = num_e - 1; i >= 0; i--) {
      const auto& [identifier, e] = expr.axis[i];
      print("mov rax, 0");
      push("rax", Int::shared);
      stack.add_lvalue(identifier);
    }

    // 2/4
    auto jump_label = genlabel();
    print(jump_label, ":");
    expr.expr->accept(*this);
    if (expr.expr->type->is<Int>()) {
      pop("rax");
      print("add [rsp + ", 2 * num_e * 8, "], rax");
    } else {
      pop("xmm0");
      print("addsd xmm0, [rsp + ", 2 * num_e * 8, "]");
      print("movsd [rsp + ", 2 * num_e * 8, "], xmm0");
    }

    // 3/4
    print("add qword [rsp + ", (num_e - 1) * 8, "], 1");
    for (int i = num_e - 1; i >= 0; i--) {
      print("mov rax, [rsp + ", i * 8, "]");
      print("cmp rax, [rsp + ", (i + num_e) * 8, "]");
      print("jl ", jump_label);
      // if something
      if (i > 0) {
        print("mov qword [rsp + ", i * 8, "], 0");
        print("add qword [rsp + ", (i - 1) * 8, "], 1");
      }
    }

    // 4/4
    asm_free(num_e, Int::shared);
    asm_free(num_e, Int::shared);
  }

  virtual void visit(const ShowCmd& cmd) override {
    auto type = cmd.expr->type;
    align(type->size(ctx.get()) + 8);  // call pushes return address
    ASTVisitor::visit(cmd);
    read_const("rdi", type->show_type(ctx.get()));
    print("lea rsi, [rsp]");
    print("call _show");
    // free self.stack by sizeof EXPR_TYPE
    print("add rsp, ", type->size(ctx.get()), " ; free stack by sizeof expr");
    // stack.size -= type->size();
    stack.pop();
    unalign();
  }

  virtual void visit(const ArrayLoopExpr& expr) override {
    print();
    print("; begin array loop expr");

    // 1/4
    auto num_e = expr.axis.size();
    asm_alloc(Int::shared);
    for (int i = num_e - 1; i >= 0; i--) {
      expr.axis[i].second->accept(*this);
      // check bounds step-by-step
      print("mov rax, [rsp]");
      print("cmp rax, 0");
      asm_assert("jg", "non-positive loop bound");
    }
    print("mov rdi, ", expr.expr->type->size(ctx.get()));
    for (int i = 0; i < num_e; i++) {
      print("imul rdi, [rsp + ", i * 8, "]");
      asm_assert("jno", "overflow computing array size");
    }
    align(8);
    print("call _jpl_alloc");
    unalign();
    print("mov [rsp + ", num_e * 8, "], rax ; move to pre-alloc");
    for (int i = num_e - 1; i >= 0; i--) {
      const auto& [identifier, e] = expr.axis[i];
      print("mov rax, 0");
      push("rax", Int::shared);
      stack.add_lvalue(identifier);
    }

    // 2/4
    auto jump_label = genlabel();
    print(jump_label, ":");
    expr.expr->accept(*this);
    auto offset = expr.expr->type->size(ctx.get());

    // generate index for result
    if (opt == 0) {
      print("mov rax, 0");
    } else {
      print("mov rax, [rsp + ", offset, "]");
    }
    for (int i = opt > 0; i < num_e; i++) {
      auto int_expr = dynamic_cast<const IntExpr*>(expr.axis[i].second.get());
      if (opt > 0 && int_expr) {
        if (log_2(int_expr->value) >= 0) {
          print("shl rax, ", log_2(int_expr->value));
        } else {
          print("imul rax, ", int_expr->value, ";here??1");
        }
      } else {
        print("imul rax, [rsp + ", offset + (num_e + i) * 8, "]");
      }
      print("add rax, [rsp + ", offset + i * 8, "]");
    }
    if (log_2(offset) >= 0 && opt > 0) {
      print("shl rax, ", log_2(offset));
    } else {
      print("imul rax, ", offset, ";here??2");
    }
    print("add rax, [rsp + ", offset + 2 * num_e * 8, "]");

    copy(offset, "rsp", "rax");
    asm_free(expr.expr->type);

    // 3/4
    print("add qword [rsp + ", (num_e - 1) * 8, "], 1");
    for (int i = num_e - 1; i >= 0; i--) {
      print("mov rax, [rsp + ", i * 8, "]");
      print("cmp rax, [rsp + ", (i + num_e) * 8, "]");
      print("jl ", jump_label);
      // if something
      if (i > 0) {
        print("mov qword [rsp + ", i * 8, "], 0");
        print("add qword [rsp + ", (i - 1) * 8, "], 1");
      }
    }

    // 4/4
    asm_free(num_e, Int::shared);
    stack.recharacterize(num_e + 1, expr.type);
  }

  virtual void visit(const AssertCmd& cmd) override {
    ASTVisitor::visit(cmd);
    pop("rax");
    print("cmp rax, 0");
    asm_assert("jne", cmd.string);
  }

  virtual void visit(const AssertStmt& stmt) override {
    ASTVisitor::visit(stmt);
    pop("rax");
    print("cmp rax, 0");
    asm_assert("jne", stmt.string);
  }

  virtual void visit(const ReadCmd& cmd) override {
    auto rgba = std::make_shared<Struct>("rgba");
    auto type = std::make_shared<Array>(rgba, 2);
    print("; rgba size ", type->size(ctx.get()));
    asm_alloc(type);
    print("lea rdi, [rsp]");
    align(8);
    read_const("rsi", cmd.stripped_string());
    print("call _read_image");
    unalign();
    stack.add_lvalue(cmd.lvalue.get());
    stack.local_var_size += 24;
  }

  virtual void visit(const WriteCmd& cmd) override {
    auto rgba = std::make_shared<Struct>("rgba");
    auto type = std::make_shared<Array>(rgba, 2);
    stack.align(type->size(ctx.get()));
    cmd.expr->accept(*this);
    ASTVisitor::visit(cmd);
    read_const("rdi", cmd.stripped_string());
    print("call _write_image");
    asm_free(cmd.expr->type);
    stack.unalign();
  }

  virtual void visit(const PrintCmd& cmd) override {
    read_const("rdi", cmd.string);
    stack.align(8);
    print("call _print");
    stack.unalign();
  }

  virtual void visit(const VoidExpr& expr) override {
    push_const(1, Void::shared);
  }

  virtual void visit(const StructLiteralExpr& expr) override {
    ASTVisitor::visit(expr);
    stack.recharacterize(expr.fields.size(), expr.type);
  }

  virtual void visit(const DotExpr& expr) override {
    ASTVisitor::visit(expr);
    auto start = 0;  // TODO: calc offset
    // for (auto field : ctx->lookup<StructInfo>(expr.symbol)->fields) {
    //   if (field.first == expr.field) break;
    //   start += field.second->size(ctx.get());
    // }
    auto size = expr.type->size(ctx.get());
    auto end = expr.expr->type->size(ctx.get()) - size;
    copy(size, "rsp + " + start, "rsp + " + end);
    print("add rsp, ", end);
    stack.recharacterize(1, expr.type);
    // stack.decrement(end);
    // asm_free(end);
  }

 private:
  int jump_ctr = 0;
  int opt = 0;
  const std::shared_ptr<Context> ctx;
  const Logger& logger;
  ASMDataVisitor data_visitor;
  ASMFnVisitor fn_visitor;
  std::map<asmval, std::string> const_map;
  Stack stack;

  std::string genlabel() {
    return ".jump" + std::to_string(++jump_ctr);
  }

  int log_2(int64_t x) {
    if (x > 0 && (x & (x - 1)) == 0) {
      return (int64_t)log2(x);
    }
    return -1;
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
