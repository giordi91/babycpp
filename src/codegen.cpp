#include "codegen.h"
#include <iostream>

#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Verifier.h>

namespace babycpp {
namespace codegen {

using llvm::Value;

Value *NumberExprAST::codegen(Codegenerator *gen) const {

  if (val.type == Token::tok_float) {
    return llvm::ConstantFP::get(gen->context, llvm::APFloat(val.floatNumber));

  } else if (val.type == Token::tok_int) {
    return llvm::ConstantInt::get(gen->context,
                                  llvm::APInt(32, val.integerNumber));
  }
  // this should not be triggered, we should find this errors at
  // parsing time
  std::cout << "Error unrecognized type number on code gen" << std::endl;
  return nullptr;
}
Codegenerator::Codegenerator()
    : lexer(), parser(&lexer), context(), builder(context),
      module("", context) {}
llvm::Value *VariableExprAST::codegen(Codegenerator *gen) const {

  // first we try to see if the variable is already defined at scope
  // level
  Value *v = gen->namedValues[name];
  // here we extract the variable from the scope.
  // if we get a nullptr and the variable is not a definition
  // we got an error
  if (v == nullptr && !flags.isDefinition) {
    std::cout << "Error variable " << name << " not defined" << std::endl;
    return nullptr;
  }

  // if we get here it means the variable needs to be defined
  // TODO(giordi) implement alloca for variable definition
  return v;
}

llvm::Value *BinaryExprAST::codegen(Codegenerator *gen) const {
  // generating code recursively for left and right end side
  Value *L = lhs->codegen(gen);
  Value *R = rhs->codegen(gen);

  if (L == nullptr || R == nullptr) {
    return nullptr;
  }

  // checking the operator to generate the correct operation
  if (op == "+") {
    return gen->builder.CreateFAdd(L, R, "addtmp");
  } else if (op == "-") {
    return gen->builder.CreateFSub(L, R, "subtmp");
  } else if (op == "*") {
    return gen->builder.CreateFMul(L, R, "multmp");
  } else if (op == "/") {
    return gen->builder.CreateFDiv(L, R, "divtmp");
  } else if (op == "<") {
    L = gen->builder.CreateFCmpULT(L, R, "cmptmp");
    return gen->builder.CreateUIToFP(L, llvm::Type::getDoubleTy(gen->context),
                                     "booltmp");
  }
  std::cout << "error unrecognized operator" << std::endl;
  return nullptr;
}

llvm::Value *PrototypeAST::codegen(Codegenerator *gen) const {
  std::vector<llvm::Type *> Doubles(args.size(),
                                    llvm::Type::getDoubleTy(gen->context));
  auto *funcType = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(gen->context), Doubles, false);

  auto *function = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, name, &gen->module);
  // Set names for all arguments.
  uint32_t Idx = 0;
  for (auto &arg : function->args())
    arg.setName(args[Idx++].name);

  return function;
}

llvm::Value *FunctionAST::codegen(Codegenerator *gen) const {
  //// First, check for an existing function from a previous 'extern'
  /// declaration.
  llvm::Function *function = gen->module.getFunction(proto->name);

  if (function == nullptr) {
    Value *p = proto->codegen(gen);
    function = static_cast<llvm::Function*>(p);
  }
  if (function == nullptr) {
    std::cout << "error generating protoype code gen" << std::endl;
    return nullptr;
  }

  if (!function->empty()) {
    std::cout << "Function cannot be redefined." << std::endl;
    return nullptr;
  }

  using llvm::BasicBlock;
  // Create a new basic block to start insertion into.
  BasicBlock *block = BasicBlock::Create(gen->context, "entry", function);
  gen->builder.SetInsertPoint(block);

  // Record the function arguments in the NamedValues map.
  gen->namedValues.clear();
  for (auto &arg : function->args())
    gen->namedValues[arg.getName()] = &arg;

  for (auto &b : body) {
    if (Value *RetVal = b->codegen(gen)) {
      if (b->flags.isReturn) {
        gen->builder.CreateRet(RetVal);
      }
    }
  }
  verifyFunction(*function);
  return function;
}

} // namespace codegen
} // namespace babycpp
