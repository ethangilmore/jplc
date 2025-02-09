#include "parser.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

Parser::Parser(Lexer &lexer, Logger &logger) : logger(logger), lexer(lexer) {}

Token Parser::consume(Token::Type type) {
  Token token = lexer.next();
  if (token.type != type) {
    logger.log_error("Unexpected token: " + token.value, token.start);
  }
  return token;
}

Token Parser::consume(Token::Type type, const std::string &value) {
  Token token = lexer.next();
  if (token.type != type || token.value != value) {
    logger.log_error("Unexpected token: " + token.value, token.start);
  }
  return token;
}

/* ========== Program ========== */
std::unique_ptr<Program> Parser::parse() {
  std::vector<std::unique_ptr<Cmd>> cmds;
  Token token = lexer.next();
  while (token.type != Token::Type::Eof) {
    if (token.type == Token::Type::NewLine) {
      token = lexer.next();
      continue;
    }
    cmds.push_back(parse_cmd(token));
    consume(Token::Type::NewLine);
    token = lexer.next();
  }
  return std::make_unique<Program>(std::move(cmds));
}

/* ========== Type ========== */
std::unique_ptr<Type> Parser::parse_type(Token token) {
  std::unique_ptr<Type> type = parse_base_type(token);
  while (lexer.peek().type == Token::Type::LSquare) {
    type = parse_array_type(std::move(type));
  }
  return type;
}

std::unique_ptr<Type> Parser::parse_base_type(Token token) {
  switch (token.type) {
    case Token::Type::Int:
      return std::make_unique<IntType>();
    case Token::Type::Bool:
      return std::make_unique<BoolType>();
    case Token::Type::Float:
      return std::make_unique<FloatType>();
    case Token::Type::Variable:
      return std::make_unique<StructType>(std::move(token.value));
    case Token::Type::Void:
      return std::make_unique<VoidType>();
    default:
      logger.log_error("Unexpected token: " + token.value, token.start);
  }
}

std::unique_ptr<Type> Parser::parse_array_type(
    std::unique_ptr<Type> base_type) {
  consume(Token::Type::LSquare);
  size_t rank = 1;
  while (lexer.peek().type == Token::Type::Comma) {
    consume(Token::Type::Comma);
    rank++;
  }
  consume(Token::Type::RSquare);
  return std::make_unique<ArrayType>(std::move(base_type), rank);
}

/* ========== Cmd ========== */
std::unique_ptr<Cmd> Parser::parse_cmd(Token token) {
  switch (token.type) {
    case Token::Type::Read:
      return parse_read_cmd(token);
    case Token::Type::Write:
      return parse_write_cmd(token);
    case Token::Type::Let:
      return parse_let_cmd(token);
    case Token::Type::Assert:
      return parse_assert_cmd(token);
    case Token::Type::Print:
      return parse_print_cmd(token);
    case Token::Type::Show:
      return parse_show_cmd(token);
    case Token::Type::Time:
      return parse_time_cmd(token);
    case Token::Type::Fn:
      return parse_fn_cmd(token);
    case Token::Type::Struct:
      return parse_struct_cmd(token);
    default:
      logger.log_error("Unexpected token: " + token.value, token.start);
  }
}

std::unique_ptr<ReadCmd> Parser::parse_read_cmd(Token token) {
  consume(Token::Type::Image);
  std::string string = consume(Token::Type::String).value;
  consume(Token::Type::To);
  std::unique_ptr<LValue> lvalue = parse_lvalue(lexer.next());
  return std::make_unique<ReadCmd>(std::string(string), std::move(lvalue));
}

std::unique_ptr<WriteCmd> Parser::parse_write_cmd(Token token) {
  consume(Token::Type::Image);
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  consume(Token::Type::To);
  std::string string = consume(Token::Type::String).value;
  return std::make_unique<WriteCmd>(std::move(expr), std::move(string));
}

std::unique_ptr<LetCmd> Parser::parse_let_cmd(Token token) {
  std::unique_ptr<LValue> lvalue = parse_lvalue(lexer.next());
  consume(Token::Type::Equals);
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  return std::make_unique<LetCmd>(std::move(lvalue), std::move(expr));
}

