#include "AST.h"
#include "codegen.h"
#include <iostream>
#include <llvm/IR/Verifier.h>

namespace babycpp {
namespace codegen {

// utility

using diagnostic::IssueCode;
inline void logCodegenError(const std::string &msg, Codegenerator *codegen,
                            diagnostic::IssueCode code) {

  Lexer *lexer = &(codegen->lexer);
  diagnostic::Issue err{msg, lexer->lineNumber, lexer->columnNumber,
                        diagnostic::IssueType::CODEGEN, code};
  lexer->diagnostic->pushError(err);
}

using llvm::Value;
Value *NumberExprAST::codegen(Codegenerator *gen) {

  if (val.type == Token::tok_float) {
    return llvm::ConstantFP::get(gen->context, llvm::APFloat(val.floatNumber));
  }
  if (val.type == Token::tok_int) {
    return llvm::ConstantInt::get(gen->context,
                                  llvm::APInt(32, val.integerNumber));
  }
  // this should not be triggered, we should find this errors at
  // parsing time
  std::cout << "Error unrecognized type number on code gen" << std::endl;
  return nullptr;
}
llvm::Value *VariableExprAST::codegen(Codegenerator *gen) {

  // first we try to see if the variable is already defined at scope
  // level
  llvm::AllocaInst *v = gen->namedValues[name];
  // here we extract the variable from the scope.
  // if we get a nullptr and the variable is not a definition
  // we got an error
  if (v == nullptr && datatype == 0) {
    logCodegenError("Error variable " + name + " not defined", gen,
                    IssueCode::UNDEFINED_VARIABLE);
    return nullptr;
  }

  if (flags.isDefinition) {

    llvm::IRBuilder<> tempBuilder(&gen->currentScope->getEntryBlock(),
                                  gen->currentScope->getEntryBlock().begin());
    llvm::Type *varType = getType(datatype, gen);
    v = tempBuilder.CreateAlloca(varType, nullptr, name);
    gen->namedValues[name] = v;

    if (value == nullptr) {
      std::cout << "error: expected value for value definition" << std::endl;
      return nullptr;
    }

    Value *valGen = value->codegen(gen);
    // return gen->builder.CreateLoad(v, name.c_str());
    if (valGen == nullptr) {
      // we were not able to generate the error for this, the error should be
      // handled in  code gen but we are gonna log one eitherway to givem ore of
      // a stack trace
      logCodegenError("cannot generate RHS of variable assigment", gen,
                      IssueCode::ERROR_RHS_VARIABLE_ASSIGMENT);
      return nullptr;
    }
    return gen->builder.CreateStore(valGen, v);
  }

  // if the datatype is not know, it means we need to be able to understand that
  // from
  // the variable that has be pre-generated, so we try to extract that
  if (datatype == 0) {
    if (v->getAllocatedType()->getTypeID() == llvm::Type::FloatTyID) {
      datatype = Token::tok_float;
    } else {
      datatype = Token::tok_int;
    }
  }

  // now at this point ,we might have a simple variable for
  // which we gen a load, or we might have an assignment
  // if it is the case ,we have a value which is not nullptr
  if (value != nullptr) {
    // storing the result of RHS into LHS
    Value *valGen = value->codegen(gen);
    return gen->builder.CreateStore(valGen, v);
  }
  // otherwise we just generate the load
  return gen->builder.CreateLoad(v, name.c_str());
  // if we got here, it means the variable has a known datatype
  // but has not been defined yet, this only happens for variable
  // definitions, so we need to define it, we are gonna do that with
  // alloca
}
llvm::Value *BinaryExprAST::codegen(Codegenerator *gen) {
  // generating code recursively for left and right end side
  Value *L = lhs->codegen(gen);
  Value *R = rhs->codegen(gen);

  if (L == nullptr || R == nullptr) {
    return nullptr;
  }

  datatype = gen->omogenizeOperation(lhs, rhs, &L, &R);

  if (datatype == Token::tok_float) {
    // checking the operator to generate the correct operation
    if (op == "+") {
      return gen->builder.CreateFAdd(L, R, "addtmp");
    }
    if (op == "-") {
      return gen->builder.CreateFSub(L, R, "subtmp");
    }
    if (op == "*") {
      return gen->builder.CreateFMul(L, R, "multmp");
    }
    if (op == "/") {
      return gen->builder.CreateFDiv(L, R, "divtmp");
    }
    if (op == "<") {
      // TODO(giordi) fix this, to return int?
      L = gen->builder.CreateFCmpULT(L, R, "cmptmp");
      return gen->builder.CreateUIToFP(L, llvm::Type::getDoubleTy(gen->context),
                                       "booltmp");
    }
  } else {
    // checking the operator to generate the correct operation
    if (op == "+") {
      return gen->builder.CreateAdd(L, R, "addtmp");
    }
    if (op == "-") {
      return gen->builder.CreateSub(L, R, "subtmp");
    }
    if (op == "*") {
      return gen->builder.CreateMul(L, R, "multmp");
    }
    if (op == "/") {
      return gen->builder.CreateSDiv(L, R, "divtmp");
    }
    if (op == "<") {
      L = gen->builder.CreateICmpULT(L, R, "cmptmp");
      return L;
    }
  }
  std::cout << "error unrecognized operator" << std::endl;
  return nullptr;
}
llvm::Value *PrototypeAST::codegen(Codegenerator *gen) {
  uint32_t argSize = args.size();
  std::vector<llvm::Type *> funcArgs(argSize);
  // generating args with correct type
  for (uint32_t t = 0; t < argSize; ++t) {
    const auto &astArg = args[t];
    funcArgs[t] = getType(astArg.type, gen);
  }

  llvm::Type *returnType = getType(datatype, gen);
  auto *funcType = llvm::FunctionType::get(returnType, funcArgs, false);

  auto *function = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, name, gen->module.get());
  // Set names for all arguments.
  uint32_t Idx = 0;
  for (auto &arg : function->args()) {
    arg.setName(args[Idx++].name);
  }

