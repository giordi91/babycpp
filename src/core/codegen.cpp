#include "codegen.h"
#include <iostream>

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
                                      const std::string &varName, int type,
                                      bool isPointer) {
  llvm::IRBuilder<> tempBuilder(&function->getEntryBlock(),
                                function->getEntryBlock().begin());
  llvm::Type *varType = getType(type, this, isPointer);
  return tempBuilder.CreateAlloca(varType, nullptr, varName);
}

Codegenerator::Codegenerator(bool loadBuiltinFunctions)
    : lexer(&diagnostic), parser(&lexer, &factory, &diagnostic),
      builder(context), module(new llvm::Module("", context)) {
  if (loadBuiltinFunctions) {
    doLoadBuiltinFunctions();
  }
}

void Codegenerator::doLoadBuiltinFunctions() {
  ;
  std::string sizeArgument("size");
  std::string pointerArgument("ptr");
  auto *mallocFunc = factory.allocPrototypeAST(
      Token::tok_void_ptr, "malloc",
      std::vector<Argument>{Argument(Token::tok_int, sizeArgument, false)},
      true);

  auto *freeFunc = factory.allocPrototypeAST(
	  Token::tok_void_ptr, "free",
	  std::vector<Argument>{
	  Argument(Token::tok_void_ptr, pointerArgument, true)},
	  true);
  mallocFunc->flags.isPointer = true;
  freeFunc->flags.isPointer = false;
  freeFunc->flags.isNull= true;
  builtInFunctions["malloc"] = mallocFunc;
  builtInFunctions["free"] = freeFunc;
}

void Codegenerator::setCurrentModule(std::shared_ptr<llvm::Module> mod) {
  module = mod;
}

int Codegenerator::omogenizeOperation(ExprAST *leftAST, ExprAST *rightAST,
                                      llvm::Value **leftValue,
                                      llvm::Value **rightValue) {

  int Ltype = leftAST->datatype;
  int Rtype = rightAST->datatype;

  if (Ltype == 0 || Rtype == 0) {
    // TODO(giordi) implement proper error log
    std::cout << "error cannot deduce output type of operation" << std::endl;
    return -1;
  }

  if (Ltype == Rtype) {
    // same type nothing to do here
    return Ltype;
  }

  if (Ltype == Token::tok_float && Rtype == Token::tok_int) {
    // need to convert R side
    *rightValue = builder.CreateUIToFP(
        *rightValue, llvm::Type::getFloatTy(context), "intToFPcast");
    // TODO(giordi) implement waning log
    // std::cout << "warning: implicit conversion int->float" << std::endl;
    return Token::tok_float;
  }
  if (Rtype == Token::tok_float && Ltype == Token::tok_int) {
    // need to convert L side
    *leftValue = builder.CreateUIToFP(
        *leftValue, llvm::Type::getFloatTy(context), "intToFPcast");
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

std::string Codegenerator::printDiagnostic() {
  std::string diagnosticMessage =
      "================== ERRORS ================= \n";
  while (diagnostic.hasErrors() != 0) {
    auto err = diagnostic.getError();
    diagnosticMessage += diagnostic.printErorr(err);
    diagnosticMessage += "\n";
  }
  return diagnosticMessage;
}

llvm::Function *Codegenerator::getFunction(const std::string &name,
                                           PrototypeAST **returnProto) {
  // First, see if the function has already been added to the current module.
  if (auto *F = module->getFunction(name)) {

    if (returnProto != nullptr) {
      // trying to find the prototype
      auto FI = functionProtos.find(name);
      if (FI != functionProtos.end()) {
        *returnProto = FI->second;
      } else {
        FI = builtInFunctions.find(name);
        if (FI != builtInFunctions.end()) {
            *returnProto = FI->second;
          }
        }
      }
	else
	{
		std::cout << "cannot return proto pointer is null" << std::endl;
	}
      return F;
    }

    // If not, check whether we can codegen the declaration from some existing
    // prototype.
    auto FI = functionProtos.find(name);
    if (FI != functionProtos.end()) {
      auto *f = FI->second->codegen(this);
      if (returnProto != nullptr)
        *returnProto = FI->second;
      if (f == nullptr) {
        return nullptr;
      }
      return static_cast<llvm::Function *>(f);
    }
    // lets check the built in functions
    FI = builtInFunctions.find(name);
    if (FI != builtInFunctions.end()) {
      auto *f = FI->second->codegen(this);
      if (returnProto != nullptr)
        *returnProto = FI->second;
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
} // namespace codegen
