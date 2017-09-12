#include "parser.h"

#include <iostream>

namespace babycpp {
namespace parser {

using lexer::Token;

const std::unordered_map<char, int> Parser::BIN_OP_PRECEDENCE = {
    {'<', 10}, {'+', 20}, {'-', 20}, {'*', 40}, {'/', 50}};

NumberExprAST *Parser::parseNumber() {
  if (lex->currtok != Token::tok_number) {
    return nullptr;
  }
  auto *node = new NumberExprAST(lex->value);
  lex->gettok(); // eating the number;
  return node;
}

ExprAST *Parser::parseIdentifier() {
  const std::string idstr = lex->identifierStr;
  // look ahead and eat identifier;
  lex->gettok();

  int tok = lex->currtok;
  if (tok != Token::tok_open_round) {
    // fix this!! need to know what the variable type is
    // or if is being referenced
    return new VariableExprAST(idstr);
  }
  lex->gettok(); // eating paren;
  std::vector<ExprAST *> args;
  if (tok == Token::tok_close_round) {
    // must be a function call we eat until we find the
    // corresponding closing paren
    while (true) {
      ExprAST *arg = parseExpression();
      if (arg == nullptr) {
        return nullptr;
      }
      args.push_back(arg);
      if (lex->currtok == Token::tok_close_curly) {
        break;
      }

      if (lex->currtok != Token::tok_comma) {
        std::cout << "Error: expected ')' or , in function call" << std::endl;
        return nullptr;
      }

      // moving forward and eating tok
      lex->gettok();
    }
  }
  return new CallExprAST(idstr, args);
}

ExprAST *Parser::parseExpression() {
  ExprAST *LHS = parsePrimary();
  if (LHS == nullptr) {
    return nullptr;
  }
  return parseBinOpRHS(0, LHS);
}

ExprAST *Parser::parseBinOpRHS(int givenPrec, ExprAST *LHS) {
  while (true) {
    int tokPrec = getTokPrecedence();

    // if this bin op is less then the current one  we eat it;
    if (tokPrec < givenPrec) {
      return LHS;
    }

    int binOp = lex->currtok;
    lex->gettok();
    ExprAST *RHS = parsePrimary();
    if (RHS == nullptr) {
      return nullptr;
    }

    // if the bin op binds less tightly with RHS than the OP
    // afer RHS, let the pending op take RHS its LHS
    int nextPrec = getTokPrecedence();
    if (tokPrec < nextPrec) {
      RHS = parseBinOpRHS(tokPrec + 1, RHS);
      if (RHS == nullptr) {
        return nullptr;
      }
    }

	LHS = new BinaryExprAST(binOp, LHS, RHS);
  }
}

ExprAST *Parser::parsePrimary() {
  switch (lex->currtok) {
  default: {
    std::cout << "unknow token when expecting expression" << std::endl;
    return nullptr;
  }
  case Token::tok_identifier: {
    return parseIdentifier();
  }
  case Token::tok_number: {
    return parseNumber();
  }
  }
  return nullptr;
}

int Parser::getTokPrecedence() {
  if (lex->currtok != Token::tok_operator) {
    std::cout << "error given token is not an operator, cannot get precednece";
    return -1;
  }

  const char op = lex->identifierStr[0];
  auto iter = BIN_OP_PRECEDENCE.find(op);
  if (iter != BIN_OP_PRECEDENCE.end()) {
    return iter->second;
  }
  // should never reach this since all edge cases are taken by
  // the tok check
  return -1;
}

} // namespace parser
} // namespace babycpp
