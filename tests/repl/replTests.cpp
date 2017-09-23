#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
// in one cpp file

#include <parser.h>
#include <codegen.h>
#include "catch.hpp"
#include "repl.h"

using babycpp::repl::lookAheadStatement;
using babycpp::codegen::Codegenerator;
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
}
