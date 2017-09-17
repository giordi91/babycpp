#pragma once
#include "lexer.h"

#include <llvm/IR/Value.h>

namespace babycpp {
namespace codegen {

using lexer::Number;
using llvm::Value;

struct ASTFlags {
  bool isReturn : 1;
};

struct Argument {
  Argument(int datatype, std::string &argName)
      : type(datatype), name(argName) {}
  int type;
  std::string name;
};

// base struct for all ast expr
struct ExprAST {
  ExprAST() = default;
  ExprAST(int type) : datatype(type) {}
  virtual ~ExprAST() = default;
  virtual Value *codegen() const { return nullptr; };
  int datatype = 0;
  ASTFlags flags;
};

struct NumberExprAST : public ExprAST {
  NumberExprAST(Number val) : val(val) {}
  Value *codegen() const override;

  // keeping it public mainly for testability purposes
  Number val;
};

struct VariableExprAST : public ExprAST {
  std::string name;
  ExprAST *value;
  VariableExprAST(const std::string &name, ExprAST *invalue, int type)
      : ExprAST(type), name{name}, value(invalue) {}
};

struct BinaryExprAST : public ExprAST {
  std::string op;
  ExprAST *lhs, *rhs;
  BinaryExprAST(std::string &op, ExprAST *lhs, ExprAST *rhs)
      : op(op), lhs(lhs), rhs(rhs) {}
};

struct CallExprAST : public ExprAST {
  std::string callee;
  std::vector<ExprAST *> args;

  CallExprAST(const std::string &callee, std::vector<ExprAST *> &args)
      : callee(callee), args(args) {}
};

struct PrototypeAST : public ExprAST {
  std::string name;
  std::vector<Argument> args;
  bool isExtern = false;

  PrototypeAST(int retType, const std::string &name,
               const std::vector<Argument> &args, bool externProto)
      : ExprAST(retType), name(name), args(args), isExtern(externProto) {}
};

struct FunctionAST : public ExprAST {
  PrototypeAST *proto;
  std::vector<ExprAST *> body;

  FunctionAST(PrototypeAST *inproto, std::vector<ExprAST *> &inbody)
      : proto(inproto), body(inbody) {}
};

} // namespace codegen
} // namespace babycpp
