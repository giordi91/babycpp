#include "catch.hpp"
#include <codegen.h>
#include <iostream>

using babycpp::codegen::Codegenerator;
using babycpp::lexer::Lexer;
using babycpp::lexer::Token;
using babycpp::parser::Parser;

// TODO(giordi) move this stuff in a shared header?
static void checkGenStructErrors(Codegenerator *gen) {
  if (gen->diagnostic.hasErrors()) {
    std::cout << gen->diagnostic.printAll() << std::endl;
  }
}

inline std::string getFileStruct(const ::std::string &path) {

  std::ifstream t(path);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
  return str;
}

TEST_CASE("Testing struct code gen", "[codegen]") {

  Codegenerator gen{true};
  gen.initFromString("struct Vector{ float x; int* y; float*z;float w;}");

  auto p = gen.parser.parseStruct();
  checkGenStructErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegenType(&gen);
  checkGenStructErrors(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  // gen.dumpLlvmData(v, "tests/core/vectorStruct.ll");
  auto expected = getFileStruct("tests/core/vectorStruct.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing struct member offset code gen", "[codegen]") {

  Codegenerator gen{true};
  gen.initFromString("struct Vector{ float x; int* y; float*z;float w;}");

  auto p = gen.parser.parseStruct();
  checkGenStructErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegenType(&gen);
  checkGenStructErrors(&gen);
  REQUIRE(v != nullptr);

  REQUIRE(p->byteSize == 24);
  REQUIRE(p->members[0]->biteOffset == 0);
  REQUIRE(p->members[1]->biteOffset == 4);
  REQUIRE(p->members[2]->biteOffset == 12);
  REQUIRE(p->members[3]->biteOffset == 20);
}

TEST_CASE("Testing struct instancing", "[codegen]") {

  Codegenerator gen{true};
  gen.initFromString(
      "struct Vector{ float x; int* y; float*z;float w;}"
      "int testFunc(int i) { Vector myStruct; int res = i; return res;}");

  auto p = gen.parser.parseStruct();
  checkGenStructErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegenType(&gen);
  checkGenStructErrors(&gen);
  REQUIRE(v != nullptr);

  auto func = gen.parser.parseFunction();
  checkGenStructErrors(&gen);
  REQUIRE(func != nullptr);

  auto funcv = func->codegen(&gen);
  checkGenStructErrors(&gen);
  REQUIRE(funcv != nullptr);

  std::string outs = gen.printLlvmData(funcv);
  // gen.dumpLlvmData(funcv, "tests/core/vectorStructAlloc.ll");
  auto expected = getFileStruct("tests/core/vectorStructAlloc.ll");
  REQUIRE(outs == expected);
}

TEST_CASE("Testing struct as return type codegen", "[codegen]") {

  Codegenerator gen{true};
  gen.initFromString(
      "struct Vector{ float x; int* y; float*z;float w;}"
      "Vector testFunc(int i) { Vector myStruct;  return myStruct;}");

  auto p = gen.parser.parseStruct();
  checkGenStructErrors(&gen);
  REQUIRE(p != nullptr);

  auto v = p->codegenType(&gen);
  checkGenStructErrors(&gen);
  REQUIRE(v != nullptr);

  auto func = gen.parser.parseFunction();
  checkGenStructErrors(&gen);
  REQUIRE(func != nullptr);

  auto funcv = func->codegen(&gen);
  checkGenStructErrors(&gen);
  REQUIRE(funcv != nullptr);

  std::string outs = gen.printLlvmData(funcv);
  //gen.dumpLlvmData(funcv, "tests/core/vectorStructReturnType.ll");
  auto expected = getFileStruct("tests/core/vectorStructReturnType.ll");
  REQUIRE(outs == expected);
}
