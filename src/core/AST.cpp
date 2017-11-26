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
  llvm::AllocaInst *v = nullptr;
  auto found = gen->namedValues.find(name);
  if (found != gen->namedValues.end()) {
    v = found->second;
    auto &storeDatatype = gen->variableTypes[name];
    datatype = storeDatatype.datatype;
    flags.isPointer = storeDatatype.isPointer;
    flags.isNull = storeDatatype.isNull;
  };
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
    llvm::Type *varType = getType(datatype, gen, flags.isPointer);
    v = tempBuilder.CreateAlloca(varType, nullptr, name);
    gen->namedValues[name] = v;
    gen->variableTypes[name] = {datatype, flags.isPointer, flags.isNull};

    if (value == nullptr) {
      std::cout << "error: expected value for value definition" << std::endl;
      return nullptr;
    }

    Value *valGen;
    if (value->flags.isPointer && value->flags.isNull) {
      // in the case that the RHS is a nullpointer we are going to generate the
      // instruction directly
      // TODO(giordi) this is not that pretty we are bypassing a node, what I
      // think might be better is to  have the assigment operator feed the
      // datatype if known
      valGen = llvm::ConstantPointerNull::get(
          llvm::PointerType::get(getType(datatype, gen, false), 0));
    } else {
      valGen = value->codegen(gen);
    }
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
  // from the variable that has be pre-generated, so we try to extract that
  if (datatype == 0) {
    auto currType = v->getAllocatedType()->getTypeID();
    if (currType == llvm::Type::FloatTyID) {
      datatype = Token::tok_float;
    } else if (currType == llvm::Type::IntegerTyID) {
      datatype = Token::tok_int;
    } else if (v->getType()->isPointerTy()) {

      // TODO(giordi) I really don't like that, I need to start having a proper
      // datatype around I can use and rely on FIX THIS WHEN POSSIBLE
      if (gen->variableTypes.find(name) != gen->variableTypes.end()) {
        auto currPtrType = gen->variableTypes[name];
        datatype = currPtrType.datatype;
        flags.isPointer = currPtrType.isPointer;
        flags.isNull = currPtrType.isNull;
      }
      // TODO(giordi) investigate what happens when adding else branch with
      // error, couple of tests fails else {
      //  logCodegenError("cannot deduce variable datatype", gen,
      //                  IssueCode::ERROR_RHS_VARIABLE_ASSIGMENT);
      //  return nullptr;
      //}
    }
  }

  // now at this point ,we might have a simple variable for
  // which we gen a load, or we might have an assignment
  // if it is the case ,we have a value which is not nullptr
  if (value != nullptr) {
    Value *valGen;
    if (value->flags.isPointer && value->flags.isNull) {
      // in the case that the RHS is a nullpointer we are going to generate
      // the instruction directly
      // TODO(giordi) this is not that pretty we are bypassing a node, what I
      // think might be better is to  have the assigment operator feed the
      // datatype if known
      valGen = llvm::ConstantPointerNull::get(
          llvm::PointerType::get(getType(datatype, gen, false), 0));
    } else {
      // storing the result of RHS into LHS
      valGen = value->codegen(gen);
    }
    return gen->builder.CreateStore(valGen, v);
  }
  // otherwise we just generate the load
  return gen->builder.CreateLoad(v, name.c_str());
}

