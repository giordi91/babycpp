#include "parser.h"
#include "codegen.h"

#include <iostream>

namespace babycpp {
namespace parser {

using codegen::Argument;
using codegen::ExprAST;
using codegen::FunctionAST;
using codegen::NumberExprAST;
using codegen::PrototypeAST;
using codegen::VariableExprAST;
using diagnostic::IssueCode;
using lexer::Token;

const std::unordered_map<char, int> Parser::BIN_OP_PRECEDENCE = {
    {'<', 10}, {'+', 20}, {'-', 20}, {'*', 40}, {'/', 50}};

// ERROR LOGGING
inline void logParserError(const std::string &msg, Lexer *lexer,
                           IssueCode code) {

  diagnostic::Issue err{msg, lexer->lineNumber, lexer->columnNumber,
                        diagnostic::IssueType::PARSER, code};
  lexer->diagnostic->pushError(err);
}

inline void logParserError(const std::string &msg, Parser *parser,
                           IssueCode code) {

  diagnostic::Issue err{msg, parser->lex->lineNumber, parser->lex->columnNumber,
                        diagnostic::IssueType::PARSER, code};
  parser->diagnostic->pushError(err);
}

bool parseStatementsUntillCurly(std::vector<ExprAST *> *statements,
                                Parser *parser, bool processingIf = false) {

  Lexer *lex = parser->lex;
  const int SECURITY = 2000;
  int counter = 0;
  while ((static_cast<int>(lex->currtok != Token::tok_close_curly) &
          static_cast<int>(counter < SECURITY)) != 0) {
    // there might be the case where you have a else statement before anything
    // else  this should not be allowed.If the number of not supported stand
    // alone keywords incresases we might  have to change this check to one more
    // appropriate
    if (lex->currtok == Token::tok_else && processingIf) {
      logParserError("error, expected } got else statement, please close the "
                     "curly bracket",
                     parser, IssueCode::EXPECTED_TOKEN);
      return false;
    }
    auto *curr_statement = parser->parseStatement();
    if (curr_statement == nullptr) {
      // error should be handled by parse statement;
      return false;
    }
    statements->push_back(curr_statement);

    // check if we are at end of file
    if (lex->currtok == Token::tok_eof) {
      logParserError("error, expected } got EOF", parser,
                     IssueCode::EXPECTED_TOKEN);
      return false;
    }
  }

  // here we should have all the statements, expected }
  // if not it means something went wrong or we hit the
  // security limit
  if (lex->currtok != Token::tok_close_curly) {
    logParserError("expected } at end of function bodygot:" +
                       std::to_string(lex->currtok),
                   parser, IssueCode::EXPECTED_TOKEN);
    return false;
  }
  return true;
}

codegen::StructMemberAST *parseStructMemberDeclaration(Parser *parser) {
  Lexer *lex = parser->lex;
  if (!Parser::isDatatype(lex->currtok)) {
    logParserError("expected datatype in struct memebr got:" +
                       std::to_string(lex->currtok),
                   parser, IssueCode::UNEXPECTED_TOKEN_IN_STRUCT);
    return nullptr;
  }

  int datatype = lex->currtok;
  bool isPointer = false;

  lex->gettok(); // eating datatype
  if (lex->currtok == Token::tok_operator && lex->identifierStr == "*") {
    isPointer = true;
    lex->gettok(); // eating *
  }

  if (lex->currtok != Token::tok_identifier) {
    logParserError("expected identifier in struct memebr got:" +
                       std::to_string(lex->currtok),
                   parser, IssueCode::UNEXPECTED_TOKEN_IN_STRUCT);
    return nullptr;
  }
  std::string identifierName = lex->identifierStr;
  lex->gettok(); // eating identifier;

  if (lex->currtok != Token::tok_end_statement) {
    logParserError("expected end of statment ; at end of struct memeber got:" +
                       std::to_string(lex->currtok),
                   parser, IssueCode::UNEXPECTED_TOKEN_IN_STRUCT);
    return nullptr;
  }
  lex->gettok(); // eat ;

  return parser->factory->allocStructMemberAST(datatype, isPointer,
                                               identifierName);
}

bool parseDeclarationsUntilClosedCurly(
    std::vector<codegen::StructMemberAST *> *statements, Parser *parser) {

  Lexer *lex = parser->lex;
  const int SECURITY = 2000;
  int counter = 0;
  while ((static_cast<int>(lex->currtok != Token::tok_close_curly) &
          static_cast<int>(counter < SECURITY)) != 0) {

    auto *curr_statement = parseStructMemberDeclaration(parser);
    if (curr_statement == nullptr) {
      // error should be handled by parse statement;
      return false;
    }
    statements->push_back(curr_statement);

    // check if we are at end of file
    if (lex->currtok == Token::tok_eof) {
      logParserError("error, expected } got EOF", parser,
                     IssueCode::EXPECTED_TOKEN);
      return false;
    }
  }

  // here we should have all the statements, expected }
  // if not it means something went wrong or we hit the
  // security limit
  if (lex->currtok != Token::tok_close_curly) {
    logParserError("expected } at end of function bodygot:" +
                       std::to_string(lex->currtok),
                   parser, IssueCode::EXPECTED_TOKEN);
    return false;
  }
  return true;
}

inline bool isPointerCast(Lexer *lex) {

  // this function expects 3 look ahead tokens
  return (Parser::isDatatype(lex->lookAheadToken[0].token) &
          (lex->lookAheadToken[1].token == Token::tok_operator) &
          (lex->lookAheadToken[1].identifierStr == "*") &
          (lex->lookAheadToken[2].token == Token::tok_close_round));
}
inline bool isDataCast(Lexer *lex) {
  // this function expects 2 look ahead tokens
  return (Parser::isDatatype(lex->lookAheadToken[0].token) &
          (lex->lookAheadToken[1].token == Token::tok_close_round));
}

// this call assumes the lookahead to be already done
inline bool isCastOperation(Lexer *lex) {

  return (isPointerCast(lex) | isDataCast(lex));
}

// this function defines whether or not a token is a declaration
// token or not, meaning defining an external function or datatype.
// interesting to think of casting as "anonymous declaration maybe?"
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

// PARSING
NumberExprAST *Parser::parseNumber() {
  if (lex->currtok != Token::tok_number) {
    return nullptr;
  }
  auto *node = factory->allocNuberAST(lex->value);
  lex->gettok(); // eating the number;
  return node;
}

ExprAST *Parser::parseIdentifier() {
  const std::string idstr = lex->identifierStr;
  // look ahead and eat identifier;
  lex->gettok();

  int tok = lex->currtok;
  if (tok != Token::tok_open_round) {
    return factory->allocVariableAST(idstr, nullptr, 0);
  }
  lex->gettok(); // eating paren;
  std::vector<ExprAST *> args;
  if (lex->currtok != Token::tok_close_round) {
    // must be a function call we eat until we find the
    // corresponding closing paren
    while (true) {
      ExprAST *arg = parseExpression();
      if (arg == nullptr) {
        logParserError("expected argument after comma in function call got:" +
                           std::to_string(lex->currtok),
                       lex, IssueCode::MISSING_ARG_IN_FUNC_CALL);
        return nullptr;
      }
      args.push_back(arg);
      if (lex->currtok == Token::tok_close_round) {
        break;
      }

      if (lex->currtok != Token::tok_comma) {
        logParserError("expected ')' or , in function call got:" +
                           std::to_string(lex->currtok),
                       lex,
                       IssueCode::MISSING_OPEN_ROUND_OR_COMMA_IN_FUNC_CALL);

        return nullptr;
      }

      // moving forward and eating tok
      lex->gettok();
    }
  }
  lex->gettok(); // eat )
  return factory->allocCallexprAST(idstr, args);
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
      if (RHS == nullptr) {
        logParserError("expected valid expression of RHS of assigment operator",
                       lex, IssueCode::EXPECTED_VARIABLE);
        return nullptr;
      }
      if (LHS->nodetype != codegen::VariableNode) {

        logParserError("LHS of assigment operator must be a variable got:" +
                           std::to_string(lex->currtok),
                       lex, IssueCode::EXPECTED_VARIABLE);
        return nullptr;
      }

      // setting the right hand side as value;
      auto *LHScasted = static_cast<VariableExprAST *>(LHS);
      LHScasted->value = RHS;
      return LHS;
    }

