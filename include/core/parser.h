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
} // namespace codegen

namespace memory {
struct FactoryAST;
}

namespace parser {

using lexer::Lexer;
using lexer::Number;
using lexer::Token;

/** @brief set of flags representing the status of the parser */
struct ParserFlags {
  bool processed_assigment : 1;
};

/**
 * @brief struct in charge of parsing the stream of tokens
 */
struct Parser {
  /**
   * @brief construtor
   * @param inputLexer: pointer to the lexer
   * @param inputfactory: pointer to the factory class in charge of
   *                      allocating the AST nodes
   */
  explicit Parser(Lexer *inputLexer, memory::FactoryAST *inputfactory,
                  diagnostic::Diagnostic *indiagnostic)
      : lex(inputLexer), factory(inputfactory), diagnostic(indiagnostic) {
    flags.processed_assigment = false;
  }

  codegen::NumberExprAST *parseNumber();
  codegen::ExprAST *parseIdentifier();
  /**
   * @brief parses an expression
   * Expressions is a high level concepts, not necesserly in the
   * mathematical meaning. An expression can be a function call,
   * an assigment a bin op etc
   * @return ExprAST* pointer to the top of the expression subtree
   */
  codegen::ExprAST *parseExpression();
  /**
   * @brief parses a binary operations
   * @param givenPrec: this is the current precedens of the operator
   *                   alraedy processed, in case is the first call
   *                   in the stack a -1 should be passed, in case
   *                   of chained binary operations, the previously
   *                   parsed operator precedence should be passed,
   *                   so that correct precendence is handled
   * @param LHS: pointer to the left hand side of the operation which
   *             has been already processed, that can be a sub operation
   *             in case of chained operations
   * @return : pointer to the AST node representing the RHS of the op
   */
  codegen::ExprAST *parseBinOpRHS(int givenPrec, codegen::ExprAST *LHS);

  /**@brief parses the leftmost part of an expression, or sub expression */
  codegen::ExprAST *parsePrimary();

  /** @brief by statment we mean an expression with complete sense that
   * can stand on its own, meaning a valid expression
   * being ";" terminated. For example you can think of the body
   * of a function as an array of statements
   * @return: pointer to the AST node representing the statement
   */
  codegen::ExprAST *parseStatement();
  codegen::PrototypeAST *parseExtern();
  codegen::FunctionAST *parseFunction();
  /**@brief parses a function declaration, just the signature
   * @return pointer to the AST node representing the signature
   */
  codegen::PrototypeAST *parsePrototype();
  /**
   * @brief function called whenever a declaration token is found
   * This can be either a function declaration, a variable, class
   * declaration in the future etc
   * @return: pointer to the AST node representing the declaration
   */
  codegen::ExprAST *parseDeclaration();
  /**@brief parses an expression wrapped in parenthesis */
  codegen::ExprAST *parseParen();

  /**@brief parses an if statement and corresponding branches */
  codegen::ExprAST *parseIfStatement();

  /**@brief parses a for loop statement and corresponding body*/
  codegen::ExprAST *parseForStatement();

  /**@brief parses a null pointer return a NumberExpression as 0 and ptr which
   * identifies nullptr */
  codegen::NumberExprAST *parseNullptr();

  /**@brief parses a statement which involves an assigment, both LHS anr RHS*/
  codegen::ExprAST *parseAssigment();

  /**@brief parses a statement which involves a pointer dereference*/
  codegen::ExprAST *parseDereference();

  /**@brief parses an assiment to wherver the pointer is pointing to*/
  codegen::ExprAST *parseToPointerAssigment();

  /**@brief this function parses a casts which can be either datatype or pointer
   * cast*/
  codegen::ExprAST *parseCast();

  /** @brief constant map representing the different operators precedences
   *  a higher positive number represents an higher precedence
   */
  const static std::unordered_map<char, int> BIN_OP_PRECEDENCE;

  // UTILITY
  static inline bool isDatatype(int tok) {
    return ((tok == Token::tok_float) | (tok == Token::tok_int) |
            (tok == Token::tok_void_ptr));
  }

  /**@brief utiltiy function telling us if the given token is part
   * of a declaration.
   * By declaration we mean like a prototype declaration or variable
   * declaration
   * @param tok: token to be processed
   * @return: whether or not the token represents a declaration
   */
  static inline bool isDeclarationToken(int tok) {
    bool isDatatype = Parser::isDatatype(tok);
    bool isExtern = tok == Token::tok_extern;
    return isDatatype | isExtern;
  }
  // data
  Lexer *lex;
  memory::FactoryAST *factory;
  diagnostic::Diagnostic *diagnostic;
  ParserFlags flags;
};

} // namespace parser
} // namespace babycpp
