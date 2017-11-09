//this define is to disable the config main so that we can build all the test in a single executable
#ifndef CUMULATIVE_TESTS
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
#endif
// in one cpp file

#include <parser.h>
#include <codegen.h>
#include "catch.hpp"
#include "repl.h"
#include "jit.h"

using babycpp::repl::lookAheadStatement;
using babycpp::repl::handleExpression;
using babycpp::codegen::Codegenerator;
using babycpp::jit::BabycppJIT;
using babycpp::lexer::Token;

TEST_CASE("Testing look ahead for repl", "[repl]") {

  Codegenerator gen;
  gen.initFromString("int test(float x){return x;}");
  int res = lookAheadStatement(&gen.lexer);
  REQUIRE(res == Token::tok_function_repl);

  gen.initFromString("float test;");
  res = lookAheadStatement(&gen.lexer);
  REQUIRE(res == Token::tok_invalid_repl);

  gen.initFromString("float test = 2.0;");
  res = lookAheadStatement(&gen.lexer);
  REQUIRE(res == Token::tok_assigment_repl);

  gen.initFromString("test = x+ 2.0;");
  res = lookAheadStatement(&gen.lexer);
  REQUIRE(res == Token::tok_anonymous_assigment_repl);

  gen.initFromString("x+ 2.0;");
  res = lookAheadStatement(&gen.lexer);
  REQUIRE(res == Token::tok_expression_repl);

  gen.initFromString("extern sin(x);");
  res = lookAheadStatement(&gen.lexer);
  REQUIRE(res == Token::tok_extern);

  gen.initFromString("5 + 2;");
  res = lookAheadStatement(&gen.lexer);
  REQUIRE(res == Token::tok_expression_repl);

  gen.initFromString("(5 + 2);");
  res = lookAheadStatement(&gen.lexer);
  REQUIRE(res == Token::tok_expression_repl);

  gen.initFromString("float avg(float x){ return x *2.0;}");
  res = lookAheadStatement(&gen.lexer);
  REQUIRE(res == Token::tok_function_repl);
}


TEST_CASE("Testing expressions", "[repl]") {
  Codegenerator gen;
  gen.initFromString("3 + 2");
  BabycppJIT jit;
  auto anonymousModule =
      std::make_shared<llvm::Module>("anonymous", gen.context);
  auto staticModule =
      std::make_shared<llvm::Module>("static", gen.context);

  handleExpression(&gen, &jit, anonymousModule, staticModule);

}