std::unique_ptr<AssertCmd> Parser::parse_assert_cmd(Token token) {
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  consume(Token::Type::Comma);
  std::string string = consume(Token::Type::String).value;
  return std::make_unique<AssertCmd>(std::move(expr), std::move(string));
}

std::unique_ptr<PrintCmd> Parser::parse_print_cmd(Token token) {
  std::string string = consume(Token::Type::String).value;
  return std::make_unique<PrintCmd>(std::move(string));
}

std::unique_ptr<ShowCmd> Parser::parse_show_cmd(Token token) {
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  return std::make_unique<ShowCmd>(std::move(expr));
}

std::unique_ptr<TimeCmd> Parser::parse_time_cmd(Token token) {
  std::unique_ptr<Cmd> cmd = parse_cmd(lexer.next());
  return std::make_unique<TimeCmd>(std::move(cmd));
}

std::unique_ptr<FnCmd> Parser::parse_fn_cmd(Token token) {
  std::string identifier = consume(Token::Type::Variable).value;
  consume(Token::Type::LParen);
  std::vector<std::unique_ptr<Binding>> params;
  while (true) {
    Token next = lexer.next();
    if (next.type == Token::Type::RParen) {
      break;
    }
    params.push_back(parse_binding(next));
    next = lexer.next();
    if (next.type == Token::Type::RParen) {
      break;
    } else if (next.type != Token::Type::Comma) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
  }
  consume(Token::Type::Colon);
  std::unique_ptr<Type> return_type = parse_type(lexer.next());
  consume(Token::Type::LCurly);
  std::vector<std::unique_ptr<Stmt>> stmts;
  while (consume(Token::Type::NewLine).type == Token::Type::NewLine) {
    Token next = lexer.next();
    if (next.type == Token::Type::NewLine) {
      continue;
    } else if (next.type == Token::Type::RCurly) {
      break;
    }
    stmts.push_back(parse_stmt(next));
  }
  return std::make_unique<FnCmd>(std::move(identifier), std::move(params),
                                 std::move(return_type), std::move(stmts));
}

std::unique_ptr<StructCmd> Parser::parse_struct_cmd(Token token) {
  std::string identifier = consume(Token::Type::Variable).value;
  consume(Token::Type::LCurly);
  consume(Token::Type::NewLine);
  std::vector<std::pair<std::string, std::unique_ptr<Type>>> fields;
  while (true) {
    Token next = lexer.next();
    if (next.type == Token::Type::RCurly) {
      break;
    } else if (next.type != Token::Type::Variable) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
    std::string field_name = next.value;
    consume(Token::Type::Colon);
    std::unique_ptr<Type> field_type = parse_type(lexer.next());
    fields.push_back(std::make_pair(field_name, std::move(field_type)));
    next = lexer.next();
    if (next.type == Token::Type::RCurly) {
      break;
    } else if (next.type != Token::Type::NewLine) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
  }
  return std::make_unique<StructCmd>(std::move(identifier), std::move(fields));
}

/* ========== Stmt ========== */
std::unique_ptr<Stmt> Parser::parse_stmt(Token token) {
  switch (token.type) {
    case Token::Type::Let:
      return parse_let_stmt(token);
    case Token::Type::Assert:
      return parse_assert_stmt(token);
    case Token::Type::Return:
      return parse_return_stmt(token);
    default:
      logger.log_error("Unexpected token: " + token.value, token.start);
  }
}

std::unique_ptr<LetStmt> Parser::parse_let_stmt(Token token) {
  std::unique_ptr<LValue> lvalue = parse_lvalue(lexer.next());
  consume(Token::Type::Equals);
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  return std::make_unique<LetStmt>(std::move(lvalue), std::move(expr));
}

std::unique_ptr<AssertStmt> Parser::parse_assert_stmt(Token token) {
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  consume(Token::Type::Comma);
  std::string string = consume(Token::Type::String).value;
  return std::make_unique<AssertStmt>(std::move(expr), std::move(string));
}

