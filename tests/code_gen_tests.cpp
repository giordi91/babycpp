#include <iostream>

#include "catch.hpp"

#include <codegen.h>

using babycpp::codegen::Codegenerator;
using babycpp::lexer::Lexer;
using babycpp::lexer::Token;
using babycpp::parser::Parser;

TEST_CASE("Testing code gen number float", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("1.0");

  auto *node = gen.parser.parseNumber();
  REQUIRE(node != nullptr);

  auto * val = node->codegen(&gen);
  REQUIRE(val!= nullptr);

  std::string outs= gen.printLlvmValue(val);
  REQUIRE(outs == "float 1.000000e+00");
}
TEST_CASE("Testing code gen number int", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("39");

  auto *node = gen.parser.parseNumber();
  REQUIRE(node != nullptr);

  auto * val = node->codegen(&gen);
  REQUIRE(val!= nullptr);

  std::string outs= gen.printLlvmValue(val);
  REQUIRE(outs == "i32 39");

}