  return function;
}
llvm::Value *FunctionAST::codegen(Codegenerator *gen) {
  // First, check for an existing function from a previous 'extern'
  // declaration.
  llvm::Function *function = gen->getFunction(proto->name);

  if (function == nullptr) {
    Value *p = proto->codegen(gen);
    // here we are forced to downcast to function from a value* we can't use
    // dynamic cast either since we have to compile with -no-rtti due to llvm
    // unless of course you re-compile llvm enabling rtti, there is no danger
    // this to
    // fail, because if that is not a function it should fail at parsing time.
    function = static_cast<llvm::Function *>(p);
    gen->functionProtos[proto->name] = proto;
    function = gen->getFunction(proto->name);
  }
  if (function == nullptr) {
    std::cout << "error generating prototype code gen" << std::endl;
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
  int counter = 0;
  for (auto &arg : function->args()) {
    // Create an alloca for this variable.
    llvm::AllocaInst *alloca = gen->createEntryBlockAlloca(
        function, arg.getName(), proto->args[counter++].type);

    // Store the initial value into the alloca.
    gen->builder.CreateStore(&arg, alloca);

    // Add arguments to variable symbol table.
    gen->namedValues[arg.getName()] = alloca;
  }

  gen->currentScope = function;
  for (auto &b : body) {
    if (Value *RetVal = b->codegen(gen)) {
      if (b->flags.isReturn) {
        gen->builder.CreateRet(RetVal);
      }
    }
  }
  gen->currentScope = nullptr;

  std::string outs;
  llvm::raw_string_ostream os(outs);
  bool res = verifyFunction(*function, &os);
  if (res) {
    os.flush();
    std::cout << "error verifying function" << outs << std::endl;
    std::cout << "here what was generated" << std::endl;
    // std::string outs;
    // llvm::raw_string_ostream os(outs);
    // gen->printLlvmData(function);
    gen->module->print(llvm::errs(), nullptr);
    return nullptr;
  }
  return function;
}
llvm::Value *CallExprAST::codegen(Codegenerator *gen) {
  // lets try to get the function
  llvm::Function *calleeF = gen->getFunction(callee);
  if (calleeF == nullptr) {
    std::cout << "error function not defined" << std::endl;
    return nullptr;
  }

  // checking function signature
  if (calleeF->arg_size() != args.size()) {
    std::cout << "error function call with wrong number of args" << std::endl;
    return nullptr;
  }
  uint32_t argSize = args.size();
  std::vector<Value *> argValues;
  argValues.reserve(argSize);
  for (uint32_t t = 0; t < argSize; ++t) {
    // check type
    llvm::Argument *currFunctionArg = calleeF->args().begin() + t;
    if (args[t]->datatype == 0) {
      // if the variable has no datatype it means we don't know what it is
      // the only option here is that is actually a variable is scope
      if (args[t]->nodetype == VariableNode) {
        auto temp = static_cast<VariableExprAST *>(args[t]);
        bool found =
            gen->namedValues.find(temp->name) != gen->namedValues.end();
        if (found) {
          datatype = Token::tok_int;
          auto type = gen->namedValues[temp->name]->getAllocatedType();
          if (type->isFloatTy()) {
            args[t]->datatype = Token::tok_float;
          }
        }
      }
    }
    if (!Codegenerator::compareASTArgWithLLVMArg(args[t], currFunctionArg)) {
      std::cout << "mismatch type for function call argument" << std::endl;
      return nullptr;
    }
    // if we got here the type is correct, so we can push the argument
    Value *argValuePtr = args[t]->codegen(gen);
    if (argValuePtr == nullptr) {
      std::cout << "error in generating code for function argument"
                << std::endl;
      return nullptr;
    }
    argValues.push_back(argValuePtr);
  }

  // setting datatype
  datatype = Token::tok_int;
  if (calleeF->getType()->isFloatTy()) {
    datatype = Token::tok_float;
  }

  return gen->builder.CreateCall(calleeF, argValues, "calltmp");
}

llvm::Value *IfAST::codegen(Codegenerator *gen) {
  Value *condValue = nullptr;
  if (condition == nullptr) {
    // in this case a condition was not provided we know it was an else
    // and we use a dummy condition of true
    condValue = llvm::ConstantInt::get(gen->context, llvm::APInt(32, 1, true));

  } else {
    condValue = condition->codegen(gen);
  }
  if (condValue == nullptr) {
    std::cout << "error : failed to generate if statement condition IR"
              << std::endl;
    return nullptr;
  }
  // at this point we need to figure out if is an int or
  // float and make the corresponding comparison
  Value *comparisonValue = nullptr;
  if (condValue->getType()->isFloatTy()) {

    // here we perform a zero comparison but hey, who am i to judge
    auto zeroConst = llvm::ConstantFP::get(gen->context, llvm::APFloat(0.0f));
    // ONE stands for ordered and not equal
    comparisonValue =
        gen->builder.CreateFCmpONE(condValue, zeroConst, "ifcond");
  } else if (condValue->getType()->isIntegerTy()) {

    auto zeroConst =
        llvm::ConstantInt::get(gen->context, llvm::APInt(32, 0, true));

    comparisonValue = gen->builder.CreateICmpNE(condValue, zeroConst, "ifcond");
    // resolving to int
  } else {
    std::cout << "error undefined type for if condition expr" << std::endl;
    return nullptr;
  }
  // at this point we have our value that will tell us which
  // branche we will take, so next we create the blocks for
  // both the if and else statment
  llvm::Function *theFunction = gen->builder.GetInsertBlock()->getParent();
  llvm::BasicBlock *thenBlock =
      llvm::BasicBlock::Create(gen->context, "then", theFunction);
  llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(gen->context, "else");
  llvm::BasicBlock *mergeBlock =
      llvm::BasicBlock::Create(gen->context, "merge");

  gen->builder.CreateCondBr(comparisonValue, thenBlock, elseBlock);
  // starting to work out the branch
  gen->builder.SetInsertPoint(thenBlock);

  Value *ifBranchValue = nullptr;
  for (auto &ifE : ifExpr) {
    ifBranchValue = ifE->codegen(gen);
    if (ifBranchValue == nullptr) {
      logCodegenError("Error in generating if branch code", gen,
                      IssueCode::BRANCH_CODE_FAILURE);
      return nullptr;
    }
  }
  // now that we inserted the then block we need to jump to the merge
  gen->builder.CreateBr(mergeBlock);

  // getting back up to date thenBlock
  thenBlock = gen->builder.GetInsertBlock();

  // now working on the else branch
  theFunction->getBasicBlockList().push_back(elseBlock);
  gen->builder.SetInsertPoint(elseBlock);

  if (elseExpr.size() != 0) {

    Value *elseBranchValue = nullptr;
    for (auto elseE : elseExpr) {
      elseBranchValue = elseE->codegen(gen);
      if (elseBranchValue == nullptr) {
        logCodegenError("Error in generating else branch code", gen,
                        IssueCode::BRANCH_CODE_FAILURE);
        return nullptr;
      }
    }
    gen->builder.CreateBr(mergeBlock);
  }
  // updating else block for phi node
  elseBlock = gen->builder.GetInsertBlock();

  // mergin the code
  theFunction->getBasicBlockList().push_back(mergeBlock);
  gen->builder.SetInsertPoint(mergeBlock);
  // we don't need a phi node since we handle everything with alloca

  return comparisonValue;
}

llvm::Value *ForAST::codegen(Codegenerator *gen) {
  // we start by  generating the starting condition
  Value *initValue = initialization->codegen(gen);
  if (initValue == nullptr) {
    logCodegenError("Error in generating initial condition for the for loop",
                    gen, IssueCode::FOR_LOOP_CODE_FAILURE);
    return nullptr;
  }
  llvm::Function *function = gen->builder.GetInsertBlock()->getParent();
  llvm::BasicBlock *LoopBB =
      llvm::BasicBlock::Create(gen->context, "loop", function);
  llvm::BasicBlock *AfterBB =
      llvm::BasicBlock::Create(gen->context, "afterloop", function);

  // here we generate the condition and we evaluate
  Value *conditionValue = condition->codegen(gen);
  // Insert the conditional branch into the end of LoopEndBB.
  // here we valuate the condition for the first time, if it valid we 
  // jump to the loop, otherwise we get out after the loop immediatly,
  // this handle gracefully the insertion from entry block to after
  // or loop block
  gen->builder.CreateCondBr(conditionValue, LoopBB, AfterBB);

  // Start insertion in LoopBB.
  gen->builder.SetInsertPoint(LoopBB);
  Value *bodyValue = nullptr;
  for (auto *s : body) {
    bodyValue = s->codegen(gen);
    if (bodyValue == nullptr) {
      logCodegenError("Error in body for the for loop", gen,
                      IssueCode::FOR_LOOP_CODE_FAILURE);
      return nullptr;
    }
  }

  // here we need to do the increment;
  Value *incrementValue = increment->codegen(gen);
  if (incrementValue == nullptr) {
    logCodegenError("Error in generating increment of the for loop", gen,
                    IssueCode::FOR_LOOP_CODE_FAILURE);
    return nullptr;
  }

  // here we need to perform the check on the condition
  conditionValue = condition->codegen(gen);
  // need to add the branch here

  // Create the "after loop" block and insert it.
  llvm::BasicBlock *LoopEndBB = gen->builder.GetInsertBlock();

  // Insert the conditional branch into the end of LoopEndBB.
  gen->builder.CreateCondBr(conditionValue, LoopEndBB, AfterBB);

  // Any new code will be inserted in AfterBB.
  gen->builder.SetInsertPoint(AfterBB);

  return conditionValue;
  // return nullptr;
}
} // namespace codegen
} // namespace babycpp
