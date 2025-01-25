#include "astnodes.h"
#include "astvisitor.h"

Program::Program(std::vector<std::unique_ptr<Cmd>> cmds)
    : cmds(std::move(cmds)) {}

void Program::accept(ASTVisitor &visitor) { visitor.visit(*this); }

ReadCmd::ReadCmd(std::string string, std::unique_ptr<VarLValue> lvalue)
    : string(std::move(string)), lvalue(std::move(lvalue)) {}

void ReadCmd::accept(ASTVisitor &visitor) { visitor.visit(*this); }

WriteCmd::WriteCmd(std::unique_ptr<Expr> expr, std::string string)
    : expr(std::move(expr)), string(std::move(string)) {}

void WriteCmd::accept(ASTVisitor &visitor) { visitor.visit(*this); }

LetCmd::LetCmd(std::unique_ptr<LValue> lvalue, std::unique_ptr<Expr> expr)
    : lvalue(std::move(lvalue)), expr(std::move(expr)) {}

void LetCmd::accept(ASTVisitor &visitor) { visitor.visit(*this); }

AssertCmd::AssertCmd(std::unique_ptr<Expr> expr, std::string string)
    : expr(std::move(expr)), string(std::move(string)) {}

void AssertCmd::accept(ASTVisitor &visitor) { visitor.visit(*this); }

PrintCmd::PrintCmd(std::string string) : string(std::move(string)) {}

void PrintCmd::accept(ASTVisitor &visitor) { visitor.visit(*this); }

ShowCmd::ShowCmd(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}

void ShowCmd::accept(ASTVisitor &visitor) { visitor.visit(*this); }

TimeCmd::TimeCmd(std::unique_ptr<Cmd> cmd) : cmd(std::move(cmd)) {}

void TimeCmd::accept(ASTVisitor &visitor) { visitor.visit(*this); }

IntExpr::IntExpr(int64_t value) : value(value) {}

void IntExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }

FloatExpr::FloatExpr(double value) : value(value) {}

void FloatExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }

TrueExpr::TrueExpr() {}

void TrueExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }

FalseExpr::FalseExpr() {}

void FalseExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }

VarExpr::VarExpr(std::string variable) : variable(std::move(variable)) {}

void VarExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }

ArrayLiteralExpr::ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>> elements)
    : elements(std::move(elements)) {}

void ArrayLiteralExpr::accept(ASTVisitor &visitor) { visitor.visit(*this); }

VarLValue::VarLValue(std::string variable) : variable{variable} {}

void VarLValue::accept(ASTVisitor &visitor) { visitor.visit(*this); }