std::unique_ptr<ReturnStmt> Parser::parse_return_stmt(Token token) {
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  return std::make_unique<ReturnStmt>(std::move(expr));
}

/* ========== Expr ========== */
std::unique_ptr<Expr> Parser::parse_expr(Token token) {
  return parse_boolean_expr(token);
}

std::unique_ptr<Expr> Parser::parse_boolean_expr(Token token) {
  std::unique_ptr<Expr> base_expr = parse_compare_expr(token);
  std::unordered_set<std::string> ops{"&&", "||"};
  while (ops.count(lexer.peek().value)) {
    std::string op = lexer.next().value;
    base_expr = std::make_unique<BinopExpr>(std::move(base_expr), op, parse_compare_expr(lexer.next()));
  }
  return base_expr;
}

std::unique_ptr<Expr> Parser::parse_compare_expr(Token token) {
  std::unique_ptr<Expr> base_expr = parse_add_expr(token);
  std::unordered_set<std::string> ops{"<", ">", "<=", ">=", "==", "!="};
  while (ops.count(lexer.peek().value)) {
    std::string op = lexer.next().value;
    base_expr = std::make_unique<BinopExpr>(std::move(base_expr), op, parse_add_expr(lexer.next()));
  }
  return base_expr;
}

std::unique_ptr<Expr> Parser::parse_add_expr(Token token) {
  std::unique_ptr<Expr> base_expr = parse_mult_expr(token);
  std::unordered_set<std::string> ops{"+", "-"};
  while (ops.count(lexer.peek().value)) {
    std::string op = lexer.next().value;
    base_expr = std::make_unique<BinopExpr>(std::move(base_expr), op, parse_mult_expr(lexer.next()));
  }
  return base_expr;
}

std::unique_ptr<Expr> Parser::parse_mult_expr(Token token) {
  std::unique_ptr<Expr> base_expr = parse_unop_expr(token);
  std::unordered_set<std::string> ops{"*", "/", "%"};
  while (ops.count(lexer.peek().value)) {
    std::string op = lexer.next().value;
    base_expr = std::make_unique<BinopExpr>(std::move(base_expr), op, parse_unop_expr(lexer.next()));
  }
  return base_expr;
}

std::unique_ptr<Expr> Parser::parse_unop_expr(Token token) {
  std::unordered_set<std::string> ops{"-", "!"};
  if (ops.count(token.value)) {
    auto expr = parse_unop_expr(lexer.next());
    return std::make_unique<UnopExpr>(token.value, std::move(expr));
  } else {
    auto expr = parse_index_expr(token);
    return expr;
  }
}

std::unique_ptr<Expr> Parser::parse_index_expr(Token token) {
  std::unique_ptr<Expr> base_expr = parse_base_expr(token);
  while (lexer.peek().type == Token::Type::Dot || lexer.peek().type == Token::Type::LSquare) {
    if (lexer.peek().type == Token::Type::Dot) {
      base_expr = parse_dot_expr(std::move(base_expr));
    } else if (lexer.peek().type == Token::Type::LSquare) {
      base_expr = parse_array_index_expr(std::move(base_expr));
    } else {
      break;
    }
  }
  return base_expr;
}

std::unique_ptr<Expr> Parser::parse_base_expr(Token token) {
  switch (token.type) {
    case Token::Type::IntVal:
      return parse_int_expr(token);
    case Token::Type::FloatVal:
      return parse_float_expr(token);
    case Token::Type::True:
      return parse_true_expr(token);
    case Token::Type::False:
      return parse_false_expr(token);
    case Token::Type::Variable:
      return parse_var_expr(token);
    case Token::Type::Void:
      return parse_void_expr(token);
    case Token::Type::LSquare:
      return parse_array_literal_expr(token);
    case Token::Type::LParen:
      return parse_paren_expr(token);
    case Token::Type::If:
      return parse_if_expr(token);
    case Token::Type::Array:
      return parse_array_loop_expr(token);
    case Token::Type::Sum:
      return parse_sum_loop_expr(token);
    default:
      logger.log_error("Unexpected token: " + token.value, token.start);
  }
}

