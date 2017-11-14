// this define is to disable the config main so that we can build all the test
// in a single executable
#ifndef CUMULATIVE_TESTS
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
#endif
// in one cpp file
#include "catch.hpp"

#include <codegen.h>
#include <jit.h>

inline std::string getFile(const ::std::string &path) {

  std::ifstream t(path);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
  return str;
}
using babycpp::codegen::Codegenerator;

TEST_CASE("Testing jit add", "[jit]") {

  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString("float test(float x){ return x+1.0;}");
  // making sure parsing went fine
  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  // making sure the code is generated
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  jit.addModule(gen.module);
  auto symbol = jit.findSymbol("test");
  auto func = (float (*)(float))(intptr_t)llvm::cantFail(symbol.getAddress());
  REQUIRE(func(10) == 11);
}

TEST_CASE("Testing jit complex  add", "[jit]") {

  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString("float complexAdd(float x, int y){ return x+y;}");
  // making sure parsing went fine
  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  // making sure the code is generated
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  jit.addModule(gen.module);
  auto symbol = jit.findSymbol("complexAdd");
  auto func =
      (float (*)(float, int))(intptr_t)llvm::cantFail(symbol.getAddress());
  REQUIRE(func(10.0f, 5) == Approx(15.0f));
}

TEST_CASE("Testing jit alloca", "[jit]") {

  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString("float allocaTest(float x){ float temp = x * 2.0; temp = "
                     "x - 2.0; return temp;}");
  // making sure parsing went fine
  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  // making sure the code is generated
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  jit.addModule(gen.module);
  auto symbol = jit.findSymbol("allocaTest");
  auto func = (float (*)(float))(intptr_t)llvm::cantFail(symbol.getAddress());
  REQUIRE(func(10.0f) == Approx(8.0f));
}

TEST_CASE("Testing jit if statement", "[jit]") {

  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString("int testFunc(int inv){int res = 0;if(inv){res = "
                     "10;}else{res= 2;} return res;}");
  // making sure parsing went fine
  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  // making sure the code is generated
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  jit.addModule(gen.module);
  auto symbol = jit.findSymbol("testFunc");
  auto func = (int (*)(int))(intptr_t)llvm::cantFail(symbol.getAddress());
  REQUIRE(func(2) == 10);
  REQUIRE(func(0) == 2);
  REQUIRE(func(2 + 3) == 10);
  REQUIRE(func(-0) == 2);
}

TEST_CASE("Testing jit if statement 2", "[jit]") {

  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString(
      "int testFunc(int a, int b){int res = 0;if(a - (3*b)){res = "
      "a+ 10;}else{res= 2 - b;} return res;}");
  // making sure parsing went fine
  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  // making sure the code is generated
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  jit.addModule(gen.module);
  auto symbol = jit.findSymbol("testFunc");
  auto func = (int (*)(int, int))(intptr_t)llvm::cantFail(symbol.getAddress());
  REQUIRE(func(2, 5) == 12);
  REQUIRE(func(15, 5) == -3);
}

TEST_CASE("if function with multi statement jit", "[jit]") {
  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString(
      "int testFunc(int a, int b, int c, int k){int res = 0;if(a *2){ "
      "int x = k +1; res = "
      "a+ x;}else{int x = c - 1; res= x - b;} return res;}");

  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  // making sure the code is generated
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  jit.addModule(gen.module);
  auto symbol = jit.findSymbol("testFunc");
  auto func = (int (*)(int, int, int, int))(intptr_t)llvm::cantFail(
      symbol.getAddress());
  REQUIRE(func(2, 5, 7 ,1) == 4);
  REQUIRE(func(0, 5, 3, 7) == -3);
}
