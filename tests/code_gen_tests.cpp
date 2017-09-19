#include <iostream>

#include "catch.hpp"

#include <codegen.h>

using babycpp::codegen::Codegenerator;
using babycpp::lexer::Lexer;
using babycpp::lexer::Token;
using babycpp::parser::Parser;

TEST_CASE("Testing code gen number float", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("1.0");

  auto *node = gen.parser.parseNumber();
  REQUIRE(node != nullptr);

  auto *val = node->codegen(&gen);
  REQUIRE(val != nullptr);

  std::string outs = gen.printLlvmData(val);
  REQUIRE(outs == "float 1.000000e+00");
}
TEST_CASE("Testing code gen number int", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("39");

  auto *node = gen.parser.parseNumber();
  REQUIRE(node != nullptr);

  auto *val = node->codegen(&gen);
  REQUIRE(val != nullptr);

  std::string outs = gen.printLlvmData(val);
  REQUIRE(outs == "i32 39");
}

TEST_CASE("Testing number variable ref gen", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("x");

  // creating an argument called X in database
  llvm::Argument a{llvm::Type::getFloatTy(gen.context)};
  a.setName("x");
  gen.namedValues[a.getName().str()] = &a;

  // doing the parsing
  auto *p = gen.parser.parseIdentifier();
  REQUIRE(p != nullptr);
  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  REQUIRE(outs == "float %x");
}

TEST_CASE("Testing binop codegen plus", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(x+2.0)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);

  // adding x variable
  llvm::Argument a{llvm::Type::getFloatTy(gen.context)};
  a.setName("x");
  gen.namedValues[a.getName().str()] = &a;
  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  REQUIRE(outs == "  %addtmp = fadd float %x, 2.000000e+00");
}

TEST_CASE("Testing binop codegen -", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(yy-2.0)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);

  // adding x variable
  llvm::Argument a{llvm::Type::getFloatTy(gen.context)};
  a.setName("yy");
  gen.namedValues[a.getName().str()] = &a;
  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  REQUIRE(outs == "  %subtmp = fsub float %yy, 2.000000e+00");
}

TEST_CASE("Testing binop codegen *", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(temp*4.0)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);

  // adding x variable
  llvm::Argument a{llvm::Type::getFloatTy(gen.context)};
  a.setName("temp");
  gen.namedValues[a.getName().str()] = &a;
  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  REQUIRE(outs == "  %multmp = fmul float %temp, 4.000000e+00");
}

TEST_CASE("Testing binop codegen /", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(temp /10.0)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);

  // adding x variable
  llvm::Argument a{llvm::Type::getFloatTy(gen.context)};
  a.setName("temp");
  gen.namedValues[a.getName().str()] = &a;
  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  REQUIRE(outs == "  %divtmp = fdiv float %temp, 1.000000e+01");
}
TEST_CASE("Testing binop codegen <", "[codegen]") {
  Codegenerator gen;
  gen.initFromString("(z<13.0)");

  auto p = gen.parser.parseParen();
  REQUIRE(p != nullptr);

  // adding x variable
  llvm::Argument a{llvm::Type::getFloatTy(gen.context)};
  a.setName("z");
  gen.namedValues[a.getName().str()] = &a;
  llvm::Value *v = p->codegen(&gen);
  REQUIRE(v != nullptr);

  // converting Value to string and check result
  std::string outs = gen.printLlvmData(v);
  // here we are only checking the last result, where we convert from bool to
  // double  since we only support doubles, when we will have a function the whole
  // body  will be there
  REQUIRE(outs == "  %booltmp = uitofp i1 %cmptmp to double");
}

TEST_CASE("Testing function codegen simple add", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("float test(float x){ return x+1.0;}");
  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  std::string expected{"\ndefine float @test(float %x) {\nentry:\n  %addtmp = "
                       "fadd float %x, 1.000000e+00\n  ret float %addtmp\n}\n"};

  REQUIRE(outs == expected);
}
TEST_CASE("Testing function codegen mutable variable", "[codegen]") {

	Codegenerator gen;
	gen.initFromString("float complexAdd(float x, int y){ return x+y;}");
	auto p = gen.parser.parseFunction();
	REQUIRE(p != nullptr);

	auto v = p->codegen(&gen);
	REQUIRE(v != nullptr);
	std::string outs = gen.printLlvmData(v);
	std::string expected = 
		"\ndefine float @complexAdd(float %x, i32 %y) {\n"
		"entry:\n"
		"  %intToFPcast = uitofp i32 %y to float\n"
		"  %addtmp = fadd float %x, %intToFPcast\n"
		"  ret float %addtmp\n"
		"}\n";
  REQUIRE(outs == expected);
}

TEST_CASE("Testing function codegen conversion", "[codegen]") {

  Codegenerator gen;
  gen.initFromString("float average(float a, float b) \n { \n"
                         "float avg = (a+b)/2.0; return avg;}");

  auto p = gen.parser.parseFunction();
  REQUIRE(p != nullptr);

  auto v = p->codegen(&gen);
  REQUIRE(v != nullptr);
  std::string outs = gen.printLlvmData(v);
  std::cout<<outs<<std::endl;
}
