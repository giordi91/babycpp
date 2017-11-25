#include "catch.hpp"
#include <codegen.h>
#include <iostream>

using babycpp::codegen::Codegenerator;
using babycpp::lexer::Lexer;
using babycpp::lexer::Token;
using babycpp::parser::Parser;

static void checkGenErrors(Codegenerator *gen) {
  if (gen->diagnostic.hasErrors()) {
    std::cout << gen->diagnostic.printAll() << std::endl;
  }
}

inline std::string getFile(const ::std::string &path) {

  std::ifstream t(path);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
  return str;
}

TEST_CASE("Testing code gen number float", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("1.0");

  auto *node = gen.parser.parseNumber();
  REQUIRE(node != nullptr);

  auto *val = node->codegen(&gen);
  REQUIRE(val != nullptr);

  std::string outs = gen.printLlvmData(val);
  REQUIRE(outs == "float 1.000000e+00");
}
TEST_CASE("Testing code gen number int", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("39");

  auto *node = gen.parser.parseNumber();
  REQUIRE(node != nullptr);

  auto *val = node->codegen(&gen);
  REQUIRE(val != nullptr);

  std::string outs = gen.printLlvmData(val);
  REQUIRE(outs == "i32 39");
}
// TEST_CASE("Testing number variable ref gen", "[codegen]") {
//  Codegenerator gen;
//  gen.initFromString("x");
//
//  // adding dummy var
//  llvm::IRBuilder<> tmpb(gen.context);
//  std::vector<llvm::Type *> funcArgs;
//  auto *returnType = llvm::Type::getFloatTy(gen.context);
//  auto *funcType = llvm::FunctionType::get(returnType, funcArgs, false);
//  auto *function = llvm::Function::Create(
//      funcType, llvm::Function::ExternalLinkage, "debug", &gen.module);
//
//  llvm::BasicBlock *block =
//      llvm::BasicBlock::Create(gen.context, "entry", function);
//  tmpb.SetInsertPoint(block);
//  auto a = tmpb.CreateAlloca(llvm::Type::getFloatTy(gen.context), 0, "x");
//  gen.namedValues[a->getName().str()] = a;
//
//  // doing the parsing
//  auto *p = gen.parser.parseIdentifier();
//  REQUIRE(p != nullptr);
//  llvm::Value *v = p->codegen(&gen);
//  REQUIRE(v != nullptr);
//
// converting Value to string and check result
//  std::string outs = gen.printLlvmData(v);
//  REQUIRE(outs == "  %x = alloca float");
//}
TEST_CASE("Testing add constants", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(3+2)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);
  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  REQUIRE(outs == "i32 5");
}

TEST_CASE("Testing binop codegen plus", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(x+2.0)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);

  // adding dummy var
  // llvm::IRBuilder<> tmpb(gen.context);
  std::vector<llvm::Type *> funcArgs;
  auto *returnType = llvm::Type::getFloatTy(gen.context);
  auto *funcType = llvm::FunctionType::get(returnType, funcArgs, false);
  auto *function = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, "debug", gen.module.get());

  llvm::BasicBlock *block =
      llvm::BasicBlock::Create(gen.context, "entry", function);
  gen.builder.SetInsertPoint(block);
  auto a =
      gen.builder.CreateAlloca(llvm::Type::getFloatTy(gen.context), 0, "x");
  gen.namedValues[a->getName().str()] = a;

  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  REQUIRE(outs == "  %addtmp = fadd float %x1, 2.000000e+00");
}

TEST_CASE("Testing binop codegen -", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(yy-2.0)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);

  // adding dummy var
  std::vector<llvm::Type *> funcArgs;
  auto *returnType = llvm::Type::getFloatTy(gen.context);
  auto *funcType = llvm::FunctionType::get(returnType, funcArgs, false);
  auto *function = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, "debug", gen.module.get());

  llvm::BasicBlock *block =
      llvm::BasicBlock::Create(gen.context, "entry", function);
  gen.builder.SetInsertPoint(block);
  auto a =
      gen.builder.CreateAlloca(llvm::Type::getFloatTy(gen.context), 0, "yy");
  gen.namedValues[a->getName().str()] = a;

  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  REQUIRE(outs == "  %subtmp = fsub float %yy1, 2.000000e+00");
}