std::unique_ptr<IntExpr> Parser::parse_int_expr(Token token) {
  try {
    return std::make_unique<IntExpr>(std::stoll(token.value));
  } catch (std::out_of_range) {
    logger.log_error("Integer literal out of range: " + token.value,
                     token.start);
  }
}

std::unique_ptr<FloatExpr> Parser::parse_float_expr(Token token) {
  try {
    return std::make_unique<FloatExpr>(std::stod(token.value));
  } catch (std::out_of_range) {
    logger.log_error("Float literal out of range: " + token.value, token.start);
  }
}

std::unique_ptr<TrueExpr> Parser::parse_true_expr(Token token) {
  return std::make_unique<TrueExpr>();
}

std::unique_ptr<FalseExpr> Parser::parse_false_expr(Token token) {
  return std::make_unique<FalseExpr>();
}

std::unique_ptr<Expr> Parser::parse_var_expr(Token token) {
  switch (lexer.peek().type) {
    case Token::Type::LCurly:
      return parse_struct_literal_expr(token);
    case Token::Type::LParen:
      return parse_call_expr(token);
    default:
      return std::make_unique<VarExpr>(std::move(token.value));
  }
}

std::unique_ptr<VoidExpr> Parser::parse_void_expr(Token token) {
  return std::make_unique<VoidExpr>();
}

std::unique_ptr<ArrayLiteralExpr> Parser::parse_array_literal_expr(
    Token token) {
  std::vector<std::unique_ptr<Expr>> elements;
  while (true) {
    Token next = lexer.next();
    if (next.type == Token::Type::RSquare) {
      return std::make_unique<ArrayLiteralExpr>(std::move(elements));
    }
    elements.push_back(parse_expr(next));
    next = lexer.next();
    if (next.type == Token::Type::RSquare) {
      return std::make_unique<ArrayLiteralExpr>(std::move(elements));
    } else if (next.type != Token::Type::Comma) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
  }
}

std::unique_ptr<StructLiteralExpr> Parser::parse_struct_literal_expr(
    Token token) {
  std::string identifier = token.value;
  consume(Token::Type::LCurly);
  std::vector<std::unique_ptr<Expr>> fields;
  while (true) {
    Token next = lexer.next();
    if (next.type == Token::Type::RCurly) {
      break;
    }
    fields.push_back(parse_expr(next));
    next = lexer.next();
    if (next.type == Token::Type::RCurly) {
      break;
    } else if (next.type != Token::Type::Comma) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
  }
  return std::make_unique<StructLiteralExpr>(std::move(identifier),
                                             std::move(fields));
}

std::unique_ptr<Expr> Parser::parse_paren_expr(Token token) {
  std::unique_ptr<Expr> expr = parse_expr(lexer.next());
  consume(Token::Type::RParen);
  return expr;
}

std::unique_ptr<DotExpr> Parser::parse_dot_expr(
    std::unique_ptr<Expr> base_expr) {
  consume(Token::Type::Dot);
  std::string field = consume(Token::Type::Variable).value;
  return std::make_unique<DotExpr>(std::move(base_expr), std::move(field));
}

std::unique_ptr<ArrayIndexExpr> Parser::parse_array_index_expr(
    std::unique_ptr<Expr> base_expr) {
  consume(Token::Type::LSquare);
  std::vector<std::unique_ptr<Expr>> indices;
  while (true) {
    Token next = lexer.next();
    if (next.type == Token::Type::RSquare) {
      break;
    }
    indices.push_back(parse_expr(next));
    next = lexer.next();
    if (next.type == Token::Type::RSquare) {
      break;
    } else if (next.type != Token::Type::Comma) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
  }
  return std::make_unique<ArrayIndexExpr>(std::move(base_expr),
                                          std::move(indices));
}

