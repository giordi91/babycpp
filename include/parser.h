#pragma once
#include "lexer.h"

#include <unordered_map>

namespace babycpp {

// forward declare from codegen
namespace codegen {
struct ExprAST;
struct NumberExprAST;
struct PrototypeAST;
struct FunctionAST;
}

namespace parser {

using lexer::Lexer;
using lexer::Number;

struct ParserFlags {
  bool processed_assigment : 1;
};

struct Parser {
  explicit Parser(Lexer *inputLexer) : lex(inputLexer) {
    flags.processed_assigment = false;
  }

  codegen::NumberExprAST *parseNumber();
  codegen::ExprAST *parseIdentifier();
  codegen::ExprAST *parseExpression();
  codegen::ExprAST *parseBinOpRHS(int givenPrec, codegen::ExprAST *LHS);
  codegen::ExprAST *parsePrimary();
  codegen::ExprAST *parseStatement();
  codegen::PrototypeAST *parseExtern();
  codegen::FunctionAST *parseFunction();
  codegen::PrototypeAST *parsePrototype();
  codegen::ExprAST *parseDeclaration();
  codegen::ExprAST *parseParen();
  const static std::unordered_map<char, int> BIN_OP_PRECEDENCE;
  Lexer *lex;
  ParserFlags flags;
};

} // namespace parser
} // namespace babycpp