TEST_CASE("Testing binop codegen *", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(temp*4.0)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);

  // adding dummy var
  std::vector<llvm::Type *> funcArgs;
  auto *returnType = llvm::Type::getFloatTy(gen.context);
  auto *funcType = llvm::FunctionType::get(returnType, funcArgs, false);
  auto *function = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, "debug", gen.module.get());

  llvm::BasicBlock *block =
      llvm::BasicBlock::Create(gen.context, "entry", function);
  gen.builder.SetInsertPoint(block);
  auto a =
      gen.builder.CreateAlloca(llvm::Type::getFloatTy(gen.context), 0, "temp");
  gen.namedValues[a->getName().str()] = a;

  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  REQUIRE(outs == "  %multmp = fmul float %temp1, 4.000000e+00");
}

TEST_CASE("Testing binop codegen /", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(temp /10.0)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);

  // adding dummy var
  std::vector<llvm::Type *> funcArgs;
  auto *returnType = llvm::Type::getFloatTy(gen.context);
  auto *funcType = llvm::FunctionType::get(returnType, funcArgs, false);
  auto *function = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, "debug", gen.module.get());

  llvm::BasicBlock *block =
      llvm::BasicBlock::Create(gen.context, "entry", function);
  gen.builder.SetInsertPoint(block);
  auto a =
      gen.builder.CreateAlloca(llvm::Type::getFloatTy(gen.context), 0, "temp");
  gen.namedValues[a->getName().str()] = a;

  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  REQUIRE(outs == "  %divtmp = fdiv float %temp1, 1.000000e+01");
}

/*
//TODO (giordi) fix comparison
TEST_CASE("Testing binop codegen <", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(z<13.0)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);

  // adding dummy var
  llvm::IRBuilder<> tmpb(gen.context);
  std::vector<llvm::Type *> funcArgs;
  auto *returnType = llvm::Type::getFloatTy(gen.context);
  auto *funcType = llvm::FunctionType::get(returnType, funcArgs, false);
  auto *function = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, "debug", &gen.module);

  llvm::BasicBlock *block =
      llvm::BasicBlock::Create(gen.context, "entry", function);
  tmpb.SetInsertPoint(block);
  auto a = tmpb.CreateAlloca(llvm::Type::getFloatTy(gen.context), 0, "z");
  gen.namedValues[a->getName().str()] = a;

  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  // here we are only checking the last result, where we convert from bool to
  // double  since we only support doubles, when we will have a function the
  // whole body  will be there
  REQUIRE(outs == "  %booltmp = uitofp i1 %cmptmp to double");
}
*/

