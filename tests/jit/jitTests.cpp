// this define is to disable the config main so that we can build all the test
// in a single executable
#ifndef CUMULATIVE_TESTS
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
#endif
// in one cpp file
#include "catch.hpp"

#include <codegen.h>
#include <iostream>
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
  REQUIRE(func(2, 5, 7, 1) == 4);
  REQUIRE(func(0, 5, 3, 7) == -3);
}
TEST_CASE("testing basic for loop jit", "[jit]") {
  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString(" int testFunc(int a){"
                     "int x = 0; "
                     "for ( int i = 1; i < a+1 ; i= i+1){ "
                     "x = x + i;}"
                     " return x;}");

  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  // making sure the code is generated
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  jit.addModule(gen.module);
  auto symbol = jit.findSymbol("testFunc");
  auto func = (int (*)(int))(intptr_t)llvm::cantFail(symbol.getAddress());
  // defining the closed formula for sum of first N numbers
  auto sumN = [](int x) { return (x * (x + 1)) / 2; };
  // testing the first x numbers
  for (int i = 0; i < 30; ++i) {
    REQUIRE(func(i) == sumN(i));
  }
}
TEST_CASE("testing bind with c functions", "[jit]") {
  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString(" "
                     "float testFunc(float a){"
                     " extern float cosf(float a);"
                     "float x=0.0;"
                     "x= cosf(a); "
                     " return x;}");

  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  // making sure the code is generated
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  jit.addModule(gen.module);
  auto symbol = jit.findSymbol("testFunc");
  auto func = (float (*)(float))(intptr_t)llvm::cantFail(symbol.getAddress());
  REQUIRE(func(0.5) == Approx(cos(0.5f)));
}
TEST_CASE("testing pointer derefenence float jit", "[jit]") {
  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString("float testFunc(float* a){ float res = *a;return res;};");

  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  // making sure the code is generated
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  jit.addModule(gen.module);
  auto symbol = jit.findSymbol("testFunc");
  auto func = (float (*)(float *))(intptr_t)llvm::cantFail(symbol.getAddress());
  float data = 0.125f;
  REQUIRE(func(&data) == Approx(0.125f));
  data = -1.23234f;
  REQUIRE(func(&data) == Approx(-1.23234f));
}
TEST_CASE("testing pointer derefenence int jit", "[jit]") {
  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString("int testFunc(int* a, int* b){ int res = 0; int aValue = "
                     "*a; int bValue = *b; res = aValue + bValue; return "
                     "res;};");

  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  // making sure the code is generated
  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  jit.addModule(gen.module);
  auto symbol = jit.findSymbol("testFunc");
  auto func = (int (*)(int*, int*))(intptr_t)llvm::cantFail(symbol.getAddress());
  int a = 10;
  int b = -5;
  REQUIRE(func(&a,&b) == 5);
  a = 102;
  b = 8;

  REQUIRE(func(&a, &b) ==  110);
}