Value *handleBinOpSimpleDatatype(BinaryExprAST *bin, Codegenerator *gen,
                                 llvm::Value *L, llvm::Value *R) {
  bin->datatype = gen->omogenizeOperation(bin->lhs, bin->rhs, &L, &R);

  if (bin->datatype == Token::tok_float) {
    // checking the operator to generate the correct operation
    if (bin->op == "+") {
      return gen->builder.CreateFAdd(L, R, "addtmp");
    }
    if (bin->op == "-") {
      return gen->builder.CreateFSub(L, R, "subtmp");
    }
    if (bin->op == "*") {
      return gen->builder.CreateFMul(L, R, "multmp");
    }
    if (bin->op == "/") {
      return gen->builder.CreateFDiv(L, R, "divtmp");
    }
    if (bin->op == "<") {
      // TODO(giordi) fix this, to return int?
      L = gen->builder.CreateFCmpULT(L, R, "cmptmp");
      return gen->builder.CreateUIToFP(L, llvm::Type::getDoubleTy(gen->context),
                                       "booltmp");
    }
  } else {
    // checking the operator to generate the correct operation
    if (bin->op == "+") {
      return gen->builder.CreateAdd(L, R, "addtmp");
    }
    if (bin->op == "-") {
      return gen->builder.CreateSub(L, R, "subtmp");
    }
    if (bin->op == "*") {
      return gen->builder.CreateMul(L, R, "multmp");
    }
    if (bin->op == "/") {
      return gen->builder.CreateSDiv(L, R, "divtmp");
    }
    if (bin->op == "<") {
      L = gen->builder.CreateICmpULT(L, R, "cmptmp");
      return L;
    }
  }
  return nullptr;
}

llvm::Value *BinaryExprAST::codegen(Codegenerator *gen) {
  // generating code recursively for left and right end side
  Value *L = lhs->codegen(gen);
  Value *R = rhs->codegen(gen);

  if (L == nullptr || R == nullptr) {
    return nullptr;
  }

  if (lhs->flags.isPointer || rhs->flags.isPointer) {

    // TODO(giordi) add support for < > in pointer comparison?
    if (rhs->flags.isPointer) {
      logCodegenError("unsupporter pointer at rhs in bin operation", gen,
                      IssueCode::POINTER_ARITHMETIC_ERROR);
      return nullptr;
    }
    if (rhs->datatype != Token::tok_int) {
      logCodegenError("unsupporter datatype at RHS of pointer arithmetic", gen,
                      IssueCode::POINTER_ARITHMETIC_ERROR);
      return nullptr;
    }
    if (op != "+") {
      logCodegenError(
          "unsupporter operator for pointer arithmetic only valid is +, got: " +
              op,
          gen, IssueCode::POINTER_ARITHMETIC_ERROR);
      return nullptr;
    }

    // at this point we should have a pointer at lhs and an int rhs  we should
    // be able to perform  math with it, only operator supported is + the time
    // being
    Value *indexList[1] = {R};
    return gen->builder.CreateGEP(L, llvm::ArrayRef<Value *>(indexList, 1),
                                  "pointerShift");
  }

  Value *result = handleBinOpSimpleDatatype(this, gen, L, R);
  if (!result) {
    logCodegenError("error unrecognized operator", gen,
                    IssueCode::UNKNOWN_BIN_OPERATOR);
    return nullptr;
  }
  return result;
}