TEST_CASE("Testing function codegen simple add", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("float test(float x){ return x+1.0;}");
  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "simpleAdd.ll");

  // TODO I am trying to return a float* i should store in a
  // value and return???
  std::string expected = getFile("tests/core/simpleAdd.ll");
  REQUIRE(outs == expected);
}
TEST_CASE("Testing function codegen conversion", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("float complexAdd(float x, int y){ return x+y;}");
  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "complexAdd.ll");
  std::string expected = getFile("tests/core/complexAdd.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing function codegen alloca", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("float complexAdd(float x){ float temp = x * 2.0; temp = "
                     "x - 2.0; return temp;}");
  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/alloca1.ll");
  std::string expected = getFile("tests/core/alloca1.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing function call in function", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("float avg(float x){ return x *2.0;}"
                     "float test(){return 2.0 + avg(2.0); }");
  auto p = gen.parser.parseFunction();
  auto p2 = gen.parser.parseFunction();
  REQUIRE(p != nullptr);
  REQUIRE(p2 != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/avg.ll");
  std::string expected = getFile("tests/core/avg.ll");

  auto v2 = p2->codegen(&gen);
  REQUIRE(v2 != nullptr);
  outs = gen.printLlvmData(v2);
  // gen.dumpLlvmData(v2, "tests/core/callInFunc.ll");
  expected = getFile("tests/core/callInFunc.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing function call in function2", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("float avg(float x){return x*2.0;}"
                     "float tt(float x , float y){return y + avg(x);}");
  auto p = gen.parser.parseFunction();
  auto p2 = gen.parser.parseFunction();
  REQUIRE(p != nullptr);
  REQUIRE(p2 != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);

  auto v2 = p2->codegen(&gen);
  REQUIRE(v2 != nullptr);
  outs = gen.printLlvmData(v2);
}

TEST_CASE("Testing if statement code gen", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("if ( 3 + 1) { int x = 1 +1 ;}else{ int x = 2 + 2;}");

  std::vector<llvm::Type *> funcArgs;
  auto *returnType = llvm::Type::getFloatTy(gen.context);
  auto *funcType = llvm::FunctionType::get(returnType, funcArgs, false);
  auto *function = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, "debug", gen.module.get());

  llvm::BasicBlock *block =
      llvm::BasicBlock::Create(gen.context, "entry", function);
  gen.builder.SetInsertPoint(block);
  gen.currentScope = function;

  auto p = gen.parser.parseIfStatement();
  REQUIRE(p != nullptr);
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(function);
  // gen.dumpLlvmData(function, "tests/core/basicIfStatement.ll");
  auto expected = getFile("tests/core/basicIfStatement.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing simple if function code gen", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("int testFunc(int inv){int res = 0;if(inv){res = "
                     "10;}else{res= 2;} return res;}");
  auto p = gen.parser.parseStatement();
  REQUIRE(p != nullptr);
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/functionWithIfstatement.ll");
  auto expected = getFile("tests/core/functionWithIfstatement.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing more complex if function code gen", "[codegen]") {
  Codegenerator gen;
  gen.initFromString(
      "int testFunc(int a, int b){int res = 0;if(a - (3*b)){res = "
      "a+ 10;}else{res= 2 - b;} return res;}");
  auto p = gen.parser.parseStatement();
  REQUIRE(p != nullptr);
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/functionWithIfstatement2.ll");
  auto expected = getFile("tests/core/functionWithIfstatement2.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing more complex if function with multi statement code gen",
          "[codegen]") {
  Codegenerator gen;
  gen.initFromString("int testFunc(int a, int b, int c, int k){"
                     "int res = 0;"
                     "if(a *2){ "
                     "int x = k +1;"
                     "res = a+ x;"
                     "}else{"
                     "int x = c - 1;"
                     "res= x - b;"
                     "}"
                     "return res;}");
  auto p = gen.parser.parseStatement();
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  gen.dumpLlvmData(v, "tests/core/functionWithIfstatement3.ll");
  auto expected = getFile("tests/core/functionWithIfstatement3.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing for loop gen", "[codegen]") {
  Codegenerator gen;
  gen.initFromString(" int testFunc(int a){"
                     "int x = 0; "
                     "for ( int i = 0; i < a ; i= i+1){ "
                     "x = x + i;}"
                     " return x;}");
  auto p = gen.parser.parseStatement();
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/forLoop1.ll");
  auto expected = getFile("tests/core/forLoop1.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing pointer value dereference code gen", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("float testFunc(float* a){ float res = *a;return res;};");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/floatDereferenceFunction.ll");
  auto expected = getFile("tests/core/floatDereferenceFunction.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing pointer value dereference int code gen", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("int testFunc(int* a, int* b){ int res = 0; int aValue = "
                     "*a; int bValue = *b; res = aValue + bValue; return "
                     "res;};");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/intDereferenceFunction.ll");
  auto expected = getFile("tests/core/intDereferenceFunction.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing pointer value writing to code gen", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("int testFunc(int* a){ *a = 10; int x  =0; return x;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/intDereferenceForWritingPointer.ll");
  auto expected = getFile("tests/core/intDereferenceForWritingPointer.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing less operator codegen", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("int testFunc(int a, int b){ int res = 0; if(a < b){res = "
                     "1;}else{res =2;}return res;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/implicitBoolSupportInComparison.ll");
  auto expected = getFile("tests/core/implicitBoolSupportInComparison.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing float pointer as return in protoype is correct codegen ",
          "[codegen]") {

  Codegenerator gen;
  gen.initFromString("float* testFunc(float* a){ float* res = a; return res;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  gen.dumpLlvmData(v, "tests/core/floatPointerReturnTypeCorrect.ll");
  auto expected = getFile("tests/core/floatPointerReturnTypeCorrect.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing float pointer assigment null codegen ", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("float* testFunc(){ float* res = nullptr; return res;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  gen.dumpLlvmData(v, "tests/core/floatPointerAssignNull.ll");
  auto expected = getFile("tests/core/floatPointerAssignNull.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing int pointer assigment null codegen ", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("int* testFunc(){ int* res = nullptr; return res;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/intPointerAssignNull.ll");
  auto expected = getFile("tests/core/intPointerAssignNull.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing int pointer assigment null to var not declaration codegen ",
          "[codegen]") {

  Codegenerator gen;
  gen.initFromString(
      "int* testFunc(int* b){ int* res = b;res = nullptr; return res;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/intPointerAssignNullInNotDeclaration.ll");
  auto expected = getFile("tests/core/intPointerAssignNullInNotDeclaration.ll");
  REQUIRE(outs == expected);
}

TEST_CASE(
    "Testing float pointer assigment null to var not declaration codegen ",
    "[codegen]") {

  Codegenerator gen;
  gen.initFromString(
      "float* testFunc(float* b){ float* res = b;res = nullptr; return res;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v,
  // "tests/core/floatPointerAssignNullInNotDeclaration.ll");
  auto expected =
      getFile("tests/core/floatPointerAssignNullInNotDeclaration.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing pointer cast int to float", "[codegen]") {

  Codegenerator gen;
  gen.initFromString(
      "float* testFunc(int *b){ float* res = (float*)b; return res;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/intToFloatPointerCast.ll");
  auto expected = getFile("tests/core/intToFloatPointerCast.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing pointer cast float to int", "[codegen]") {

  Codegenerator gen;
  gen.initFromString(
      "int* testFunc(float *b){ int* res = (int*)b; return res;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/floatToIntPointerCast.ll");
  auto expected = getFile("tests/core/floatToIntPointerCast.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing pointer cast float to void", "[codegen]") {

  Codegenerator gen;
  gen.initFromString(
      "void* testFunc(float *b){ void* res = (void*)b; return res;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/floatToVoidPointerCast.ll");
  auto expected = getFile("tests/core/floatToVoidPointerCast.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing pointer cast void* to int", "[codegen]") {

  Codegenerator gen;
  gen.initFromString(
      "int* testFunc(void* b){ int* res = (int*)b; return res;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/voidToIntPointerCast.ll");
  auto expected = getFile("tests/core/voidToIntPointerCast.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing malloc call with no loaded builtins", "[codegen]") {

  Codegenerator gen;
  gen.initFromString(
      "int* testFunc(){ int* ptr = (int*) malloc(20); return ptr;}");

  auto p = gen.parser.parseStatement();
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v == nullptr);
  REQUIRE(gen.diagnostic.hasErrors() == 4);

  auto err1 = gen.diagnostic.getError();
  REQUIRE(err1.code == babycpp::diagnostic::IssueCode::UNDEFINED_FUNCTION);
}

TEST_CASE("Testing malloc with wrong arguments", "[codegen]") {

  Codegenerator gen{true};
  gen.initFromString(
      "int* testFunc(){ int* ptr = (int*) malloc(20,20); return ptr;}");

  auto p = gen.parser.parseStatement();
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v == nullptr);
  REQUIRE(gen.diagnostic.hasErrors() == 4);

  auto err1 = gen.diagnostic.getError();
  REQUIRE(err1.code == babycpp::diagnostic::IssueCode::WRONG_ARGUMENTS_COUNT_IN_FUNC_CALL);
}

TEST_CASE("Testing malloc call", "[codegen]") {

  Codegenerator gen{true};
  gen.initFromString(
      "int* testFunc(){ int* ptr = (int*) malloc(20); return ptr;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/mallocTest.ll");
  auto expected = getFile("tests/core/mallocTest.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing free call", "[codegen]") {

  Codegenerator gen{true};
  gen.initFromString(
      "int freeWrap(float* ptr){ void* toFree = (void*)ptr; free(toFree);int ret = 0; return ret;}");

  auto p = gen.parser.parseStatement();
  checkGenErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  checkGenErrors(&gen);
  REQUIRE(v != nullptr);

  std::string outs = gen.printLlvmData(v);
   gen.dumpLlvmData(v, "tests/core/freeTest.ll");
  auto expected = getFile("tests/core/freeTest.ll");
  REQUIRE(outs == expected);
}

// TODO(giordi) test concatenated casts, not really useful but let see what
// happen should  hold, something like (float*)(void*)myPyt;  not sure if double
// parent is working back to back
