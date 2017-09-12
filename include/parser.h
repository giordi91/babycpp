#pragma once
#include "lexer.h"

namespace babycpp {
namespace parser {

using lexer::Lexer;
using lexer::Number;


// base struct for all ast expr
struct ExprAST {
  virtual ~ExprAST() = default;
};

struct NumberExprAST : public ExprAST {
  NumberExprAST(Number val) : val(val) {}

  // keeping it public mainly for testabiliy purposes
  Number val;
};

struct VariableExprAST : public ExprAST {
  std::string name;
  VariableExprAST(const std::string &name) : name{name} {}
};

struct BinaryExprAST : public ExprAST {
  char op;
  ExprAST *lhs, *rhs;
  BinaryExprAST(char op, ExprAST *lhs, ExprAST *rhs)
      : op(op), lhs(lhs), rhs(rhs) {}
};

struct CallExprAST : public ExprAST {
  std::string callee;
  std::vector<ExprAST *> args;

  CallExprAST(const std::string &callee, std::vector<ExprAST *> &args)
      : callee(callee), args(args) {}
};

struct PrototypeAST {
  std::string name;
  std::vector<std::string> args;

  PrototypeAST(const std::string &name, const std::vector<std::string> &args)
      : name(name), args(args) {}
};

struct FunctionAST {
  PrototypeAST *proto;
  ExprAST *body;

  FunctionAST(PrototypeAST *proto, ExprAST *body)
      : proto(proto), body(body) {}
};

struct Parser {
  explicit Parser(Lexer* inputLexer):lex(inputLexer){}

  NumberExprAST* parseNumber();
  const static std::map<char, int> BIN_OP_PRECEDENCE;
  Lexer* lex;

};


} //namespace parser
} // namespace babycpp
