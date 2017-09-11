#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include "catch.hpp"
#include <lexer.h>

using babycpp::lexer::Lexer;
using babycpp::lexer::NumberType;
using babycpp::lexer::Token;

TEST_CASE("Testing empty lexer", "[lexer]") {
  Lexer lex;
  int tok = lex.gettok();
  REQUIRE(tok == Token::tok_empty_lexer);
}

TEST_CASE("Testing empty string", "[lexer]") {
  Lexer lex;
  lex.initFromStr("");
  int tok = lex.gettok();
  REQUIRE(tok == Token::tok_eof);
}

TEST_CASE("Testing no match", "[lexer]") {
  Lexer lex;
  lex.initFromStr(" ~~~~~~~ ");
  int tok = lex.gettok();
  REQUIRE(tok == Token::tok_no_match);
}

TEST_CASE("Testing keyword tok", "[lexer,stream]") {
  // const std::string numbers {" int customIdentifier_12314_longName -1.234250
  // +1023948.00"};
  std::string str{" int"};
  Lexer lex;
  lex.initFromStr(str);

  int tok = lex.gettok();
  REQUIRE(tok == Token::tok_int);
  tok = lex.gettok();
  REQUIRE(tok == Token::tok_eof);

  str = " float lksdfjlj";
  lex.initFromStr(str);
  tok = lex.gettok();
  REQUIRE(tok == Token::tok_float);

  str = "string ";
  lex.initFromStr(str);
  tok = lex.gettok();
  REQUIRE(tok == Token::tok_string);
  tok = lex.gettok();
  REQUIRE(tok != Token::tok_eof);
  // bool res =true;
  // while (res) {

  // res = std::regex_search(running, m, expr,
  //                             std::regex_constants::match_continuous);
  //  for (uint32_t i = 1; i < m.size(); ++i) {
  //    if (m[i].matched) {
  //      std::cout << m[i] << std::endl;
  //      running += m.length();
  //      break;
  //    }
  //  }
  //}
  // std::cout<<(*running == 0)<<" "<<*running<<std::endl;
}
TEST_CASE("Testing numbers tok", "[lexer,stream]") {
  std::string str{" 12334 "};
  Lexer lex;
  lex.initFromStr(str);

  int tok = lex.gettok();
  REQUIRE(tok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::INTEGER);
  REQUIRE(lex.value.integerNumber == 12334);

  str = "0.3234214";
  lex.initFromStr(str);
  tok = lex.gettok();
  REQUIRE(tok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::FLOAT);
  REQUIRE(lex.value.floatNumber == Approx(0.3234214));

  str = "0.3.4214";
  lex.initFromStr(str);
  tok = lex.gettok();
  REQUIRE(tok == Token::tok_malformed_number);

  str = "3240..34214";
  lex.initFromStr(str);
  tok = lex.gettok();
  REQUIRE(tok == Token::tok_malformed_number);

  str = "3240.";
  lex.initFromStr(str);
  tok = lex.gettok();
  REQUIRE(tok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::FLOAT);
  REQUIRE(lex.value.floatNumber == Approx(3240));
}

TEST_CASE("Testing operators tok", "[lexer,stream]") {
  std::string str{" + 99 "};
  Lexer lex;
  lex.initFromStr(str);

  int tok = lex.gettok();
  REQUIRE(tok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "+");

  tok = lex.gettok();
  REQUIRE(tok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::INTEGER);
  REQUIRE(lex.value.integerNumber == 99);

  str = " - 0.314";
  lex.initFromStr(str);
  tok = lex.gettok();
  REQUIRE(tok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "-");

  tok = lex.gettok();
  REQUIRE(tok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::FLOAT);
  REQUIRE(lex.value.floatNumber == Approx(0.314));

  str = " * 1191";
  lex.initFromStr(str);
  tok = lex.gettok();
  REQUIRE(tok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "*");

  tok = lex.gettok();
  REQUIRE(tok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::INTEGER);
  REQUIRE(lex.value.integerNumber == 1191);

  str = " / 0.1135";
  lex.initFromStr(str);
  tok = lex.gettok();
  REQUIRE(tok == Token::tok_operator);
  REQUIRE(lex.identifierStr == "/");

  tok = lex.gettok();
  REQUIRE(tok == Token::tok_number);
  REQUIRE(lex.value.type == NumberType::FLOAT);
  REQUIRE(lex.value.floatNumber == Approx(0.1135));
}

TEST_CASE("Testing extern tok", "[lexer,stream]") {
  std::string str{" extern sin( float x);"};
  Lexer lex;
  lex.initFromStr(str);

  int tok = lex.gettok();
  REQUIRE(tok == Token::tok_extern);

  tok = lex.gettok();
  REQUIRE(tok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "sin");

  tok = lex.gettok();
  std::cout << lex.identifierStr << std::endl;
  REQUIRE(tok == Token::tok_open_round);

  tok = lex.gettok();
  REQUIRE(tok == Token::tok_float);

  tok = lex.gettok();
  REQUIRE(tok == Token::tok_identifier);
  REQUIRE(lex.identifierStr == "x");

  tok = lex.gettok();
  REQUIRE(tok == Token::tok_close_round);

  tok = lex.gettok();
  REQUIRE(tok == Token::tok_end_statement);
}