llvm::Value *PrototypeAST::codegen(Codegenerator *gen) {
  uint32_t argSize = args.size();
  std::vector<llvm::Type *> funcArgs(argSize);
  // generating args with correct type
  for (uint32_t t = 0; t < argSize; ++t) {
    const auto &astArg = args[t];
    funcArgs[t] = getType(astArg.type, gen, astArg.isPointer);
  }

  llvm::Type *returnType = nullptr;
  if (flags.isNull && !flags.isPointer) {
    returnType = gen->builder.getVoidTy();
  } else {
    returnType = getType(datatype, gen, flags.isPointer);
  }
  auto *funcType = llvm::FunctionType::get(returnType, funcArgs, false);

  auto *function = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, name, gen->module.get());
  // Set names for all arguments.
  uint32_t Idx = 0;
  for (auto &arg : function->args()) {
    arg.setName(args[Idx++].name);
  }

  // if the function is an extern is going to be stand alone in the body of a
  // function  or somewhere, noramlly is the function itself that takes care
  // of adding it to the function protos but in this case we need to do it
  // manually
  if (isExtern) {
    if (function != nullptr) {
      gen->functionProtos[name] = this;
    }
  }

  return function;
}
llvm::Value *FunctionAST::codegen(Codegenerator *gen) {
  // First, check for an existing function from a previous 'extern'
  // declaration.
  llvm::Function *function = gen->getFunction(proto->name);

  if (function == nullptr) {
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
  gen->variableTypes.clear();
  int counter = 0;
  for (auto &arg : function->args()) {
    // Create an alloca for this variable.
    // TODO(giordi) clean this to pass in the arg directly
    llvm::AllocaInst *alloca = gen->createEntryBlockAlloca(
        function, arg.getName(), proto->args[counter].type,
        proto->args[counter].isPointer);

    // Store the initial value into the alloca.
    gen->builder.CreateStore(&arg, alloca);

    // Add arguments to variable symbol table.
    gen->namedValues[arg.getName()] = alloca;
    gen->variableTypes[arg.getName()] = {
        proto->args[counter].type, proto->args[counter].isPointer,
        false}; // TODO(giordi) should pass argumetn as not null? we don't
                // supprot  default values so can't be nullptr
    counter += 1;
  }

  bool hasTerminator = false;
  gen->currentScope = function;
  for (auto &b : body) {
    if (Value *RetVal = b->codegen(gen)) {
      if (b->flags.isReturn) {
        gen->builder.CreateRet(RetVal);
        hasTerminator = true;
      }
    } else {
      logCodegenError("error generating body statement for function", gen,
                      IssueCode::ERROR_IN_FUNCTION_BODY);
      return nullptr;
    }
  }
  flags.isNull = proto->flags.isNull;
  flags.isPointer = proto->flags.isPointer;
  datatype = proto->datatype;

  // if the function has return void then we can create it
  if (!hasTerminator && flags.isNull && !flags.isPointer) {
    gen->builder.CreateRetVoid();
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
  PrototypeAST *proto = nullptr;
  llvm::Function *calleeF = gen->getFunction(callee, &proto);
  if (calleeF == nullptr) {
    logCodegenError("error getting function, either is not defined or builtin "
                    "functions have not been loaded",
                    gen, IssueCode::UNDEFINED_FUNCTION);
    return nullptr;
  }

  // if the function returns a pointer we mark teh call expression as a
  // pointer type
  if (calleeF->getReturnType()->isPointerTy()) {
    flags.isPointer = true;
  }

  // checking function signature
  if (calleeF->arg_size() != args.size()) {
    logCodegenError("error function called with wrong number of arguments", gen,
                    IssueCode::WRONG_ARGUMENTS_COUNT_IN_FUNC_CALL);
    return nullptr;
  }
  uint32_t argSize = args.size();
  std::vector<Value *> argValues;
  argValues.reserve(argSize);
  for (uint32_t t = 0; t < argSize; ++t) {
    // check type
    if (args[t]->datatype == 0) {
      // if the variable has no datatype it means we don't know what it is
      // the only option here is that is actually a variable is scope
      if (args[t]->nodetype == VariableNode) {
        auto temp = static_cast<VariableExprAST *>(args[t]);
        bool found =
            gen->namedValues.find(temp->name) != gen->namedValues.end();
        if (found) {
          auto &currDatatype = gen->variableTypes[temp->name];
          temp->datatype = currDatatype.datatype;
          temp->flags.isPointer = currDatatype.isPointer;
          temp->flags.isNull = currDatatype.isNull;
          // datatype = Token::tok_int;
          // auto type = gen->namedValues[temp->name]->getAllocatedType();
          // if (type->isFloatTy()) {
          //  args[t]->datatype = Token::tok_float;
          //}
        }
      }
    }
    if (proto == nullptr) {

      std::cout << "cannot check function arguments " << callee << std::endl;
    } else {
      if (proto->args[t].type != args[t]->datatype ||
          proto->args[t].isPointer != args[t]->flags.isPointer) {
        std::cout << "mismatch type for function call argument" << std::endl;
      }
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

  if (proto == nullptr) {
    std::cout << "cannot get return type properly" << std::endl;
    return nullptr;
  }

  // copy datatype from proto to function call
  datatype = proto->datatype;
  flags.isPointer = proto->flags.isPointer;
  flags.isNull = proto->flags.isNull;

  // if we are a void call we don't pass a name so we don't store to a
  // register
  if (flags.isNull && !flags.isPointer) {
    return gen->builder.CreateCall(calleeF, argValues);
  } else {
    return gen->builder.CreateCall(calleeF, argValues, "calltmp");
  }
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

    // extracting the type size to make the comparison correct
    int typeSize = condValue->getType()->getPrimitiveSizeInBits();
    auto zeroConst =
        llvm::ConstantInt::get(gen->context, llvm::APInt(typeSize, 0, true));

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

  // now working on the else branch
  theFunction->getBasicBlockList().push_back(elseBlock);
  gen->builder.SetInsertPoint(elseBlock);

  if (!elseExpr.empty()) {

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

  // merging the code
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
llvm::Value *DereferenceAST::codegen(Codegenerator *gen) {

  // first we try to see if the variable is already defined at scope
  // level
  llvm::AllocaInst *v = gen->namedValues[identifierName];
  // here we extract the variable from the scope.
  // if we get a nullptr and the variable is not a definition
  // we got an error
  if (v == nullptr && datatype == 0) {
    logCodegenError("Error variable " + identifierName + "is not defined", gen,
                    IssueCode::UNDEFINED_VARIABLE);
    return nullptr;
  }

  if (flags.isDefinition) {

    logCodegenError("Error variable " + identifierName +
                        "is a definition cannot be dereferenced",
                    gen, IssueCode::EXPECTED_POINTER);
    return nullptr;
  }

  // if the datatype is not know, it means we need to be able to understand
  // that from the variable that has be pre-generated, so we try to extract
  // that
  if (datatype == 0) {
    if (v->getAllocatedType()->getTypeID() == llvm::Type::FloatTyID) {
      datatype = Token::tok_float;
    } else {
      datatype = Token::tok_int;
    }
  }

  // here we first load the pointer to a register and then we load from that
  // pointer,  this hields a double load
  Value *ptrLoaded = gen->builder.CreateLoad(v, identifierName.c_str());
  // now we loaded the pointer, what we are going to do is load from the
  // pionter
  return gen->builder.CreateLoad(ptrLoaded,
                                 (identifierName + "Dereferenced").c_str());
}

llvm::Value *ToPointerAssigmentAST::codegen(Codegenerator *gen) {

  // to dereference a pointer it means it must be defined already
  llvm::AllocaInst *v = gen->namedValues[identifierName];
  if (v == nullptr) {
    logCodegenError("Error pointer variable" + identifierName +
                        "is not defined",
                    gen, IssueCode::UNDEFINED_VARIABLE);
    return nullptr;
  }
  // if we got here it means we have a pointer we can write to so we first
  // generate the value for the  RHS the we write to it using the pointer
  Value *rhsValue = rhs->codegen(gen);
  if (rhsValue == nullptr) {
    logCodegenError("error in generating rhs for pointer assigment", gen,
                    IssueCode::ERROR_RHS_VARIABLE_ASSIGMENT);
    return nullptr;
  }
  // updating the datatype
  if (datatype == 0) {
    auto &currDatatype = gen->variableTypes[identifierName];
    datatype = currDatatype.datatype;
    flags.isPointer = currDatatype.isPointer;
    flags.isNull = currDatatype.isNull;
  }

  // TODO(giordi) this won't work in the future for custom datatypes like
  // structs, will need a fat  datatype a int won't cut anymore probably
  // lets compare the datatype with what we want to assign
  if (rhs->datatype != datatype) {
    logCodegenError(
        "mismatch datatype assigment got: " + std::to_string(datatype) +
            " on LHS and got: " + std::to_string(rhs->datatype) + " on RHS",
        gen, IssueCode::ERROR_RHS_VARIABLE_ASSIGMENT);
    return nullptr;
  }

  // we can now proceed with the store
  Value *ptrLoaded =
      gen->builder.CreateLoad(v, (identifierName + "Dereferenced").c_str());
  return gen->builder.CreateStore(rhsValue, ptrLoaded);
}

llvm::Value *CastAST::codegen(Codegenerator *gen) {
  // here we need to use a bitcast operation
  Value *rhsValue = rhs->codegen(gen);
  if (rhsValue == nullptr) {
    logCodegenError("error in pointer cast RHS code generation", gen,
                    IssueCode::CAST_ERROR);

    return nullptr;
  }
  if (rhs->flags.isPointer == false && flags.isPointer) {
    logCodegenError("mismatch datatype for casting, rhs is not a pointer", gen,
                    IssueCode::CAST_ERROR);
  }

  Value *cast = nullptr;
  // we can do the cast
  if (flags.isPointer) {
    cast = gen->builder.CreateBitCast(rhsValue, getType(datatype, gen, true),
                                      "pointerCast");
  }

  return cast;
}

llvm::Value *StructMemberAST::codegen(Codegenerator *gen) {
  logCodegenError("error struct member should not use codgen but codgenType",
                  gen, IssueCode::STRUCT_MEMBER_ERROR);
  return nullptr;
}

llvm::Type *StructMemberAST::codegenType(Codegenerator *gen) {
  return getType(datatype, gen, flags.isPointer);
}

llvm::Value *StructAST::codegen(Codegenerator *gen) {
  logCodegenError("error struct member should not use codgen but codgenType",
                  gen, IssueCode::STRUCT_MEMBER_ERROR);
  return nullptr;
}

int StructMemberAST::getTypeSize() {
  if (flags.isPointer) {
    return 8;
  }
  // TODO(giordi) handle having a struct as member type
  return 4;
}
llvm::Type *StructAST::codegenType(Codegenerator *gen) {

  // lets loop all the members to get the type
  std::vector<llvm::Type *> memberValues;
  int globalOffset = 0;
  for (auto &member : members) {
    llvm::Type *currV = member->codegenType(gen);
    if (currV == nullptr) {
      logCodegenError("error in generating struct member", gen,
                      IssueCode::STRUCT_MEMBER_ERROR);
      return nullptr;
    }
    memberValues.push_back(currV);
    member->biteOffset = globalOffset;
    int currOffset = member->getTypeSize();
    if (currOffset == -1) {
      logCodegenError("error in computing member type size", gen,
                      IssueCode::STRUCT_MEMBER_ERROR);
      return nullptr;
    }
    globalOffset += currOffset;
  }
  // setting the struct size, which should now be the global offset
  byteSize = globalOffset;
  // if we got here we have all the members to create our struct

  llvm::StructType *structDefinition = llvm::StructType::create(
      gen->context,
      llvm::ArrayRef<llvm::Type *>(memberValues.data(), memberValues.size()),
      identifierName, false);

  StructDefinition def{this, structDefinition};
  gen->addCustomStruct(identifierName, def);
  return structDefinition;
}

llvm::Value *StructInstanceAST::codegen(Codegenerator *gen) {

	auto customStruct = gen->customStructs.find(structType);
	if( customStruct == gen->customStructs.end())
	{
      logCodegenError("struct is not defined: "+structType, gen,
                      IssueCode::UNDEFINED_STRUCT);
      return nullptr;
	}

    auto v = gen->builder.CreateAlloca(customStruct->second.type, nullptr, identifierName);
	return v;


}
} // namespace codegen
} // namespace babycpp
