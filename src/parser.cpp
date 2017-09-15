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

  // if (isDeclarationToken()) {
  //}
  const std::string idstr = lex->identifierStr;
  // look ahead and eat identifier;
  lex->gettok();

  int tok = lex->currtok;
  if (tok != Token::tok_open_round) {
    // fix this!! need to know what the variable type is
    // or if is being referenced
    return new VariableExprAST(idstr, nullptr, 0);
  }
  lex->gettok(); // eating paren;
  std::vector<ExprAST *> args;
  if (lex->currtok != Token::tok_close_round) {
    // must be a function call we eat until we find the
    // corresponding closing paren
    while (true) {
      ExprAST *arg = parseExpression();
      if (arg == nullptr) {
        return nullptr;
      }
      args.push_back(arg);
      if (lex->currtok == Token::tok_close_round) {
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

    std::string op = lex->identifierStr;
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

    LHS = new BinaryExprAST(op, LHS, RHS);
  }
}

ExprAST *Parser::parsePrimary() {
  switch (lex->currtok) {
  default: {
    std::cout << "unknown token when expecting expression" << std::endl;
    return nullptr;
  }
  case Token::tok_identifier: {
    return parseIdentifier();
  }
  case Token::tok_number: {
    return parseNumber();
  }
  case Token::tok_open_round: {
    return parseParen();
  }
    // need to parse parens
    // case Token::tok_open_round
  }
}

int Parser::getTokPrecedence() {
  if (lex->currtok != Token::tok_operator) {
    // TODO explaing the -1 here
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

ExprAST *Parser::parseDeclaration() {
  // if we have a declaration token, something like int, flaot etc we might have
  // several cases like, we might have a variable definition, we might have
  // a function definition etc, this require a bit of look ahead!
  int datatype = lex->currtok;
  lex->gettok(); // eating datatype
  // looking ahead 2 tokens, which should give us the identifier
  // and the next token

  // const lexer::MovableToken& nextTok= lex->lookAheadToken[0];
  if (lex->currtok == Token::tok_identifier) {
    std::string identifier = lex->identifierStr;
    lex->gettok(); // eat identifier;
    // we got an identifier great, now the next token will
    // tell us wheter is a function prototype or a variable
    switch (lex->currtok) {
    default: {
      // error
      return nullptr;
    }
    case Token::tok_open_curly:
      // TODO PARSE PROPERLY FUNCTION
      return parseFunction();
    case Token::tok_assigment_operator: {
      // this can be, a direct value assigment
      // like int x = 10;
      // it can be a math expression like :
      // int x = 10 + y;
      // it can be a function call
      // int x = getMagicNumber();
      lex->gettok(); // eat = operator
      auto *RHS = parseExpression();
      return new VariableExprAST(identifier, RHS, datatype);
    }
      // not supported yet
      // case Token::tok_end_statement:
      //    return parseVariableDefinition();
    }

    // we got here so we parsed something
  }

  // error
  return nullptr;
}

ExprAST *Parser::parseStatement() {
  if (lex->currtok == Token::tok_extern) {
    return parseExtern();
  }

  if (isDeclarationToken()) {
    return parseDeclaration();

    // error
    return nullptr;
  }
  return nullptr;
}

PrototypeAST *Parser::parseExtern() {
  // eating extern token;
  lex->gettok();
  if (!isDatatype()) {
    std::cout << "expected return data type after extern" << std::endl;
    return nullptr;
  }
  int datatype = lex->currtok;
  lex->gettok(); // eating datatype

  if (lex->currtok != Token::tok_identifier) {
    std::cout << "expected idnetifier after extern return datatype"
              << std::endl;
    return nullptr;
  }
  // we know that we need a function call so we get started
  std::string funName = lex->identifierStr;
  lex->gettok(); // eat identifier

  if (lex->currtok != Token::tok_open_round) {
    std::cout << "expected ( after extern function name" << std::endl;
    // should i eat bad identifier here?
    return nullptr;
  }
  // parsing arguments
  lex->gettok(); // eat parenthesis
  std::vector<Argument> args;
  if (!parseArguments(args)) {
    // no need to log error, error already logged
    return nullptr;
  }
  // need to check semicolon at the end;
  // here we cangenerate the function node;
  return new PrototypeAST(datatype, funName, args, true);
}

bool Parser::parseArguments(std::vector<Argument> &args) {
  int datatype;
  std::string argName;
  while (true) {
    if (lex->currtok == Token::tok_close_round) {
      // no args
      return true;
    }
    // we expect to see seq of data_type identifier comma
    if (!isDatatype()) {
      std::cout << "expected data type identifier for argument" << std::endl;
      return false;
    }
    // saving datatype
    datatype = lex->currtok;
    lex->gettok(); // eat datatype

    if (lex->currtok != Token::tok_identifier) {
      std::cout << "expected identifier name for argument" << std::endl;
      return false;
    }

    // storing name
    argName = lex->identifierStr;
    lex->gettok(); // eating identifier name

    // finally checking if we have a comma or paren
    if (lex->currtok == Token::tok_comma) {
      lex->gettok(); // eating the comma
      // checking if we have a paren if so we  have
      // an error
      if (lex->currtok == Token::tok_close_curly) {
        std::cout << "expected data type idnetifier after comma" << std::endl;
        return false;
      }
    }
    // if we got here we have a sanitized argument
    args.emplace_back(Argument(datatype, argName));
  }
  return true;
}

ExprAST *Parser::parseFunction() { return nullptr; }

ExprAST *Parser::parseVariableDefinition() { return nullptr; }

bool Parser::isDeclarationToken() {
  int tok = lex->currtok;
  int isDatatype = tok == Token::tok_float || tok == Token::tok_int;
  int isExtern = tok == Token::tok_extern;
  return isDatatype || isExtern;
}

bool Parser::isDatatype() {
  int tok = lex->currtok;
  return (tok == Token::tok_float || tok == Token::tok_int);
}

ExprAST *Parser::parseParen() {
  lex->gettok();//eating paren
  auto* exp = parseExpression();
  if(lex->currtok != Token::tok_close_round)
  {
    std::cout<<"error:, expected close parent after expression";
    return nullptr;
  }
  return exp;
}

} // namespace parser
} // namespace babycpp
