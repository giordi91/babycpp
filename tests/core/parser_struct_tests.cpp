#include "catch.hpp"
#include <iostream>

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
using babycpp::codegen::CastAST;
using babycpp::codegen::DereferenceAST;
using babycpp::codegen::ExprAST;
using babycpp::codegen::FunctionAST;
using babycpp::codegen::IfAST;
using babycpp::codegen::NumberExprAST;
using babycpp::codegen::PrototypeAST;
using babycpp::codegen::StructAST;
using babycpp::codegen::StructMemberAST;
using babycpp::codegen::ToPointerAssigmentAST;
using babycpp::codegen::VariableExprAST;

static babycpp::memory::FactoryAST factory;

static babycpp::diagnostic::Diagnostic diagnosticParserTests;

static void checkParserErrors() {
  if (diagnosticParserTests.hasErrors()) {
    std::cout << diagnosticParserTests.printAll() << std::endl;
  }
}

TEST_CASE("Testing initial struct parsing", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.initFromString("struct MyStruct{ float x; int y; float z;}");
  // getting first token
  lex.gettok();

  auto *p = parser.parseStruct();
  checkParserErrors();
  REQUIRE(p != nullptr);
  auto *p_casted = dynamic_cast<StructAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->identifierName == "MyStruct");
  REQUIRE(p_casted->members.size() == 3);
  REQUIRE(p_casted->members[0]->datatype == Token::tok_float);
  REQUIRE(p_casted->members[0]->identifierName == "x");

  REQUIRE(p_casted->members[1]->datatype == Token::tok_int);
  REQUIRE(p_casted->members[1]->identifierName == "y");

  REQUIRE(p_casted->members[2]->datatype == Token::tok_float);
  REQUIRE(p_casted->members[2]->identifierName == "z");
}

TEST_CASE("Testing initial struct parsing using top parser", "[parser]") {
  diagnosticParserTests.clear();
  Lexer lex(&diagnosticParserTests);
  Parser parser(&lex, &factory, &diagnosticParserTests);
  lex.initFromString("struct Vector{ float x; int* y; float*z;float w;}");
  // getting first token
  lex.gettok();

  auto *p = parser.parseStatement();
  checkParserErrors();
  REQUIRE(p != nullptr);
  auto *p_casted = dynamic_cast<StructAST *>(p);
  REQUIRE(p_casted != nullptr);
  REQUIRE(p_casted->identifierName == "Vector");
  REQUIRE(p_casted->members.size() == 4);
  REQUIRE(p_casted->members[0]->datatype == Token::tok_float);
  REQUIRE(p_casted->members[0]->identifierName == "x");
  REQUIRE(p_casted->members[0]->flags.isPointer == false);

  REQUIRE(p_casted->members[1]->datatype == Token::tok_int);
  REQUIRE(p_casted->members[1]->identifierName == "y");
  REQUIRE(p_casted->members[1]->flags.isPointer == true);

  REQUIRE(p_casted->members[2]->datatype == Token::tok_float);
  REQUIRE(p_casted->members[2]->identifierName == "z");
  REQUIRE(p_casted->members[2]->flags.isPointer == true);

  REQUIRE(p_casted->members[3]->datatype == Token::tok_float);
  REQUIRE(p_casted->members[3]->identifierName == "w");
  REQUIRE(p_casted->members[3]->flags.isPointer == false);
}

TEST_CASE("Testing struct and struct instantiation", "[parser]") {
	diagnosticParserTests.clear();
	Lexer lex(&diagnosticParserTests);
	Parser parser(&lex, &factory, &diagnosticParserTests);
	lex.initFromString(
		"struct Vector{ float x; int* y; float*z;float w;}"
		"int testFunc(int i) { Vector myStruct; int res = i; return res;}");
	// getting first token
	lex.gettok();
	auto *p = parser.parseStatement();
	checkParserErrors();
	REQUIRE(p != nullptr);
	//parsing the function
	p = parser.parseStatement();
	checkParserErrors();
	REQUIRE(p != nullptr);

}