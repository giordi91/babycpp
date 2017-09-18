#pragma once
#include <memory>
#include <unordered_map>

#include "lexer.h"
#include "parser.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

namespace babycpp {

using lexer::Lexer;
using lexer::Token;
using parser::Parser;

namespace codegen {

using lexer::Number;

struct ASTFlags {
  bool isReturn : 1;
  bool isDefinition : 1;
};

struct Argument {
  Argument(int datatype, std::string &argName)
      : type(datatype), name(argName) {}
  int type;
  std::string name;
};

struct Codegenerator;
// base struct for all ast expr
struct ExprAST {
  ExprAST() = default;
  ExprAST(int type) : datatype(type) {}
  virtual ~ExprAST() = default;
  virtual llvm::Value *codegen(Codegenerator *gen) { return nullptr; };

  int datatype = 0;
  ASTFlags flags;
};

struct NumberExprAST : public ExprAST {
  explicit NumberExprAST(Number val) : val(val) {
    // TODO(giordi) add assert for it to be a datatype token
    datatype = val.type;
  }
  llvm::Value *codegen(Codegenerator *gen) override;

  Number val;
};

struct VariableExprAST : public ExprAST {
  std::string name;
  ExprAST *value;
  explicit VariableExprAST(const std::string &name, ExprAST *invalue, int type)
      : ExprAST(type), name{name}, value(invalue) {}
  llvm::Value *codegen(Codegenerator *gen) override;
};

struct BinaryExprAST : public ExprAST {
  std::string op;
  ExprAST *lhs, *rhs;
  explicit BinaryExprAST(std::string &op, ExprAST *lhs, ExprAST *rhs)
      : op(op), lhs(lhs), rhs(rhs) {}

  llvm::Value *codegen(Codegenerator *gen) override;
};

struct CallExprAST : public ExprAST {
  std::string callee;
  std::vector<ExprAST *> args;

  explicit CallExprAST(const std::string &callee, std::vector<ExprAST *> &args)
      : callee(callee), args(args) {}
};

struct PrototypeAST : public ExprAST {
  std::string name;
  std::vector<Argument> args;
  bool isExtern = false;

  explicit PrototypeAST(int retType, const std::string &name,
                        const std::vector<Argument> &args, bool externProto)
      : ExprAST(retType), name(name), args(args), isExtern(externProto) {}
  llvm::Value *codegen(Codegenerator *gen) override;
};

struct FunctionAST : public ExprAST {
  PrototypeAST *proto;
  std::vector<ExprAST *> body;

  explicit FunctionAST(PrototypeAST *inproto, std::vector<ExprAST *> &inbody)
      : proto(inproto), body(inbody) {}
  llvm::Value *codegen(Codegenerator *gen) override;
};

struct Codegenerator {

  explicit Codegenerator();
  inline void initFromString(const std::string &str) {
    lexer.initFromStr(str);
    // getting first token so the parser is ready to go
    lexer.gettok();
  };
  static std::string printLlvmData(llvm::Value *v) {
    std::string outs;
    llvm::raw_string_ostream os(outs);
    v->print(os, false);
    os.flush();
    return outs;
  }

  int omogenizeOperation(ExprAST *L, ExprAST *R,
	  llvm::Value** Lvalue,
	  llvm::Value** Rvalue);

  lexer::Lexer lexer;
  parser::Parser parser;

  llvm::LLVMContext context;
  llvm::IRBuilder<> builder;
  llvm::Module module;

  std::unordered_map<std::string, llvm::Value *> namedValues;
};

} // namespace codegen
} // namespace babycpp
