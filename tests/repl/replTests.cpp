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



}