std::unique_ptr<CallExpr> Parser::parse_call_expr(Token token) {
  std::string identifier = token.value;
  consume(Token::Type::LParen);
  std::vector<std::unique_ptr<Expr>> args;
  while (true) {
    Token next = lexer.next();
    if (next.type == Token::Type::RParen) {
      break;
    }
    args.push_back(parse_expr(next));
    next = lexer.next();
    if (next.type == Token::Type::RParen) {
      break;
    } else if (next.type != Token::Type::Comma) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
  }
  return std::make_unique<CallExpr>(std::move(identifier), std::move(args));
}

std::unique_ptr<ArrayLoopExpr> Parser::parse_array_loop_expr(Token token) {
  consume(Token::Type::LSquare);
  std::vector<std::pair<std::string, std::unique_ptr<Expr>>> axis;
  while (true) {
    if (lexer.peek().type == Token::Type::RSquare) {
      consume(Token::Type::RSquare);
      break;
    }
    auto variable = consume(Token::Type::Variable).value;
    consume(Token::Type::Colon);
    auto expr = parse_expr(lexer.next());
    axis.push_back(std::make_pair(variable, std::move(expr)));
    auto next = lexer.next();
    if (next.type == Token::Type::RSquare) {
      break;
    } else if (next.type != Token::Type::Comma) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
  }
  auto expr = parse_expr(lexer.next());
  return std::make_unique<ArrayLoopExpr>(std::move(axis), std::move(expr));
}

std::unique_ptr<SumLoopExpr> Parser::parse_sum_loop_expr(Token token) {
  consume(Token::Type::LSquare);
  std::vector<std::pair<std::string, std::unique_ptr<Expr>>> axis;
  while (true) {
    if (lexer.peek().type == Token::Type::RSquare) {
      consume(Token::Type::RSquare);
      break;
    }
    auto variable = consume(Token::Type::Variable).value;
    consume(Token::Type::Colon);
    auto expr = parse_expr(lexer.next());
    axis.push_back(std::make_pair(variable, std::move(expr)));
    auto next = lexer.next();
    if (next.type == Token::Type::RSquare) {
      break;
    } else if (next.type != Token::Type::Comma) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
  }
  auto expr = parse_expr(lexer.next());
  return std::make_unique<SumLoopExpr>(std::move(axis), std::move(expr));
}

std::unique_ptr<IfExpr> Parser::parse_if_expr(Token token) {
  auto condition = parse_expr(lexer.next());
  consume(Token::Type::Then);
  auto if_expr = parse_expr(lexer.next());
  consume(Token::Type::Else);
  auto else_expr = parse_expr(lexer.next());
  return std::make_unique<IfExpr>(std::move(condition), std::move(if_expr), std::move(else_expr));
}

/* ========== LValue ========== */
std::unique_ptr<LValue> Parser::parse_lvalue(Token token) {
  switch (lexer.peek().type) {
    case Token::Type::LSquare:
      return parse_array_lvalue(token);
    default:
      return parse_var_lvalue(token);
  }
}

std::unique_ptr<VarLValue> Parser::parse_var_lvalue(Token token) {
  return std::make_unique<VarLValue>(std::move(token.value));
}

std::unique_ptr<ArrayLValue> Parser::parse_array_lvalue(Token token) {
  auto identifier = token.value;
  consume(Token::Type::LSquare);
  std::vector<std::string> indices;
  while (true) {
    if (lexer.peek().type == Token::Type::RSquare) {
      break;
    }
    indices.push_back(consume(Token::Type::Variable).value);
    Token next = lexer.next();
    if (next.type == Token::Type::RSquare) {
      break;
    } else if (next.type != Token::Type::Comma) {
      logger.log_error("Unexpected token: " + next.value, next.start);
    }
  }
  return std::make_unique<ArrayLValue>(std::move(identifier),
                                       std::move(indices));
}

/* ========== Binding ========== */
std::unique_ptr<Binding> Parser::parse_binding(Token token) {
  std::unique_ptr<LValue> lvalue = parse_lvalue(token);
  consume(Token::Type::Colon);
  std::unique_ptr<Type> type = parse_type(lexer.next());
  return std::make_unique<Binding>(std::move(lvalue), std::move(type));
}
