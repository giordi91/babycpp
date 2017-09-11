#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <lexer.h>

using babycpp::lexer::Lexer;
using babycpp::lexer::Token;

TEST_CASE("Testing empty lexer", "[lexer]")
{
  Lexer lex;
  int tok = lex.gettok();
  REQUIRE(tok == Token::tok_empty_lexer);
}

TEST_CASE("Testing empty string", "[lexer]")
{
  Lexer lex;
  lex.initFromStr("");
  int tok = lex.gettok();
  REQUIRE(tok == Token::tok_eof);
}

TEST_CASE("Testing no match", "[lexer]")
{
  Lexer lex;
  lex.initFromStr(" ~~~~~~~ ");
  int tok = lex.gettok();
  REQUIRE(tok == Token::tok_no_match);
}

TEST_CASE("Testing keyword tok", "[lexer,stream]")
{
  //const std::string numbers {" int customIdentifier_12314_longName -1.234250 +1023948.00"};
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
  //bool res =true;
  //while (res) {

  //res = std::regex_search(running, m, expr,
  //                             std::regex_constants::match_continuous);
  //  for (uint32_t i = 1; i < m.size(); ++i) {
  //    if (m[i].matched) {
  //      std::cout << m[i] << std::endl;
  //      running += m.length();
  //      break;
  //    }
  //  }
  //}
  //std::cout<<(*running == 0)<<" "<<*running<<std::endl;
}