    // having hard time to replicate this, is mostly prevented by other errors
    // check if i find a way to trigger it i ll add a test for it
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
    // after RHS, let the pending op take RHS its LHS
    int nextPrec = getTokPrecedence(lex);
    if (tokPrec < nextPrec) {
      RHS = parseBinOpRHS(tokPrec + 1, RHS);
      if (RHS == nullptr) {
        return nullptr;
      }
    }

    LHS = factory->allocBinaryAST(op, LHS, RHS);
  }
}

ExprAST *Parser::parsePrimary() {
  switch (lex->currtok) {
  default: {
    logParserError("unknown token when expecting expression, supported "
                   "token are number, identifier or open paren, got:" +
                       std::to_string(lex->currtok),
                   lex, IssueCode::UNEXPECTED_TOKEN_IN_EXPRESSION);

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
  case Token::tok_nullptr: {
    return parseNullptr();
  }
  case Token::tok_operator: {
    // if we get here, there is only one possiblity, meaning that we have a
    // pointer dereference, lets check for that otherwise is an error
    if (lex->identifierStr != "*") {
      logParserError("found operator in primary expression, only supported "
                     "operator is * for pointer dereference, got:" +
                         std::to_string(lex->currtok),
                     lex, IssueCode::UNEXPECTED_TOKEN_IN_EXPRESSION);

      return nullptr;
    }

    // if we are here we have a pointer dereference, i don't think there are
    // other cases in a primary to have a * operator and not being a pointer
    // dereference
    return parseDereference();
  }
  }
}

ExprAST *Parser::parseAssigment() {

  // this can be, a direct value assignment
  // like int x = 10;
  // it can be a math expression like :
  // int x = 10 + y;
  // it can be a function call
  // int x = getMagicNumber();

  // here we need to eat a bit of token and proces the assigment operator
  int datatype = lex->currtok;
  lex->gettok(); // eat datatype;

  bool isPtr = false;
  if (lex->currtok == Token::tok_operator && lex->identifierStr == "*") {
    isPtr = true;
    lex->gettok(); // eating pointer
  }

  // next we eat the identifier;
  std::string identifier = lex->identifierStr;
  lex->gettok(); // eat identifier;

  lex->gettok(); // eat = operator
  auto *RHS = parseExpression();
  if (RHS == nullptr) {
    logParserError("error in generating RHS of assigment", this,
                   IssueCode::CANNOT_GENERATE_RHS);
    return nullptr;
  }
  ExprAST *node = factory->allocVariableAST(identifier, RHS, datatype);
  node->flags.isDefinition = true;
  node->flags.isPointer = isPtr;

  return node;
}

ExprAST *Parser::parseDeclaration() {
  // if we have a declaration token, something like int, float etc we might have
  // several cases like, we might have a variable definition, we might have
  // a function definition etc, this require a bit of look ahead!

  lex->lookAhead(3);
  // looking ahead 2 tokens, which should give us the identifier
  // and the next token

  // const lexer::MovableToken& nextTok= lex->lookAheadToken[0];
  if (lex->lookAheadToken[0].token == Token::tok_identifier) {
    // we got an identifier great, now the next token will
    // tell us whether is a function prototype or a variable
    switch (lex->lookAheadToken[1].token) {
    case Token::tok_open_round: {

      return parseFunction();
    }
    case Token::tok_assigment_operator: {

      // first we need to check if the tok is pointer and wheter or not we got a
      // * operator after
      if ((lex->currtok == Token::tok_void_ptr) &&
          !(lex->lookAheadToken[0].token == Token::tok_operator &&
            lex->lookAheadToken[0].identifierStr == "*")) {
        logParserError("expected * after void, cannot use void as not pointer "
                       "type, got :" +
                           std::to_string(lex->currtok),
                       this, IssueCode::ERROR_IN_VOID_DATATYPE);
        return nullptr;
      }
      return parseAssigment();
    }
    default: {
      logParserError(
          "unexpected token in declaration, expected = or open round got:" +
              std::to_string(lex->currtok),
          lex, IssueCode::UNEXPECTED_TOKEN_IN_DECLARATION);
      return nullptr;
    }
    }
  } else if (lex->lookAheadToken[0].token == Token::tok_operator) {
    if (lex->lookAheadToken[0].identifierStr != "*") {

      logParserError(
          "only supported operator after datatype is * for pointers, got" +
              std::to_string(lex->currtok),
          lex, IssueCode::UNEXPECTED_TOKEN_IN_DECLARATION);
      return nullptr;
    }

    switch (lex->lookAheadToken[2].token) {
    case Token::tok_open_round: {
      return parseFunction();
    }
    case Token::tok_assigment_operator: {
      return parseAssigment();
    }
      // TODO(giordi) should have a default case that logs error
    }
  }

  // not supported yet, declaring a function without init
  // int x; should be easy to do having default values
  // case Token::tok_end_statement:
  //    return parseVariableDefinition();

  // error
  logParserError("undexpected token while parsing declaration got: " +
                     std::to_string(lex->currtok),
                 lex, IssueCode::UNEXPECTED_TOKEN_IN_DECLARATION);
  return nullptr;
}

ExprAST *Parser::parseStatement() {

  ExprAST *exp = nullptr;
  bool expectSemicolon = true;
  if (lex->currtok == Token::tok_extern) {
    exp = parseExtern();
  } else if (lex->currtok == Token::tok_return) {
    lex->gettok(); // eat return
    exp = parseExpression();
    exp->flags.isReturn = true;
  } else if (isDeclarationToken(lex->currtok)) {
    exp = parseDeclaration();
    if (exp == nullptr) {
      return nullptr;
    }
    if (exp->nodetype == codegen::FunctionNode) {
      expectSemicolon = false;
    }
  } else if (lex->currtok == Token::tok_identifier) {
    exp = parseExpression();
  } else if (lex->currtok == Token::tok_if) {
    exp = parseIfStatement();
    expectSemicolon = false;
  } else if (lex->currtok == Token::tok_for) {
    exp = parseForStatement();
    expectSemicolon = false;
  } else if (lex->currtok == Token::tok_operator && lex->identifierStr == "*") {
    // the only time this can happen is when we are dereferencing a pointer to
    // write to it  something like *myPtr = 20;
    // to handle that we are gonna call parse pointer assigment;
    exp = parseToPointerAssigment();
  } else if (lex->currtok == Token::tok_struct) {
    exp = parseStruct();
    expectSemicolon = false;
  }
  // TODO(giordi) support statement starting with parenthesis
  // if (lex->currtok == Token::tok_open_paren){}

  if (lex->currtok != Token::tok_end_statement && expectSemicolon) {
    logParserError("expecting semicolon at end of statement got:" +
                       std::to_string(lex->currtok),
                   lex, IssueCode::EXPECTED_END_STATEMENT_TOKEN);
    // return exp;
    return nullptr;
  }
  if (lex->currtok == Token::tok_end_statement) {
    lex->gettok(); // eating semicolon;
  }

  // clearing flags, if they start to increase i will
  // change this to a clear flags
  flags.processed_assigment = false;
  return exp;
}

PrototypeAST *Parser::parseExtern() {
  // eating extern token;
  lex->gettok();
  if (!isDatatype(lex->currtok)) {
    logParserError("expected return data type after extern got:" +
                       std::to_string(lex->currtok),
                   lex, IssueCode::EXPECTED_TYPE_AFTER_EXTERN);
    return nullptr;
  }
  return parsePrototype();
}

bool parseArguments(Lexer *lex, std::vector<Argument> *args) {
  int datatype;
  bool isPointer = false;
  std::string argName;
  while (true) {
    isPointer = false;
    if (lex->currtok == Token::tok_close_round) {
      // no args or done with args
      lex->gettok(); // eat )
      return true;
    }
    // we expect to see seqence of data_type identifier comma
    if (!Parser::isDatatype(lex->currtok)) {
      logParserError("expected data type identifier for argument got:" +
                         std::to_string(lex->currtok),
                     lex, IssueCode::EXPECTED_DATATYPE_FUNCTION_ARG);
      return false;
    }
    // saving datatype
    datatype = lex->currtok;
    lex->gettok(); // eat datatype

    if (lex->currtok == Token::tok_operator && lex->identifierStr == "*") {
      // here we got a datatype and a * which means is a pointer
      isPointer = true;
      lex->gettok(); // eating *;
    }

    if (lex->currtok != Token::tok_identifier) {
      logParserError("expected identifier name for argument got:" +
                         std::to_string(lex->currtok),
                     lex, IssueCode::EXPECTED_IDENTIFIER_NAME);
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
      if (lex->currtok == Token::tok_close_round) {
        logParserError("expected data type identifier after comma, got :" +
                           std::to_string(lex->currtok),
                       lex, IssueCode::EXPECTED_DATATYPE_FUNCTION_ARG);
        return false;
      }
    }
    // if we got here we have a sanitized argument
    args->emplace_back(Argument(datatype, argName, isPointer));
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
    logParserError("error expected { after function prototype, got :" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }

  lex->gettok(); // eating curly
  std::vector<ExprAST *> statements;
  bool res = parseStatementsUntillCurly(&statements, this);
  if (!res) {
    return nullptr;
  }

  //// here we should have all the statements, expected }
  //// if not it means something went wrong or we hit the
  //// security limit
  if (lex->currtok != Token::tok_close_curly) {
    logParserError("expected } at end of function bodygot:" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }

  lex->gettok(); // eat }
  return factory->allocFunctionAST(proto, statements);
}

ExprAST *Parser::parseParen() {

  // here we need to figure out if we have a generic expression or a type cast
  lex->lookAhead(3);
  // now if the next 3 tokens are datatype , operator * and ) we have a cast
  if (isCastOperation(lex)) {
    // if we are here we have a cast
    return parseCast();
  }

  lex->gettok(); // eating paren
  auto *exp = parseExpression();
  if (lex->currtok != Token::tok_close_round) {
    logParserError("expected close paren after expression got:" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }
  lex->gettok(); // eating )
  return exp;
}

codegen::ExprAST *Parser::parseIfStatement() {
  lex->gettok(); // eating if tok
  if (lex->currtok != Token::tok_open_round) {
    logParserError("expected  ( after if , got :" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }
  lex->gettok(); // eating (
  // now inside the peren we do expect an expression
  auto *condition = parseExpression();
  if (condition == nullptr) {
    // TODO(giordi), verify error should be handled in expression
    return nullptr;
  }
  if (lex->currtok != Token::tok_close_round) {
    logParserError("expected   ) after if statement condition, got :" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }
  lex->gettok(); // eating )

  if (lex->currtok != Token::tok_open_curly) {
    logParserError("expected  { after if statement condition, got :" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }
  lex->gettok(); // eating {

  std::vector<ExprAST *> ifStatements;
  auto res = parseStatementsUntillCurly(&ifStatements, this, true);
  if (!res) {
    // TODO(giordi), verify error should be handled in expression
    return nullptr;
  }

  if (lex->currtok != Token::tok_close_curly) {
    logParserError("expected  } after if statement body , got :" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }
  lex->gettok(); // eating {

  std::vector<ExprAST *> elseStatements;
  if (lex->currtok == Token::tok_else) {
    lex->gettok(); // esting else
    // now at this point we can have two possiblities, either directly the
    // else body or the if for the next  contidion
    if (lex->currtok == Token::tok_if) {
      // TODO(giordi) support if else statement to make life easier
      // for the programmer
      logParserError("error : else if construct currently not supported:", this,
                     IssueCode::UNEXPECTED_TOKEN_IN_EXPRESSION);
      return nullptr;
    }

    if (lex->currtok == Token::tok_open_curly) {
      // if we have an open curly we need to parse the body of
      // the branch
      lex->gettok(); // eat {

      auto elseres = parseStatementsUntillCurly(&elseStatements, this);
      if (!elseres) {
        // TODO(giordi), verify error should be handled in expression
        return nullptr;
      }

      if (lex->currtok != Token::tok_close_curly) {
        std::cout << "expected } at end of statement" << std::endl;
        return nullptr;
      }
      lex->gettok(); // eat }
    } else {
      // TODO(giordi) support else if construct
      logParserError("expected  { after else keyword , got :" +
                         std::to_string(lex->currtok),
                     this, IssueCode::EXPECTED_TOKEN);
      return nullptr;
    }
  }

  return factory->allocIfAST(condition, ifStatements, elseStatements);
}

codegen::ExprAST *Parser::parseForStatement() {
  lex->gettok(); // eating for token
  if (lex->currtok != Token::tok_open_round) {
    logParserError("expected ( after for keyword got:" +
                       std::to_string(lex->currtok),
                   lex, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }

  lex->gettok(); // eating (

  // at this point we expect a variable declaration + assigment or
  // variable + assigment
  lex->lookAhead(2);
  ExprAST *initialisationExp = nullptr;
  if (isDatatype(lex->currtok)) {
    // if is a datatype we then expect an identifier and an assigment

    if (lex->lookAheadToken[0].token != Token::tok_identifier) {

      logParserError("expected identifier after datatype in for loop variable "
                     "initalisation got:" +
                         std::to_string(lex->currtok),
                     lex, IssueCode::EXPECTED_IDENTIFIER_NAME);
      return nullptr;
    }
    if (lex->lookAheadToken[1].token != Token::tok_assigment_operator) {

      logParserError("expected assigment operator after varible declaration in "
                     "for loop header got:" +
                         std::to_string(lex->currtok),
                     lex, IssueCode::EXPECTED_TOKEN);
      return nullptr;
    }

    initialisationExp = parseStatement();
  } else {
    if (lex->lookAheadToken[0].token != Token::tok_assigment_operator) {

      logParserError("expected assigment operator after varible declaration in "
                     "for loop header got:" +
                         std::to_string(lex->currtok),
                     lex, IssueCode::EXPECTED_TOKEN);
      return nullptr;
    }

    initialisationExp = parseStatement();
  }

  if (initialisationExp == nullptr) {
    // failed to parse initalization of for loop
    return nullptr;
  }
  // at this point we have parsed the initalization, and already
  // ate the ; because the parse statement takes care of that
  // next we need to parse the condition
  ExprAST *condition = parseStatement();
  if (condition == nullptr) {
    // failed to parse condition of for loop
    logParserError("error parsing condition for FOR loop", lex,
                   IssueCode::FOR_LOOP_FAILURE);
    return nullptr;
  }

  // at this point we just have the increment expression to take care of
  ExprAST *increment = parseExpression();
  // clearing up the assigment flag in the parser
  flags.processed_assigment = false;
  if (increment == nullptr) {
    logParserError("error parsing increment for FOR loop", lex,
                   IssueCode::FOR_LOOP_FAILURE);
    return nullptr;
  }
  // at this point we expect a ) and { before the body of the loop
  if (lex->currtok != Token::tok_close_round) {
    logParserError("expected ) at end of for loop header "
                   "got:" +
                       std::to_string(lex->currtok),
                   lex, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }

  lex->gettok(); // eating )
  if (lex->currtok != Token::tok_open_curly) {
    logParserError("expected { afyer for loop header "
                   "got:" +
                       std::to_string(lex->currtok),
                   lex, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }
  lex->gettok(); // eating {

  // ready to parse body
  std::vector<ExprAST *> statements;
  bool res = parseStatementsUntillCurly(&statements, this);
  if (!res) {
    return nullptr;
  }
  if (lex->currtok != Token::tok_close_curly) {
    logParserError("expected { afyer for loop header "
                   "got:" +
                       std::to_string(lex->currtok),
                   lex, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }
  lex->gettok(); // eat }

  return factory->allocForAST(initialisationExp, condition, increment,
                              statements);
} // namespace parser

codegen::NumberExprAST *Parser::parseNullptr() {
  Number zero{};
  zero.integerNumber = 0;
  zero.type = Token::tok_int;
  auto *node = factory->allocNuberAST(zero);
  node->datatype = Token::tok_int;
  node->flags.isPointer = true;
  node->flags.isNull = true;
  lex->gettok(); // eat nullptr;
  return node;
}

PrototypeAST *Parser::parsePrototype() {

  if (!isDatatype(lex->currtok)) {
    logParserError(
        "expected return data type in function protoype or extern , got :" +
            std::to_string(lex->currtok),
        this, IssueCode::EXPECTED_RETURN_DATATYPE);
    return nullptr;
  }
  int datatype = lex->currtok;
  lex->gettok(); // eating datatype
  bool isPointer = false;
  bool isNull = false;
  if (lex->currtok == Token::tok_operator && lex->identifierStr == "*") {
    isPointer = true;
    lex->gettok(); // eating *
  }
  if (datatype == Token::tok_void_ptr) {
    isNull = true;
  }
  // suppressing this for letting void datatype return
  // if ((datatype == Token::tok_void_ptr) & !isPointer) {
  //  logParserError(
  //      "expected * after void, cannot use void as not pointer type, got :" +
  //          std::to_string(lex->currtok),
  //      this, IssueCode::ERROR_IN_VOID_DATATYPE);
  //  return nullptr;
  //}

  if (lex->currtok != Token::tok_identifier) {
    logParserError("expected identifier name after function prototype return "
                   "datatype, got :" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_IDENTIFIER_NAME);
    return nullptr;
  }
  // we know that we need a function call so we get started
  std::string functionName = lex->identifierStr;
  lex->gettok(); // eat identifier

  if (lex->currtok != Token::tok_open_round) {
    logParserError("expected ( after function protoype name, got :" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_TOKEN);

    // should I eat bad identifier here or not?
    lex->gettok(); // earting bad identifier
    return nullptr;
  }
  // parsing arguments
  lex->gettok(); // eat parenthesis
  std::vector<Argument> args;
  if (!parseArguments(lex, &args)) {
    // no need to log error, error already logged
    return nullptr;
  }
  // need to check semicolon at the end;
  // here we can generate the prototype node;
  auto *node = factory->allocPrototypeAST(datatype, functionName, args, true);
  node->flags.isPointer = isPointer;
  node->flags.isNull = isNull;
  return node;
}

codegen::ExprAST *Parser::parseDereference() {

  lex->gettok(); // eating *
  if (lex->currtok != Token::tok_identifier) {
    logParserError("expected identifier after dereference operator, got:" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }
  auto *node = factory->allocDereferenceAST(lex->identifierStr);
  lex->gettok(); // eat identifier name
  return node;
}

codegen::ExprAST *Parser::parseToPointerAssigment() {

  lex->gettok(); // eating the pointer;
  // now we do expect an identifier;

  if (lex->currtok != Token::tok_identifier) {
    logParserError("expected identifier after dereference operator, got:" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }

  std::string identifier = lex->identifierStr;
  lex->gettok(); // eat identifier;

  // now we expect to see an assigment operator
  // TODO(giordi) support expression that computes a new pointer before
  // dereferncing like something  *(myPtr +3)
  if (lex->currtok != Token::tok_assigment_operator) {
    logParserError(
        "expected assigment operator after pointer dereference, got:" +
            std::to_string(lex->currtok),
        this, IssueCode::EXPECTED_TOKEN);
    return nullptr;
  }

  lex->gettok(); // eat =

  auto *RHS = parseExpression();
  if (RHS == nullptr) {
    logParserError("error parsing RHS of pointer assigment", this,
                   IssueCode::UNEXPECTED_TOKEN_IN_EXPRESSION);
    return nullptr;
  }

  return factory->allocToPointerAssigmentAST(identifier, RHS);
}

codegen::ExprAST *Parser::parseCast() {
  lex->gettok(); // eat (

  if (!isDatatype(lex->currtok)) {
    // should never get to this error since we checked outside but better safe
    // than sorry
    logParserError("expected datatype after ( in cast operation", this,
                   IssueCode::UNEXPECTED_TOKEN_IN_EXPRESSION);
    return nullptr;
  }

  int datatype = lex->currtok;
  lex->gettok(); // parse datatype

  bool isPointer = false;
  if (lex->currtok == Token::tok_operator && lex->identifierStr == "*") {
    isPointer = true;
    lex->gettok(); // eating *
  }
  if ((datatype == Token::tok_void_ptr) & !isPointer) {
    logParserError(
        "expected * after void, cannot use void as not pointer type, got :" +
            std::to_string(lex->currtok),
        this, IssueCode::ERROR_IN_VOID_DATATYPE);
    return nullptr;
  }

  if (lex->currtok != Token::tok_close_round) {
    logParserError("expected ) at end of cast operation", this,
                   IssueCode::UNEXPECTED_TOKEN_IN_EXPRESSION);
    return nullptr;
  }

  lex->gettok(); // eat )
  // now we do expect and expression which actually yields a pointer
  ExprAST *RHS = parseExpression();
  if (RHS == nullptr) {
    logParserError("error generating RHS of cast operation", this,
                   IssueCode::CAST_ERROR);
    return nullptr;
  }
  return factory->allocCastAST(datatype, isPointer, RHS);
}

codegen::StructAST *Parser::parseStruct() {

  lex->gettok(); // eating struct token

  if (lex->currtok != Token::tok_identifier) {
    logParserError("error expected identifier name after struct token, got:" +
                       std::to_string(lex->currtok),
                   this, IssueCode::UNEXPECTED_TOKEN_IN_STRUCT);
    return nullptr;
  }

  std::string identifierName = lex->identifierStr;
  lex->gettok(); // eating identifier;

  if (lex->currtok != Token::tok_open_curly) {
    logParserError("error expected { after identifier name for struct, got:" +
                       std::to_string(lex->currtok),
                   this, IssueCode::UNEXPECTED_TOKEN_IN_STRUCT);
    return nullptr;
  }
  lex->gettok(); // eat {

  std::vector<codegen::StructMemberAST *> statements;
  auto res = parseDeclarationsUntilClosedCurly(&statements, this);
  if (!res) {
    logParserError("error in parsing members of struct", this,
                   IssueCode::UNEXPECTED_TOKEN_IN_STRUCT);
    return nullptr;
  }
  if (statements.size() == 0) {
    logParserError("error, empty structs are not supported", this,
                   IssueCode::EMPTY_STRUCT);
    return nullptr;
  }

  lex->gettok(); // eat curly

  return factory->allocStructAST(identifierName, statements);
}
} // namespace parser
} // namespace babycpp
