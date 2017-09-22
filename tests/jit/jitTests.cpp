#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
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
