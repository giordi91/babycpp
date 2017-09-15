#include "catch.hpp"
#include <iostream>
#include <parser.h>

using babycpp::lexer::Lexer;
using babycpp::lexer::NumberType;
using babycpp::lexer::Token;
using babycpp::parser::Argument;
using babycpp::parser::Parser;

TEST_CASE("Testing number parser", "[parser]") {
  Lexer lex;
  Parser parser(&lex);
  lex.initFromStr("1.0");
  // getting first token
  lex.gettok();

  auto *node = parser.parseNumber();

  REQUIRE(node->val.type == NumberType::FLOAT);
  REQUIRE(node->val.floatNumber == Approx(1.0));
}

TEST_CASE("Testing function call", "[parser]") {
  Lexer lex;
  lex.initFromStr("testFunction ();");
  Parser parser(&lex);

  lex.gettok();
  auto *p = parser.parseIdentifier();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<babycpp::parser::CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "testFunction");
  REQUIRE(p_casted->args.size() == 0);

  lex.initFromStr("functionWithWeirdName__324_NOW (    )  ;");
  lex.gettok();
  p = parser.parseIdentifier();
  REQUIRE(p != nullptr);

  p_casted = dynamic_cast<babycpp::parser::CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "functionWithWeirdName__324_NOW");
  REQUIRE(p_casted->args.size() == 0);
}

TEST_CASE("Testing extern call", "[parser]") {
  Lexer lex;
  lex.initFromStr("extern float sin(float x);");
  Parser parser(&lex);
  lex.gettok();

  auto *p = parser.parseExtern();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<babycpp::parser::PrototypeAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->datatype == Token::tok_float);
  REQUIRE(p_casted->name == "sin");
  REQUIRE(p_casted->isExtern == true);
  REQUIRE(p_casted->args.size() == 1);

  Argument arg = p_casted->args[0];
  REQUIRE(arg.name == "x");
  REQUIRE(arg.type == Token::tok_float);
}

TEST_CASE("Testing variable definition", "[parser]") {
  Lexer lex;
  lex.initFromStr("int x = 2;");
  Parser parser(&lex);
  lex.gettok();

  auto *p = parser.parseDeclaration();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<babycpp::parser::VariableExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->name == "x");
  REQUIRE(p_casted->datatype == Token::tok_int);
  REQUIRE(p_casted->value != nullptr);

  auto *v_casted =
      dynamic_cast<babycpp::parser::NumberExprAST *>(p_casted->value);
  REQUIRE(v_casted != nullptr);
  REQUIRE(v_casted->val.type == NumberType::INTEGER);
  REQUIRE(v_casted->val.integerNumber == 2);
}

TEST_CASE("Testing expression", "[parser]") {
  Lexer lex;
  lex.initFromStr("x * y");
  Parser parser(&lex);
  lex.gettok();

  auto *p = parser.parseExpression();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<babycpp::parser::BinaryExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  auto *lhs = p_casted->lhs;
  REQUIRE(lhs != nullptr);
  auto *lhs_casted = dynamic_cast<babycpp::parser::VariableExprAST *>(lhs);
  REQUIRE(lhs_casted != nullptr);
  REQUIRE(lhs_casted->name == "x");
  REQUIRE(lhs_casted->value == nullptr);
  REQUIRE(lhs_casted->datatype == 0);

  auto *rhs = p_casted->rhs;
  REQUIRE(rhs != nullptr);
  auto *rhs_casted = dynamic_cast<babycpp::parser::VariableExprAST *>(rhs);
  REQUIRE(rhs_casted != nullptr);
  REQUIRE(rhs_casted->name == "y");
  REQUIRE(rhs_casted->value == nullptr);
  REQUIRE(rhs_casted->datatype == 0);

  REQUIRE(p_casted->op == "*");
}

TEST_CASE("Testing expression for function call", "[parser]") {
  Lexer lex;
  lex.initFromStr("testFunction()");
  Parser parser(&lex);
  lex.gettok();

  auto *p = parser.parseExpression();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<babycpp::parser::CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "testFunction");
  REQUIRE(p_casted->datatype == 0);
  REQUIRE(p_casted->args.size() == 0);

  lex.initFromStr("newFunction ( x,  y124, minusGravity )");
  lex.gettok();

  p = parser.parseExpression();
  REQUIRE(p != nullptr);

  p_casted = dynamic_cast<babycpp::parser::CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "newFunction");
  REQUIRE(p_casted->datatype == 0);
  REQUIRE(p_casted->args.size() == 3);

  // checking the argsss, args of function call are not
  // definition arguments so we don't know the type we can
  // only check the name, they are of instance variable
  auto *arg1 =
      dynamic_cast<babycpp::parser::VariableExprAST *>(p_casted->args[0]);
  REQUIRE(arg1 != nullptr);
  REQUIRE(arg1->datatype == 0);
  REQUIRE(arg1->name == "x");
  REQUIRE(arg1->value == nullptr);

  auto *arg2 =
      dynamic_cast<babycpp::parser::VariableExprAST *>(p_casted->args[1]);
  REQUIRE(arg2 != nullptr);
  REQUIRE(arg2->datatype == 0);
  REQUIRE(arg2->name == "y124");
  REQUIRE(arg2->value == nullptr);

  auto *arg3 =
      dynamic_cast<babycpp::parser::VariableExprAST *>(p_casted->args[2]);
  REQUIRE(arg3 != nullptr);
  REQUIRE(arg3->datatype == 0);
  REQUIRE(arg3->name == "minusGravity");
  REQUIRE(arg3->value == nullptr);
}

