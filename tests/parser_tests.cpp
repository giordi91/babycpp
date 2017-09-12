#include "catch.hpp"
#include <parser.h>

using babycpp::lexer::Lexer;
using babycpp::lexer::NumberType;
using babycpp::lexer::Token;
using babycpp::parser::Parser;

TEST_CASE("Testing number parser", "[parser]") {
  Lexer lex;
  Parser parser(&lex);
  lex.initFromStr("1.0");
  //getting first token
  lex.gettok();
    
  auto* node = babycpp::parser::parseNumber();

  REQUIRE(node->val.type == NumberType::FLOAT);
  REQUIRE(node->val.floatNumber == Approx(1.0));
}

