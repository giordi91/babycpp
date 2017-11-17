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
using babycpp::codegen::IfAST;
using babycpp::codegen::NumberExprAST;
using babycpp::codegen::PrototypeAST;
using babycpp::codegen::VariableExprAST;

static babycpp::memory::FactoryAST factory;

babycpp::diagnostic::Diagnostic diagnosticParserTests;

TEST_CASE("Testing number parser", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.initFromString("1.0");
  // getting first token
  lex.gettok();

  auto *node = parser.parseNumber();

  REQUIRE(node->val.type == Token::tok_float);
  REQUIRE(node->val.floatNumber == Approx(1.0));
}

TEST_CASE("Testing function call parsing", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("testFunction ();");
  Parser parser(&lex, &factory, &diagnosticParserTests);

  lex.gettok();
  auto *p = parser.parseIdentifier();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "testFunction");
  REQUIRE(p_casted->args.size() == 0);

  lex.initFromString("functionWithWeirdName__324_NOW (    )  ;");
  lex.gettok();
  p = parser.parseIdentifier();
  REQUIRE(p != nullptr);

  p_casted = dynamic_cast<CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "functionWithWeirdName__324_NOW");
  REQUIRE(p_casted->args.size() == 0);
}
TEST_CASE("Testing function call parsing error 1", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("testFunction (xx xx );");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  // a function call gets parsed when the identifier is parsed,
  // we check if the next top is a round paren, if so we process the func call
  auto res = parser.parseIdentifier();

  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(
      err.code ==
      babycpp::diagnostic::IssueCode::MISSING_OPEN_ROUND_OR_COMMA_IN_FUNC_CALL);
}

TEST_CASE("Testing function call parsing error 2", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("testFunction (xx,   );");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  // a function call gets parsed when the identifier is parsed,
  // we check if the next top is a round paren, if so we process the func call
  auto res = parser.parseIdentifier();

  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 2);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code ==
          babycpp::diagnostic::IssueCode::UNEXPECTED_TOKEN_IN_EXPRESSION);

  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err2 = parser.diagnostic->getError();

  REQUIRE(err2.code ==
          babycpp::diagnostic::IssueCode::MISSING_ARG_IN_FUNC_CALL);
}

TEST_CASE("Testing parsing prototype error 1", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("float testFunction xx;");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parsePrototype();

  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}

TEST_CASE("Testing parsing prototype error 2", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("float ( xx;");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parsePrototype();

  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_IDENTIFIER_NAME);
}

TEST_CASE("Testing parsing prototype error 3", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("myFuncProtoype ( int xx);");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parsePrototype();

  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_RETURN_DATATYPE);
}

TEST_CASE("Testing extern call", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("extern float sin(float x);");
  Parser parser(&lex, &factory, &diagnosticParserTests);
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
TEST_CASE("Testing extern call error 1", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("extern sin(float x);");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto *res = parser.parseExtern();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code ==
          babycpp::diagnostic::IssueCode::EXPECTED_TYPE_AFTER_EXTERN);
}

TEST_CASE("Testing variable definition", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("int x = 2;");
  Parser parser(&lex, &factory, &diagnosticParserTests);
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
// TODO(giordi) cannot find a way to trigger this error, re-enable if happens
// TEST_CASE("Testing variable definition error", "[parser]") {
//  diagnosticParserTests.clear();
//  Lexer lex(&diagnosticParserTests);
//  lex.initFromString("int x = 3 y;");
//  Parser parser(&lex, &factory, &diagnosticParserTests);
//  lex.gettok();

//  auto *p = parser.parseDeclaration();
//  REQUIRE(p == nullptr);
//  REQUIRE(parser.diagnostic->hasErrors() == true);
//  auto err = parser.diagnostic->getError();
//  REQUIRE(
//     err.code ==
//     babycpp::diagnostic::IssueCode::MISSING_OPEN_ROUND_OR_COMMA_IN_FUNC_CALL);
//  const std::string errorMessage = parser.diagnostic->printErorr(err);
//  std::cout << errorMessage << std::endl;
//}

TEST_CASE("Testing expression", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("x * y");
  Parser parser(&lex, &factory, &diagnosticParserTests);
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
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("testFunction()");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto *p = parser.parseExpression();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<CallExprAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->callee == "testFunction");
  REQUIRE(p_casted->datatype == 0);
  REQUIRE(p_casted->args.size() == 0);

  lex.initFromString("newFunction ( x,  y124, minusGravity )");
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
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("float meaningOfLife = computeMeaningOfLife(me);");
  Parser parser(&lex, &factory, &diagnosticParserTests);
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
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("x + 2 * y+z");
  Parser parser(&lex, &factory, &diagnosticParserTests);
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
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("x + 2.0 * (y+z)");
  Parser parser(&lex, &factory, &diagnosticParserTests);
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
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("x = y + z;");
  Parser parser(&lex, &factory, &diagnosticParserTests);
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
  // lex.initFromString("x = y + z = 3;");
  // p = parser.parseStatement();
  // REQUIRE(p == nullptr);
}

