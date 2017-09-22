
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
// in one cpp file
#include "catch.hpp"

#include <iostream>
#include <codegen.h>
#include <jit.h>
#include <codegen.h>
#include <factoryAST.h>
#include <parser.h>
inline std::string getFile(const ::std::string &path) {

  std::ifstream t(path);
  std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
  return str;
}
using babycpp::codegen::Codegenerator;
#include <llvm/Support/TargetSelect.h>
TEST_CASE("Testing jit add", "[jit]") {

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
  babycpp::jit::BabycppJIT jit;
  Codegenerator gen;
  gen.initFromString( "float test(float x){ return x+1.0;}");
  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  //auto* sptr =  &gen.module;
  //std::shared_ptr<llvm::Module> ptr(sptr);

  auto handle = jit.addModule(gen.module);
  auto symbol = jit.findSymbol("test");
  auto func = (float(*)(float))(intptr_t)llvm::cantFail(symbol.getAddress());
  REQUIRE(func(10)==11);
  //std::cout<<"done"<<std::endl;
  //ptr.reset(nullptr);
  //int i = 0;

}














