#include <iostream>

#include "catch.hpp"

#include <codegen.h>
#include <factoryAST.h>
#include <parser.h>

using babycpp::codegen::Argument;
using babycpp::lexer::Lexer;
using babycpp::lexer::Token;
using babycpp::parser::Parser;

using babycpp::codegen::Argument;
using babycpp::codegen::BinaryExprAST;
using babycpp::codegen::CallExprAST;
using babycpp::codegen::ExprAST;
using babycpp::codegen::FunctionAST;
using babycpp::codegen::NumberExprAST;
using babycpp::codegen::PrototypeAST;
using babycpp::codegen::VariableExprAST;

static babycpp::memory::FactoryAST factory;

TEST_CASE("Testing number parser", "[parser]") {
  Lexer lex;
  Parser parser(&lex, &factory);
  lex.initFromStr("1.0");
  // getting first token
  lex.gettok();

  auto *node = parser.parseNumber();

  REQUIRE(node->val.type == Token::tok_float);
  REQUIRE(node->val.floatNumber == Approx(1.0));
}

TEST_CASE("Testing function call parsing", "[parser]") {
  Lexer lex;
  lex.initFromStr("testFunction ();");
  Parser parser(&lex, &factory);

  lex.gettok();
  auto *p = parser.parseIdentifier();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "testFunction");
  REQUIRE(p_casted->args.size() == 0);

  lex.initFromStr("functionWithWeirdName__324_NOW (    )  ;");
  lex.gettok();
  p = parser.parseIdentifier();
  REQUIRE(p != nullptr);

  p_casted = dynamic_cast<CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "functionWithWeirdName__324_NOW");
  REQUIRE(p_casted->args.size() == 0);
}

TEST_CASE("Testing extern call", "[parser]") {
  Lexer lex;
  lex.initFromStr("extern float sin(float x);");
  Parser parser(&lex, &factory);
  lex.gettok();

  auto *p = parser.parseExtern();
  REQUIRE(p != nullptr);

  REQUIRE(p != nullptr);
  REQUIRE(p->datatype == Token::tok_float);
  REQUIRE(p->name == "sin");
  REQUIRE(p->isExtern == true);
  REQUIRE(p->args.size() == 1);

  Argument arg = p->args[0];
  REQUIRE(arg.name == "x");
  REQUIRE(arg.type == Token::tok_float);
}

TEST_CASE("Testing variable definition", "[parser]") {
  Lexer lex;
  lex.initFromStr("int x = 2;");
  Parser parser(&lex, &factory);
  lex.gettok();

  auto *p = parser.parseDeclaration();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<VariableExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->name == "x");
  REQUIRE(p_casted->datatype == Token::tok_int);
  REQUIRE(p_casted->value != nullptr);

  auto *v_casted = dynamic_cast<NumberExprAST *>(p_casted->value);
  REQUIRE(v_casted != nullptr);
  REQUIRE(v_casted->val.type == Token::tok_int);
  REQUIRE(v_casted->val.integerNumber == 2);
}

TEST_CASE("Testing expression", "[parser]") {
  Lexer lex;
  lex.initFromStr("x * y");
  Parser parser(&lex, &factory);
  lex.gettok();

  auto *p = parser.parseExpression();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<BinaryExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  auto *lhs = p_casted->lhs;
  REQUIRE(lhs != nullptr);
  auto *lhs_casted = dynamic_cast<VariableExprAST *>(lhs);
  REQUIRE(lhs_casted != nullptr);
  REQUIRE(lhs_casted->name == "x");
  REQUIRE(lhs_casted->value == nullptr);
  REQUIRE(lhs_casted->datatype == 0);

  auto *rhs = p_casted->rhs;
  REQUIRE(rhs != nullptr);
  auto *rhs_casted = dynamic_cast<VariableExprAST *>(rhs);
  REQUIRE(rhs_casted != nullptr);
  REQUIRE(rhs_casted->name == "y");
  REQUIRE(rhs_casted->value == nullptr);
  REQUIRE(rhs_casted->datatype == 0);

  REQUIRE(p_casted->op == "*");
}