TEST_CASE("Testing identifier and  function call", "[parser]") {
  Lexer lex;
  lex.initFromStr("float meaningOfLife = computeMeaningOfLife(me)");
  Parser parser(&lex);
  lex.gettok();

  auto *p = parser.parseDeclaration();
  REQUIRE(p != nullptr);
  auto *p_casted = dynamic_cast<babycpp::parser::VariableExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->datatype == Token::tok_float);
  REQUIRE(p_casted->name == "meaningOfLife");

  auto *RHS = p_casted->value;
  REQUIRE(RHS != nullptr);
  auto *RHS_casted = dynamic_cast<babycpp::parser::CallExprAST *>(RHS);
  REQUIRE(RHS_casted != nullptr);
  REQUIRE(RHS_casted->callee == "computeMeaningOfLife");
  REQUIRE(RHS_casted->datatype == 0);
  REQUIRE(RHS_casted->args.size() == 1);

  auto *arg1 =
      dynamic_cast<babycpp::parser::VariableExprAST *>(RHS_casted->args[0]);
  REQUIRE(arg1 != nullptr);
  REQUIRE(arg1->datatype == 0);
  REQUIRE(arg1->name == "me");
  REQUIRE(arg1->value == nullptr);
}

TEST_CASE("Testing more complex expression", "[parser]") {
  Lexer lex;
  lex.initFromStr("x + 2 * y+z");
  Parser parser(&lex);
  lex.gettok();

  auto *p = parser.parseExpression();
  REQUIRE(p != nullptr);
  auto *firstBin = dynamic_cast<babycpp::parser::BinaryExprAST *>(p);
  REQUIRE(firstBin != nullptr);
  REQUIRE(firstBin->op == "+");

  auto *lhs_x2y = dynamic_cast<babycpp::parser::BinaryExprAST *>(firstBin->lhs);
  REQUIRE(lhs_x2y != nullptr);
  auto *lhs_x = dynamic_cast<babycpp::parser::VariableExprAST *>(lhs_x2y->lhs);
  REQUIRE(lhs_x != nullptr);
  REQUIRE(lhs_x->name == "x");

  auto *lhs_2y = dynamic_cast<babycpp::parser::BinaryExprAST *>(lhs_x2y->rhs);
  REQUIRE(lhs_2y != nullptr);
  REQUIRE(lhs_2y->op == "*");

  auto *lhs_2 = dynamic_cast<babycpp::parser::NumberExprAST *>(lhs_2y->lhs);
  REQUIRE(lhs_2 != nullptr);
  REQUIRE(lhs_2->datatype == 0);
  REQUIRE(lhs_2->val.type == NumberType::INTEGER);
  REQUIRE(lhs_2->val.integerNumber == 2);

  auto *lhs_y = dynamic_cast<babycpp::parser::VariableExprAST *>(lhs_2y->rhs);
  REQUIRE(lhs_y != nullptr);
  REQUIRE(lhs_y->name == "y");
}
TEST_CASE("Testing more complex expression with paren", "[parser]") {
  Lexer lex;
  lex.initFromStr("x + 2.0 * (y+z)");
  Parser parser(&lex);
  lex.gettok();

  auto *p = parser.parseExpression();
  REQUIRE(p != nullptr);
  auto *firstBin = dynamic_cast<babycpp::parser::BinaryExprAST *>(p);
  REQUIRE(firstBin != nullptr);
  REQUIRE(firstBin->op == "+");

  auto *lhs_x = dynamic_cast<babycpp::parser::VariableExprAST *>(firstBin->lhs);
  REQUIRE(lhs_x != nullptr);
  REQUIRE(lhs_x->name == "x");

  auto *rhs_2yz = dynamic_cast<babycpp::parser::BinaryExprAST*>(firstBin->rhs);
  REQUIRE(rhs_2yz!= nullptr);
  REQUIRE(rhs_2yz->op == "*");

  auto *lhs_2 = dynamic_cast<babycpp::parser::NumberExprAST*>(rhs_2yz->lhs);
  REQUIRE(lhs_2!= nullptr);
  REQUIRE(lhs_2->val.type == NumberType::FLOAT);
  REQUIRE(lhs_2->val.floatNumber== Approx(2.0f));

  auto *rhs_yz = dynamic_cast<babycpp::parser::BinaryExprAST*>(rhs_2yz->rhs);
  REQUIRE(rhs_yz!= nullptr);
  REQUIRE(rhs_yz->op == "+");

  auto *lhs_y = dynamic_cast<babycpp::parser::VariableExprAST*>(rhs_yz->lhs);
  REQUIRE(lhs_y!= nullptr);
  REQUIRE(lhs_y->name == "y");

  auto *rhs_z = dynamic_cast<babycpp::parser::VariableExprAST*>(rhs_yz->rhs);
  REQUIRE(rhs_z!= nullptr);
  REQUIRE(rhs_z->name == "z");
}
