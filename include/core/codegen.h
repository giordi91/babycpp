#pragma once
#include <memory>
#include <unordered_map>

#include "factoryAST.h"
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
  bool isFunctionDynTypeRepl: 1;
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
  ExprAST() {
    flags.isReturn = 0;
    flags.isDefinition = 0;
    flags.isFunctionDynTypeRepl= 0;
  };
  ExprAST(int type) : datatype(type) {
    flags.isReturn = 0;
    flags.isDefinition = 0;
    flags.isFunctionDynTypeRepl= 0;
  }
  virtual ~ExprAST() = default;
  virtual llvm::Value *codegen(Codegenerator *gen) = 0;

  int datatype = 0;
  ASTFlags flags;
};

struct NumberExprAST : public ExprAST {
  explicit NumberExprAST(Number val) : ExprAST(), val(val) {
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
      : ExprAST(), op(op), lhs(lhs), rhs(rhs) {}

  llvm::Value *codegen(Codegenerator *gen) override;
};

struct CallExprAST : public ExprAST {
  std::string callee;
  std::vector<ExprAST *> args;

  explicit CallExprAST(const std::string &callee, std::vector<ExprAST *> &args)
      : ExprAST(), callee(callee), args(args) {}
  llvm::Value *codegen(Codegenerator *gen) override;
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
      : ExprAST(), proto(inproto), body(inbody) {}
  llvm::Value *codegen(Codegenerator *gen) override;
};

struct Codegenerator {

  explicit Codegenerator();
  void createNewModule();
  void setCurrentModule(std::shared_ptr<llvm::Module> mod);
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

  static void dumpLlvmData(llvm::Value *v, const std::string &path) {
    const std::string outs = printLlvmData(v);
    std::ofstream out(path);
    out << outs;
    out.close();
  }
  llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function,
                                           const std::string &varName,
                                           int type);

  int omogenizeOperation(ExprAST *L, ExprAST *R, llvm::Value **Lvalue,
                         llvm::Value **Rvalue);
  static bool compareASTArgWithLLVMArg(ExprAST *astArg,
                                       llvm::Argument *llvmArg);
  memory::FactoryAST factory;
  lexer::Lexer lexer;
  parser::Parser parser;

  llvm::LLVMContext context;
  llvm::IRBuilder<> builder;
  std::shared_ptr<llvm::Module> module;

  std::unordered_map<std::string, llvm::AllocaInst *> namedValues;
  static const std::unordered_map<int, int> AST_LLVM_MAP;
  llvm::Function *currentScope = nullptr;

  void generateModuleContent();
};

} // namespace codegen
} // namespace babycpp