TEST_CASE("Testing expression for function call", "[parser]") {
  Lexer lex;
  lex.initFromStr("testFunction()");
  Parser parser(&lex, &factory);
  lex.gettok();

  auto *p = parser.parseExpression();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "testFunction");
  REQUIRE(p_casted->datatype == 0);
  REQUIRE(p_casted->args.size() == 0);

  lex.initFromStr("newFunction ( x,  y124, minusGravity )");
  lex.gettok();

  p = parser.parseExpression();
  REQUIRE(p != nullptr);

  p_casted = dynamic_cast<CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "newFunction");
  REQUIRE(p_casted->datatype == 0);
  REQUIRE(p_casted->args.size() == 3);

  // checking the argsss, args of function call are not
  // definition arguments so we don't know the type we can
  // only check the name, they are of instance variable
  auto *arg1 = dynamic_cast<VariableExprAST *>(p_casted->args[0]);
  REQUIRE(arg1 != nullptr);
  REQUIRE(arg1->datatype == 0);
  REQUIRE(arg1->name == "x");
  REQUIRE(arg1->value == nullptr);

  auto *arg2 = dynamic_cast<VariableExprAST *>(p_casted->args[1]);
  REQUIRE(arg2 != nullptr);
  REQUIRE(arg2->datatype == 0);
  REQUIRE(arg2->name == "y124");
  REQUIRE(arg2->value == nullptr);

  auto *arg3 = dynamic_cast<VariableExprAST *>(p_casted->args[2]);
  REQUIRE(arg3 != nullptr);
  REQUIRE(arg3->datatype == 0);
  REQUIRE(arg3->name == "minusGravity");
  REQUIRE(arg3->value == nullptr);
}

TEST_CASE("Testing identifier and  function call", "[parser]") {
  Lexer lex;
  lex.initFromStr("float meaningOfLife = computeMeaningOfLife(me);");
  Parser parser(&lex, &factory);
  lex.gettok();

  auto *p = parser.parseDeclaration();
  REQUIRE(p != nullptr);
  auto *p_casted = dynamic_cast<VariableExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->datatype == Token::tok_float);
  REQUIRE(p_casted->name == "meaningOfLife");

  auto *RHS = p_casted->value;
  REQUIRE(RHS != nullptr);
  auto *RHS_casted = dynamic_cast<CallExprAST *>(RHS);
  REQUIRE(RHS_casted != nullptr);
  REQUIRE(RHS_casted->callee == "computeMeaningOfLife");
  REQUIRE(RHS_casted->datatype == 0);
  REQUIRE(RHS_casted->args.size() == 1);

  auto *arg1 = dynamic_cast<VariableExprAST *>(RHS_casted->args[0]);
  REQUIRE(arg1 != nullptr);
  REQUIRE(arg1->datatype == 0);
  REQUIRE(arg1->name == "me");
  REQUIRE(arg1->value == nullptr);
}

