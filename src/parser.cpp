#include "parser.h"
#include "codegen.h"

#include <iostream>

namespace babycpp {
namespace parser {

// bool parseArguments(std::vector<Argument> &args);
using lexer::Token;
using codegen::ExprAST;
using codegen::NumberExprAST;
using codegen::VariableExprAST;
using codegen::CallExprAST;
using codegen::BinaryExprAST;
using codegen::PrototypeAST;
using codegen::FunctionAST;
using codegen::Argument;

const std::unordered_map<char, int> Parser::BIN_OP_PRECEDENCE = {
    {'<', 10}, {'+', 20}, {'-', 20}, {'*', 40}, {'/', 50}};

inline bool isDatatype(int tok) {
  return (tok == Token::tok_float || tok == Token::tok_int);
}
// this function defines whether or not a token is a declaration
// token or not, meaning defining an external function or datatype.
// interesting to think of casting as "anonymous declaration maybe?"
inline bool isDeclarationToken(int tok) {
  bool isDatatype = (tok == Token::tok_float) || (tok == Token::tok_int);
  bool isExtern = tok == Token::tok_extern;
  return isDatatype || isExtern;
}
inline int getTokPrecedence(Lexer *lex) {
  if (lex->currtok != Token::tok_operator) {
    // TODO(giordi) explain the -1 here
    return -1;
  }

  const char op = lex->identifierStr[0];
  auto iter = Parser::BIN_OP_PRECEDENCE.find(op);
  if (iter != Parser::BIN_OP_PRECEDENCE.end()) {
    return iter->second;
  }
  // should never reach this since all edge cases are taken by
  // the tok check
  return -1;
}

NumberExprAST *Parser::parseNumber() {
  if (lex->currtok != Token::tok_number) {
    return nullptr;
  }
  auto *node = new NumberExprAST(lex->value);
  lex->gettok(); // eating the number;
  return node;
}

ExprAST *Parser::parseIdentifier() {

  // if (isDeclarationToken(lex->currtok)) {
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
  if (lex->currtok == Token::tok_assigment_operator) {
    if (!flags.processed_assigment) {

      flags.processed_assigment = true;
      lex->gettok(); // eating assignment operator;

      auto *RHS = parseExpression();
      return new VariableExprAST(lex->identifierStr, RHS, 0);
    }

    std::cout << "error,  cannot have multiple assignment in a statement"
              << std::endl;
    return nullptr;
  }
  return parseBinOpRHS(0, LHS);
}

ExprAST *Parser::parseBinOpRHS(int givenPrec, ExprAST *LHS) {
  while (true) {
    int tokPrec = getTokPrecedence(lex);

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
    int nextPrec = getTokPrecedence(lex);
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
  }
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
      // TODO(giordi) parse function properly
      return parseFunction();
    case Token::tok_assigment_operator: {
      // this can be, a direct value assignment
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

  ExprAST *exp = nullptr;
  if (lex->currtok == Token::tok_eof) {
    return nullptr;
  }
  if (lex->currtok == Token::tok_extern) {
    exp = parseExtern();
  }
  if (lex->currtok == Token::tok_return) {
    lex->gettok(); // eat return
    exp = parseExpression();
    exp->flags.isReturn = true;
  }

  if (isDeclarationToken(lex->currtok)) {
    exp = parseDeclaration();
  }

  if (lex->currtok == Token::tok_identifier) {
    exp = parseExpression();
  }
  // TODO(giordi) support statement starting with parenthesis
  // if (lex->currtok == Token::tok_open_paren){}

  if (lex->currtok != Token::tok_end_statement) {
    std::cout << "expecting semicolon at end of statement" << std::endl;
    return exp;
  }

  lex->gettok(); // eating semicolon;
  // clearing flags, if they start to increase i will
  // change this to a clear flags
  flags.processed_assigment = false;
  return exp;
}

PrototypeAST *Parser::parseExtern() {
  // eating extern token;
  lex->gettok();
  if (!isDatatype(lex->currtok)) {
    std::cout << "expected return data type after extern" << std::endl;
    return nullptr;
  }
  return parsePrototype();
}

bool parseArguments(Lexer *lex, std::vector<Argument> &args) {
  int datatype;
  std::string argName;
  while (true) {
    if (lex->currtok == Token::tok_close_round) {
      // no args or done with args
      lex->gettok(); // eat )
      return true;
    }
    // we expect to see seq of data_type identifier comma
    if (!isDatatype(lex->currtok)) {
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
        std::cout << "expected data type identifier after comma" << std::endl;
        return false;
      }
    }
    // if we got here we have a sanitized argument
    args.emplace_back(Argument(datatype, argName));
  }
  return true;
}

FunctionAST *Parser::parseFunction() {
  // we start by parsing the prototype;
  auto *proto = parsePrototype();
  if (proto == nullptr) {
    // error should be handled by parseProto function;
    return nullptr;
  }
  // setting the prototype as not extern
  proto->isExtern = false;

  // we expect an open curly brace starting the body of the function
  if (lex->currtok != Token::tok_open_curly) {
    std::cout << "error expected { after function prototype" << std::endl;
    return nullptr;
  }

  lex->gettok(); // eating curly
  int SECURITY = 2000;
  int counter = 0;
  std::vector<ExprAST *> statements;
  while (lex->currtok != Token::tok_close_curly && counter < SECURITY) {
    auto *curr_statement = parseStatement();
    if (curr_statement == nullptr) {
      // error should be handled by parse statement;
      return nullptr;
    }
    statements.push_back(curr_statement);

    // check if we are at end of file
    if (lex->currtok == Token::tok_eof) {
      std::cout << "error, expected } got EOF" << std::endl;
      return nullptr;
    }
  }

  // here we should have all the statements, expected }
  // if not it means something went wrong or we hit the
  // security limit
  if (lex->currtok != Token::tok_close_curly) {
    std::cout << "error: expected } at end of function body" << std::endl;
    return nullptr;
  }

  lex->gettok(); // eat }
  return new FunctionAST(proto, statements);
}

ExprAST *Parser::parseParen() {
  lex->gettok(); // eating paren
  auto *exp = parseExpression();
  if (lex->currtok != Token::tok_close_round) {
    std::cout << "error:, expected close paren after expression";
    return nullptr;
  }
  lex->gettok(); // eating )
  return exp;
}

PrototypeAST *Parser::parsePrototype() {

  if (!isDatatype(lex->currtok)) {
    std::cout << "expected return data type after extern" << std::endl;
    return nullptr;
  }
  int datatype = lex->currtok;
  lex->gettok(); // eating datatype

  if (lex->currtok != Token::tok_identifier) {
    std::cout << "expected identifier after extern return datatype"
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
  if (!parseArguments(lex, args)) {
    // no need to log error, error already logged
    return nullptr;
  }
  // need to check semicolon at the end;
  // here we can generate the prototype node;
  return new PrototypeAST(datatype, funName, args, true);
}
//////////////////////////////////////////////////
//// CODE GEN
//////////////////////////////////////////////////

} // namespace parser
} // namespace babycpp
