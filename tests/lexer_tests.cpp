#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include "catch.hpp"
#include <lexer.h>

using babycpp::lexer::Lexer;
using babycpp::lexer::NumberType;
using babycpp::lexer::Token;
using babycpp::lexer::MovableToken;

TEST_CASE("Testing empty lexer", "[lexer]") {
  Lexer lex;
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_empty_lexer);
}

TEST_CASE("Testing empty string", "[lexer]") {
  Lexer lex;
  lex.initFromStr("");
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_eof);
}

TEST_CASE("Testing no match", "[lexer]") {
  Lexer lex;
  lex.initFromStr(" ~~~~~~~ ");
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_no_match);
}

TEST_CASE("Testing keyword tok", "[lexer]") {
  std::string str{" int"};
  Lexer lex;
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
  Lexer lex;
  lex.initFromStr(str);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::INTEGER);
  REQUIRE(lex.value.integerNumber == 12334);

  str = "0.3234214";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::FLOAT);
  REQUIRE(lex.value.floatNumber == Approx(0.3234214));

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
  REQUIRE(lex.value.type == NumberType::FLOAT);
  REQUIRE(lex.value.floatNumber == Approx(3240));
}

TEST_CASE("Testing operators tok", "[lexer]") {
  std::string str{" + 99 "};
  Lexer lex;
  lex.initFromStr(str);

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "+");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::INTEGER);
  REQUIRE(lex.value.integerNumber == 99);

  str = " - 0.314";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "-");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::FLOAT);
  REQUIRE(lex.value.floatNumber == Approx(0.314));

  str = " * 1191";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "*");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::INTEGER);
  REQUIRE(lex.value.integerNumber == 1191);

  str = " / 0.1135";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "/");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::FLOAT);
  REQUIRE(lex.value.floatNumber == Approx(0.1135));



  str = " < 19";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "<");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::INTEGER);
  REQUIRE(lex.value.integerNumber == 19);

  str = " = 3242235";
  lex.initFromStr(str);
  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_assigment_operator);
  REQUIRE(lex.identifierStr == "=");

  lex.gettok();
  REQUIRE(lex.currtok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::INTEGER);
  REQUIRE(lex.value.integerNumber == 3242235);
}

TEST_CASE("Testing extern tok", "[lexer]") {
  std::string str{" extern sin( float x);"};
  Lexer lex;
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

TEST_CASE("Testing multiline", "[lexer]") {
	std::string str{ "x \n  2.0" };
	Lexer lex;
	lex.initFromStr(str);

	lex.gettok();
	REQUIRE(lex.currtok == Token::tok_identifier);
	REQUIRE(lex.identifierStr == "x");

	lex.gettok();
	REQUIRE(lex.currtok == Token::tok_number);
	REQUIRE(lex.lineNumber== 2);
	REQUIRE(lex.value.type== NumberType::FLOAT);
	REQUIRE(lex.value.floatNumber== Approx(2.0));
}

TEST_CASE("Testing testing buffering", "[lexer]") {

	std::string str{ "aa 12 cc 3.14 ee" };
	Lexer lex;
	lex.initFromStr(str);
    REQUIRE(lex.lookAheadToken.empty());

    bool res = lex.lookAhead(4);
    REQUIRE(res == true);
    REQUIRE(lex.lookAheadToken.size() ==4);

    lex.gettok();
    REQUIRE(lex.lookAheadToken.size() ==3);
	REQUIRE(lex.currtok == Token::tok_identifier);
	REQUIRE(lex.identifierStr== "aa");

    lex.gettok();
    REQUIRE(lex.lookAheadToken.size() ==2);
	REQUIRE(lex.currtok == Token::tok_number);
	REQUIRE(lex.value.type== NumberType::INTEGER);
	REQUIRE(lex.value.integerNumber== 12);

    lex.gettok();
    REQUIRE(lex.lookAheadToken.size() ==1);
	REQUIRE(lex.currtok == Token::tok_identifier);
	REQUIRE(lex.identifierStr== "cc");

    lex.gettok();
    REQUIRE(lex.lookAheadToken.empty());
	REQUIRE(lex.currtok == Token::tok_number);
	REQUIRE(lex.value.type== NumberType::FLOAT);
	REQUIRE(lex.value.floatNumber == Approx(3.14));

    //here the buffer should be emtpy
    lex.gettok();
    REQUIRE(lex.lookAheadToken.empty());
	REQUIRE(lex.currtok == Token::tok_identifier);
	REQUIRE(lex.identifierStr== "ee");
}
TEST_CASE("Testing testing too much look ahead", "[lexer]") {
	std::string str{ "xyz " };
	Lexer lex;
	lex.initFromStr(str);

    bool res = lex.lookAhead(10);
    REQUIRE(res == false);
}