TEST_CASE("Testing more complex expression", "[parser]") {
  Lexer lex;
  lex.initFromStr("x + 2 * y+z");
  Parser parser(&lex, &factory);
  lex.gettok();

  auto *p = parser.parseExpression();
  REQUIRE(p != nullptr);
  auto *firstBin = dynamic_cast<BinaryExprAST *>(p);
  REQUIRE(firstBin != nullptr);
  REQUIRE(firstBin->op == "+");

  auto *lhs_x2y = dynamic_cast<BinaryExprAST *>(firstBin->lhs);
  REQUIRE(lhs_x2y != nullptr);
  auto *lhs_x = dynamic_cast<VariableExprAST *>(lhs_x2y->lhs);
  REQUIRE(lhs_x != nullptr);
  REQUIRE(lhs_x->name == "x");

  auto *lhs_2y = dynamic_cast<BinaryExprAST *>(lhs_x2y->rhs);
  REQUIRE(lhs_2y != nullptr);
  REQUIRE(lhs_2y->op == "*");

  auto *lhs_2 = dynamic_cast<NumberExprAST *>(lhs_2y->lhs);
  REQUIRE(lhs_2 != nullptr);
  // here both the datatype of the expr and the specific type
  // of the number are set to int
  REQUIRE(lhs_2->datatype == Token::tok_int);
  REQUIRE(lhs_2->val.type == Token::tok_int);
  REQUIRE(lhs_2->val.integerNumber == 2);

  auto *lhs_y = dynamic_cast<VariableExprAST *>(lhs_2y->rhs);
  REQUIRE(lhs_y != nullptr);
  REQUIRE(lhs_y->name == "y");
}
TEST_CASE("Testing more complex expression with paren", "[parser]") {
  Lexer lex;
  lex.initFromStr("x + 2.0 * (y+z)");
  Parser parser(&lex, &factory);
  lex.gettok();

  auto *p = parser.parseExpression();
  REQUIRE(p != nullptr);
  auto *firstBin = dynamic_cast<BinaryExprAST *>(p);
  REQUIRE(firstBin != nullptr);
  REQUIRE(firstBin->op == "+");

  auto *lhs_x = dynamic_cast<VariableExprAST *>(firstBin->lhs);
  REQUIRE(lhs_x != nullptr);
  REQUIRE(lhs_x->name == "x");

  auto *rhs_2yz = dynamic_cast<BinaryExprAST *>(firstBin->rhs);
  REQUIRE(rhs_2yz != nullptr);
  REQUIRE(rhs_2yz->op == "*");

  auto *lhs_2 = dynamic_cast<NumberExprAST *>(rhs_2yz->lhs);
  REQUIRE(lhs_2 != nullptr);
  REQUIRE(lhs_2->val.type == Token::tok_float);
  REQUIRE(lhs_2->val.floatNumber == Approx(2.0f));

  auto *rhs_yz = dynamic_cast<BinaryExprAST *>(rhs_2yz->rhs);
  REQUIRE(rhs_yz != nullptr);
  REQUIRE(rhs_yz->op == "+");

  auto *lhs_y = dynamic_cast<VariableExprAST *>(rhs_yz->lhs);
  REQUIRE(lhs_y != nullptr);
  REQUIRE(lhs_y->name == "y");

  auto *rhs_z = dynamic_cast<VariableExprAST *>(rhs_yz->rhs);
  REQUIRE(rhs_z != nullptr);
  REQUIRE(rhs_z->name == "z");
}
TEST_CASE("Testing expression from top level", "[parser]") {
  Lexer lex;
  lex.initFromStr("x = y + z;");
  Parser parser(&lex, &factory);
  lex.gettok();

  auto *p = parser.parseStatement();
  REQUIRE(p != nullptr);
  auto *p_casted = dynamic_cast<VariableExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->value != nullptr);

  auto *value_p = dynamic_cast<BinaryExprAST *>(p_casted->value);
  REQUIRE(value_p != nullptr);
  REQUIRE(value_p->op == "+");

  auto *value_lhs = dynamic_cast<VariableExprAST *>(value_p->lhs);
  REQUIRE(value_lhs != nullptr);
  REQUIRE(value_lhs->name == "y");
  REQUIRE(value_lhs->value == nullptr);
  REQUIRE(value_lhs->datatype == 0);

  auto *value_rhs = dynamic_cast<VariableExprAST *>(value_p->rhs);
  REQUIRE(value_rhs != nullptr);
  REQUIRE(value_rhs->name == "z");
  REQUIRE(value_rhs->value == nullptr);
  REQUIRE(value_rhs->datatype == 0);

  // TODO(giordi) re-enable this test when there is a proper
  // error handling and suppression output
  // lex.initFromStr("x = y + z = 3;");
  // p = parser.parseStatement();
  // REQUIRE(p == nullptr);
}

TEST_CASE("Testing simple function", "[parser]") {
  Lexer lex;
  lex.initFromStr("float average(float a, float b) \n { \n"
                  "avg = (a+b)/2.0;}");
  Parser parser(&lex, &factory);
  lex.gettok();

  auto *p = parser.parseFunction();
  REQUIRE(p != nullptr);
}

TEST_CASE("Testing simple function with return", "[parser]") {
  Lexer lex;
  lex.initFromStr("float average(float a, float b) \n { \n"
                  "avg = (a+b)/2.0; return avg;}");
  Parser parser(&lex, &factory);
  lex.gettok();

  auto *p = parser.parseFunction();
  REQUIRE(p != nullptr);
  REQUIRE(p->body.size() == 2);

  auto *statement = p->body[1];
  REQUIRE(statement->flags.isReturn == true);
  auto *statement_casted = dynamic_cast<VariableExprAST *>(statement);
  REQUIRE(statement_casted != nullptr);
  REQUIRE(statement_casted->name == "avg");

  auto *p_eof = parser.parseStatement();
  REQUIRE(p_eof == nullptr);
  REQUIRE(lex.currtok == Token::tok_eof);
}

TEST_CASE("Testing function with variable declaration and expr", "[parser]") {
  Lexer lex;
  lex.initFromStr("float complexAdd(float x){ float temp = x * 2.0;temp = x - "
                  "2.0; return temp;}");
  Parser parser(&lex, &factory);
  lex.gettok();
  auto *p = parser.parseFunction();
  REQUIRE(p != nullptr);

  auto *statement = p->body[0];
  auto *statement_casted = dynamic_cast<VariableExprAST *>(statement);
  REQUIRE(statement_casted != nullptr);
  REQUIRE(statement_casted->name == "temp");
  REQUIRE(statement_casted->datatype == Token::tok_float);

  ExprAST *val = statement_casted->value;
  REQUIRE(val != nullptr);
}

TEST_CASE("Testing lookahead not losing tokens", "[parser]")
{

  Lexer lex;
  lex.initFromStr("float avg(float x){ return x *2.0;}");
  Parser parser(&lex, &factory);
  lex.gettok();
  lex.lookAhead(2);

  REQUIRE(lex.identifierStr == "float");

}
