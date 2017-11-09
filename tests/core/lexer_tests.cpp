#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include "catch.hpp"
#include <lexer.h>

using babycpp::lexer::Lexer;
using babycpp::lexer::MovableToken;
using babycpp::lexer::Token;

babycpp::diagnostic::Diagnostic	 diagnostic;

TEST_CASE("Testing empty lexer", "[lexer]") {
  Lexer lex(diagnostic);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_empty_lexer);
}

TEST_CASE("Testing empty string", "[lexer]") {
  Lexer lex(diagnostic);
  lex.initFromStr("");
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_eof);
}

TEST_CASE("Testing no match", "[lexer]") {
  Lexer lex(diagnostic);
  lex.initFromStr(" ~~~~~~~ ");
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_no_match);
}

TEST_CASE("Testing keyword tok", "[lexer]") {
  std::string str{" int"};
  Lexer lex(diagnostic);
  lex.initFromStr(str);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_int);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_eof);

  str = " float lksdfjlj";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_float);

  str = "string ";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_string);
  lex.gettok();
  REQUIRE(lex.currtok != Token::tok_eof);
}

TEST_CASE("Testing numbers tok", "[lexer]") {
  std::string str{" 12334 "};
  Lexer lex(diagnostic);
  lex.initFromStr(str);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_int);
  REQUIRE(lex.value.integerNumber == 12334);

  str = "0.3234214";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_float);
  REQUIRE(lex.value.floatNumber == Approx(0.3234214f));

  str = "0.3.4214";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_malformed_number);

  str = "3240..34214";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_malformed_number);

  str = "3240.";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_float);
  REQUIRE(lex.value.floatNumber == Approx(3240.0f));
}

TEST_CASE("Testing operators tok", "[lexer]") {
  std::string str{" + 99 "};
  Lexer lex(diagnostic);
  lex.initFromStr(str);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "+");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_int);
  REQUIRE(lex.value.integerNumber == 99);

  str = " - 0.314";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "-");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_float);
  REQUIRE(lex.value.floatNumber == Approx(0.314f));

  str = " * 1191";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "*");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_int);
  REQUIRE(lex.value.integerNumber == 1191);

  str = " / 0.1135";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "/");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_float);
  REQUIRE(lex.value.floatNumber == Approx(0.1135f));

  str = " < 19";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "<");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_int);
  REQUIRE(lex.value.integerNumber == 19);

  str = " = 3242235";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_assigment_operator);
  REQUIRE(lex.identifierStr == "=");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_int);
  REQUIRE(lex.value.integerNumber == 3242235);
}
TEST_CASE("Testing * operator with no space", "[lexer]") {

  // in a situation like this, although pointers are not supported
  // yet, there is the chance that "x*" will be parsed as a pointer
  // and not as an operator
  std::string str{" x*4 "};
  Lexer lex(diagnostic);
  lex.initFromStr(str);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "x");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "*");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_int);
  REQUIRE(lex.value.integerNumber == 4);
}
TEST_CASE("Testing extern tok", "[lexer]") {
  std::string str{" extern sin( float x);"};
  Lexer lex(diagnostic);
  lex.initFromStr(str);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_extern);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "sin");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_open_round);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_float);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "x");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_close_round);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_end_statement);
}

TEST_CASE("Testing multi line", "[lexer]") {
  std::string str{"x \n  2.0"};
  Lexer lex(diagnostic);
  lex.initFromStr(str);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "x");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.lineNumber == 2);
  REQUIRE(lex.value.type == Token::tok_float);
  REQUIRE(lex.value.floatNumber == Approx(2.0f));
}

TEST_CASE("Testing testing buffering", "[lexer]") {

  std::string str{"aa 12 cc 3.14 ee"};
  Lexer lex(diagnostic);
  lex.initFromStr(str);
  REQUIRE(lex.lookAheadToken.empty());

  bool res = lex.lookAhead(4);
  REQUIRE(res == true);
  REQUIRE(lex.lookAheadToken.size() == 4);

  lex.gettok();
  REQUIRE(lex.lookAheadToken.size() == 3);
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "aa");

  lex.gettok();
  REQUIRE(lex.lookAheadToken.size() == 2);
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_int);
  REQUIRE(lex.value.integerNumber == 12);

  lex.gettok();
  REQUIRE(lex.lookAheadToken.size() == 1);
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "cc");

  lex.gettok();
  REQUIRE(lex.lookAheadToken.empty());
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == Token::tok_float);
  REQUIRE(lex.value.floatNumber == Approx(3.14f));

  // here the buffer should be emtpy
  lex.gettok();
  REQUIRE(lex.lookAheadToken.empty());
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "ee");
}
TEST_CASE("Testing too much look ahead", "[lexer]") {
  std::string str{"xyz "};
  Lexer lex(diagnostic);
  lex.initFromStr(str);

  bool res = lex.lookAhead(10);
  REQUIRE(res == false);
}
TEST_CASE("Testing clear look ahead", "[lexer]") {
  std::string str{"this should be cleared after look ahead and init"};
  Lexer lex(diagnostic);
  lex.initFromStr(str);

  bool res = lex.lookAhead(4);
  REQUIRE(res == true);
  lex.initFromStr("cleanup");
  REQUIRE(lex.lookAheadToken.size() == 0);
}

TEST_CASE("Testing if statement", "[lexer]") {
  std::string str{"if else randomword else whatever ifelse elseif"};
  Lexer lex(diagnostic);
  lex.initFromStr(str);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_if);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_else);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "randomword");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_else);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "whatever");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "ifelse");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "elseif");

}