TEST_CASE("Testing simple function", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("float average(float a, float b) \n { \n"
                     "avg = (a+b)/2.0;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto *p = parser.parseFunction();
  REQUIRE(p != nullptr);
}

TEST_CASE("Testing simple function error missing datatype arg", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("float average(a, float b) \n { \n"
                     "avg = (a+b)/2.0;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto *p = parser.parseFunction();
  REQUIRE(p == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code ==
          babycpp::diagnostic::IssueCode::EXPECTED_DATATYPE_FUNCTION_ARG);
}

TEST_CASE("Testing simple function error missing close curly ", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("float average(float a, float b) \n { \n"
                     "avg = (a+b)/2.0;");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto *p = parser.parseFunction();
  REQUIRE(p == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}

TEST_CASE("Testing simple function error missing name arg", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("float average(float , float b) \n { \n"
                     "avg = (a+b)/2.0;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto *p = parser.parseFunction();
  REQUIRE(p == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_IDENTIFIER_NAME);
}

TEST_CASE("Testing simple function error with extra comma in args",
          "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("float average(float ,)\n { \n"
                     "avg = (a+b)/2.0;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto *p = parser.parseFunction();
  REQUIRE(p == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_IDENTIFIER_NAME);
}
TEST_CASE("Testing simple function with return", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("float average(float a, float b) \n { \n"
                     "avg = (a+b)/2.0; return avg;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
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
TEST_CASE("Testing no end of statement", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("int avg = (a+b)/2.0");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto *p = parser.parseStatement();
  REQUIRE(p == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code ==
          babycpp::diagnostic::IssueCode::EXPECTED_END_STATEMENT_TOKEN);
}

TEST_CASE("Testing function with variable declaration and expr", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString(
      "float complexAdd(float x){ float temp = x * 2.0;temp = x - "
      "2.0; return temp;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
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

TEST_CASE("Testing lookahead not losing tokens", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("float avg(float x){ return x *2.0;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();
  lex.lookAhead(2);

  REQUIRE(lex.identifierStr == "float");
}

TEST_CASE("Testing if statement parsing", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("if ( 3 + 1) { int x = 1 +1 ;}else{ int x = 2 + 2;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();
  auto res = parser.parseIfStatement();
  REQUIRE(res != nullptr);

  auto *res_casted = dynamic_cast<IfAST *>(res);
  REQUIRE(res_casted != nullptr);

  auto *condition = res_casted->condition;
  auto *cond_casted = dynamic_cast<BinaryExprAST *>(condition);
  REQUIRE(cond_casted != nullptr);

  // verify the condtion got parsed correctly
  auto *condlhs = dynamic_cast<NumberExprAST *>(cond_casted->lhs);
  auto *condrhs = dynamic_cast<NumberExprAST *>(cond_casted->rhs);
  REQUIRE(condlhs != nullptr);
  REQUIRE(condrhs != nullptr);

  REQUIRE(condlhs->datatype == Token::tok_int);
  REQUIRE(condlhs->val.integerNumber == 3);
  REQUIRE(condrhs->datatype == Token::tok_int);
  REQUIRE(condrhs->val.integerNumber == 1);

  // verify the if branch got parsed properly
  auto *ifBranch = dynamic_cast<VariableExprAST *>(res_casted->ifExpr[0]);
  REQUIRE(ifBranch != nullptr);
  // being an assigment operation the variable will have a value assigned to it,
  // and in this case is a  binary expression
  auto *value = dynamic_cast<BinaryExprAST *>(ifBranch->value);
  REQUIRE(value != nullptr);
  REQUIRE(ifBranch->datatype == Token::tok_int);

  // checking the if branch binary expression
  condlhs = dynamic_cast<NumberExprAST *>(value->lhs);
  condrhs = dynamic_cast<NumberExprAST *>(value->rhs);
  REQUIRE(condlhs != nullptr);
  REQUIRE(condrhs != nullptr);

  REQUIRE(condlhs->datatype == Token::tok_int);
  REQUIRE(condlhs->val.integerNumber == 1);
  REQUIRE(condrhs->datatype == Token::tok_int);
  REQUIRE(condrhs->val.integerNumber == 1);

  // checking the else branch
  auto *elseBranchBody =
      dynamic_cast<VariableExprAST *>(res_casted->elseExpr[0]);
  REQUIRE(elseBranchBody != nullptr);
  // being an assigment operation the variable will have a value assigned to it,
  // and in this case is a  binary expression
  value = dynamic_cast<BinaryExprAST *>(elseBranchBody->value);
  REQUIRE(value != nullptr);
  REQUIRE(elseBranchBody->datatype == Token::tok_int);

  condlhs = dynamic_cast<NumberExprAST *>(value->lhs);
  condrhs = dynamic_cast<NumberExprAST *>(value->rhs);
  REQUIRE(condlhs != nullptr);
  REQUIRE(condrhs != nullptr);

  REQUIRE(condlhs->datatype == Token::tok_int);
  REQUIRE(condlhs->val.integerNumber == 2);
  REQUIRE(condrhs->datatype == Token::tok_int);
  REQUIRE(condrhs->val.integerNumber == 2);
}

TEST_CASE("Testing if statement parsing multi statements", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  lex.initFromString("if ( z*2 ) { int x = k +1 ; x = x - y;}else{ "
                     "int x = 2 + 2; x = x + y;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();
  auto res = parser.parseIfStatement();

  REQUIRE(res != nullptr);

  auto *res_casted = dynamic_cast<IfAST *>(res);
  REQUIRE(res_casted != nullptr);

  auto *condition = res_casted->condition;
  auto *cond_casted = dynamic_cast<BinaryExprAST *>(condition);
  REQUIRE(cond_casted != nullptr);

  // verify the condtion got parsed correctly
  auto *condlhs = dynamic_cast<VariableExprAST *>(cond_casted->lhs);
  auto *condrhs = dynamic_cast<NumberExprAST *>(cond_casted->rhs);
  REQUIRE(condlhs != nullptr);
  REQUIRE(condrhs != nullptr);

  REQUIRE(condlhs->datatype == 0);
  REQUIRE(condlhs->name == "z");
  REQUIRE(condrhs->datatype == Token::tok_int);
  REQUIRE(condrhs->val.integerNumber == 2);
  REQUIRE(cond_casted->op == "*");

  // verify the if branch got parsed properly
  REQUIRE(res_casted->ifExpr.size() == 2);
  auto *ifBranch = dynamic_cast<VariableExprAST *>(res_casted->ifExpr[0]);
  REQUIRE(ifBranch != nullptr);
  // being an assigment operation the variable will have a value assigned to it,
  // and in this case is a  binary expression
  auto *value = dynamic_cast<BinaryExprAST *>(ifBranch->value);
  REQUIRE(value != nullptr);
  REQUIRE(ifBranch->datatype == Token::tok_int);

  // checking the if branch binary expression
  condlhs = dynamic_cast<VariableExprAST *>(value->lhs);
  condrhs = dynamic_cast<NumberExprAST *>(value->rhs);
  REQUIRE(condlhs != nullptr);
  REQUIRE(condrhs != nullptr);

  REQUIRE(condlhs->datatype == 0);
  REQUIRE(condlhs->name == "k");
  REQUIRE(condrhs->datatype == Token::tok_int);
  REQUIRE(condrhs->val.integerNumber == 1);

  // checking second statement in the if branch
  ifBranch = dynamic_cast<VariableExprAST *>(res_casted->ifExpr[1]);
  value = dynamic_cast<BinaryExprAST *>(ifBranch->value);
  REQUIRE(value != nullptr);
  REQUIRE(ifBranch->datatype == 0);

  // checking the if branch binary expression
  condlhs = dynamic_cast<VariableExprAST *>(value->lhs);
  auto *condrhs2 = dynamic_cast<VariableExprAST *>(value->rhs);
  REQUIRE(condlhs != nullptr);
  REQUIRE(condrhs != nullptr);

  REQUIRE(condlhs->datatype == 0);
  REQUIRE(condlhs->name == "x");
  REQUIRE(condrhs2->name == "y");
  REQUIRE(condrhs2->datatype == 0);
  REQUIRE(value->op == "-");

  // now checking else statement
  REQUIRE(res_casted->elseExpr.size() == 2);
  auto *elseBranchBody =
      dynamic_cast<VariableExprAST *>(res_casted->elseExpr[0]);
  REQUIRE(elseBranchBody != nullptr);
  // being an assigment operation the variable will have a value assigned to it,
  // and in this case is a  binary expression
  value = dynamic_cast<BinaryExprAST *>(elseBranchBody->value);
  REQUIRE(value != nullptr);
  REQUIRE(elseBranchBody->datatype == Token::tok_int);

  auto *condlhs2 = dynamic_cast<NumberExprAST *>(value->lhs);
  condrhs = dynamic_cast<NumberExprAST *>(value->rhs);
  REQUIRE(condlhs != nullptr);
  REQUIRE(condrhs != nullptr);

  REQUIRE(condlhs2->datatype == Token::tok_int);
  REQUIRE(condlhs2->val.integerNumber == 2);
  REQUIRE(condrhs->datatype == Token::tok_int);
  REQUIRE(condrhs->val.integerNumber == 2);

  // checking second statement in the if branch
  elseBranchBody = dynamic_cast<VariableExprAST *>(res_casted->elseExpr[1]);
  REQUIRE(elseBranchBody != nullptr);
  value = dynamic_cast<BinaryExprAST *>(elseBranchBody->value);
  REQUIRE(value != nullptr);
  REQUIRE(ifBranch->datatype == 0);

  auto *condlhs3 = dynamic_cast<VariableExprAST *>(value->lhs);
  auto *condrhs3 = dynamic_cast<VariableExprAST *>(value->rhs);
  REQUIRE(condlhs3 != nullptr);
  REQUIRE(condrhs3 != nullptr);

  REQUIRE(condlhs3->datatype == 0);
  REQUIRE(condlhs3->name == "x");
  REQUIRE(condrhs3->datatype == 0);
  REQUIRE(condrhs3->name == "y");
}

TEST_CASE("Testing if statement parsing error 1", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here there is a missing { after the else, we expect either and if or {
  // although right now else if is not supported
  lex.initFromString("if ( 3 + 1) { int x = 1 +1 ;}else int x = 2 + 2;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();
  auto res = parser.parseIfStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}

TEST_CASE("Testing if statement parsing error 2", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here else if construct which is not supported
  lex.initFromString("if ( 3 + 1) { int x = 1 +1 ;}else if {int x = 2 + 2;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();
  auto res = parser.parseIfStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code ==
          babycpp::diagnostic::IssueCode::UNEXPECTED_TOKEN_IN_EXPRESSION);
}

TEST_CASE("Testing if statement parsing error 3", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing } after if body
  lex.initFromString("if ( 3 + 1) { int x = 1 +1 ; else if {int x = 2 + 2;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();
  auto res = parser.parseIfStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}

TEST_CASE("Testing if statement parsing error 4", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing } after if body
  lex.initFromString("if ( 3 + 1)  int x = 1 +1 ;} else if {int x = 2 + 2;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();
  auto res = parser.parseIfStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}
TEST_CASE("Testing if statement parsing error 5", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing } after if body
  lex.initFromString("if ( 3 + 1 { int x = 1 +1 ;} else if {int x = 2 + 2;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();
  auto res = parser.parseIfStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}
TEST_CASE("Testing if statement parsing error 6", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing } after if body
  lex.initFromString(
      "if  3 + 1- 30) { int x = 1 +1 ;} else if {int x = 2 + 2;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();
  auto res = parser.parseIfStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}

TEST_CASE("Testing parsing correct for statement", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing } after if body
  lex.initFromString("for ( int i = 0; i < 20 ; i= i+1){ x = x + i;} ");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parseStatement();
  REQUIRE(res != nullptr);
}

TEST_CASE("Testing parsing for missing (", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing } after if body
  lex.initFromString("for  int i = 0; i < 20 ; i= i+1){ x = x + i;} ");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parseForStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}

TEST_CASE("Testing parsing for missing assigment in init", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing  assigment in init
  lex.initFromString("for ( int i  0; i < 20 ; i= i+1){ x = x + i;} ");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parseForStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}

TEST_CASE("Testing parsing for bad header 1 ", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing  assigment in init
  lex.initFromString("for(  i  0; i < 20 ; i= i+1){ x = x + i;} ");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parseForStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);
  auto err = parser.diagnostic->getError();
  REQUIRE(err.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}

TEST_CASE("Testing parsing for bad header 2", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing assigment in increment
  lex.initFromString("for(  i = 0; i < 20 ; i= ){ x = x + i;} ");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parseForStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 3);
  // here we pull out the 3rd error since is the one related
  // with the foor loop
  auto err1 = parser.diagnostic->getError();
  auto err2 = parser.diagnostic->getError();
  auto err3 = parser.diagnostic->getError();
  REQUIRE(err3.code == babycpp::diagnostic::IssueCode::FOR_LOOP_FAILURE);
}

TEST_CASE("Testing parsing for bad header 3", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing  ) at end of header
  lex.initFromString("for(  i = 0; i < 20 ; i= i +1 { x = x + i;} ");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parseForStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);

  auto err1 = parser.diagnostic->getError();
  REQUIRE(err1.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}
TEST_CASE("Testing parsing for bad header 4", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing  ) at start of body
  lex.initFromString("for(  i = 0; i < 20 ; i= i +1)  x = x + i;} ");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parseForStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);

  auto err1 = parser.diagnostic->getError();
  REQUIRE(err1.code == babycpp::diagnostic::IssueCode::EXPECTED_TOKEN);
}

TEST_CASE("Testing calling extern in function", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing  ) at start of body
  lex.initFromString("float testFunc(float a){"
                     "extern float cos(float a);"
                     "float x = cos(a); "
                     " return x;}");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parseStatement();
  REQUIRE(res != nullptr);
}
TEST_CASE("Testing pointer variable declaration parsing ", "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing  assigment in init
  lex.initFromString("int* myPtr = nullptr;");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto p = parser.parseStatement();
  REQUIRE(p != nullptr);

  auto *p_casted = dynamic_cast<VariableExprAST*>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->datatype == Token::tok_int);
  REQUIRE(p_casted->flags.isPointer== true);

  auto *v_casted = dynamic_cast<NumberExprAST*>(p_casted->value);
  REQUIRE(v_casted != nullptr);
  REQUIRE(v_casted->datatype == Token::tok_int);
  REQUIRE(v_casted->flags.isPointer== true);
  REQUIRE(v_casted->val.integerNumber == 0);

  //TODO(giordi) what to do with the nullptr????

}
TEST_CASE("Testing pointer variable declaration wrong operator parsing ",
          "[parser]") {

  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  // here missing  assigment in init
  lex.initFromString("int- myPtr = nullptr;");
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.gettok();

  auto res = parser.parseStatement();
  REQUIRE(res == nullptr);
  REQUIRE(parser.diagnostic->hasErrors() == 1);

  auto err1 = parser.diagnostic->getError();
  REQUIRE(err1.code == babycpp::diagnostic::IssueCode::UNEXPECTED_TOKEN_IN_DECLARATION);
}
