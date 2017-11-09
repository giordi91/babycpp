#include "codegen.h"
#include <iostream>

#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Verifier.h>

namespace babycpp {
namespace codegen {

using llvm::Value;

const std::unordered_map<int, int> Codegenerator::AST_LLVM_MAP{
    {Token::tok_float, llvm::Type::TypeID::FloatTyID},
    {Token::tok_int, llvm::Type::TypeID::IntegerTyID},
};


llvm::AllocaInst *
Codegenerator::createEntryBlockAlloca(llvm::Function *function,
                                      const std::string &varName, int type) {
  llvm::IRBuilder<> tempBuilder(&function->getEntryBlock(),
                                function->getEntryBlock().begin());
  llvm::Type *varType = getType(type, this);
  return tempBuilder.CreateAlloca(varType, nullptr, varName);
}

Codegenerator::Codegenerator()
    : diagnostic(), lexer(diagnostic), parser(&lexer, &factory), builder(context),
      module(new llvm::Module("", context)) {}

void Codegenerator::setCurrentModule(std::shared_ptr<llvm::Module> mod) {
  module = mod;
}

int Codegenerator::omogenizeOperation(ExprAST *leftAST, ExprAST *rightAST,
                                      llvm::Value **leftValue,
                                      llvm::Value **rightValue) {

  int Ltype = leftAST->datatype;
  int Rtype = rightAST->datatype;

  if (Ltype == 0 || Rtype == 0) {
    //TODO(giordi) implement proper error log
    std::cout << "error cannot deduce output type of operation" << std::endl;
    return -1;
  }

  if (Ltype == Rtype) {
    // same type nothing to do here
    return Ltype;
  }

  if (Ltype == Token::tok_float && Rtype == Token::tok_int) {
    // need to convert R side
    *rightValue = builder.CreateUIToFP(*rightValue, llvm::Type::getFloatTy(context),
                                   "intToFPcast");
    // TODO(giordi) implement waning log
    // std::cout << "warning: implicit conversion int->float" << std::endl;
    return Token::tok_float;
  }
  if (Rtype == Token::tok_float && Ltype == Token::tok_int) {
    // need to convert L side
    *leftValue = builder.CreateUIToFP(*leftValue, llvm::Type::getFloatTy(context),
                                   "intToFPcast");
    // TODO(giordi) implement waning log
    // std::cout << "warning: implicit conversion int->float" << std::endl;
    return Token::tok_float;
  }

  // should never reach this
  return -1;
}



bool Codegenerator::compareASTArgWithLLVMArg(ExprAST *astArg,
                                             llvm::Argument *llvmArg) {
  auto found = AST_LLVM_MAP.find(astArg->datatype);
  if (found != AST_LLVM_MAP.end()) {
    if (found->second == llvmArg->getType()->getTypeID()) {
      return true;
    }
  }
  return false;
}
llvm::Function *Codegenerator::getFunction(const std::string &Name) {
  // First, see if the function has already been added to the current module.
  if (auto *F = module->getFunction(Name))
    return F;

  // If not, check whether we can codegen the declaration from some existing
  // prototype.
  auto FI = functionProtos.find(Name);
  if (FI != functionProtos.end()) {
    auto *f = FI->second->codegen(this);
    if (f == nullptr) {
      return nullptr;
    }
    return static_cast<llvm::Function *>(f);
  }

  // If no existing prototype exists, return null.
  return nullptr;
}

void Codegenerator::generateModuleContent() {

  while (lexer.currtok != Token::tok_eof) {
    ExprAST *res = parser.parseStatement();
    if (res != nullptr) {
      res->codegen(this);
    }
  }
}

} // namespace codegen
} // namespace babycpp
