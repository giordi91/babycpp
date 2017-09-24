#include "catch.hpp"
#include <codegen.h>
#include <iostream>

using babycpp::codegen::Codegenerator;
using babycpp::lexer::Lexer;
using babycpp::lexer::Token;
using babycpp::parser::Parser;

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
  std::cout << outs << std::endl;
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
  //std::cout << outs << std::endl;

  auto v2 = p2->codegen(&gen);
  REQUIRE(v2 != nullptr);
  outs = gen.printLlvmData(v2);
  //std::cout << outs << std::endl;
  // gen.dumpLlvmData(v, "tests/core/alloca1.ll");
  // std::string expected = getFile("tests/core/alloca1.ll");
  // REQUIRE(outs == expected);
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
  std::cout << outs << std::endl;

  auto v2 = p2->codegen(&gen);
  REQUIRE(v2 != nullptr);
  outs = gen.printLlvmData(v2);
  std::cout << outs << std::endl;
}
