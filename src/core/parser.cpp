#include "parser.h"
#include "codegen.h"

#include <iostream>

namespace babycpp {
namespace parser {

using codegen::Argument;
using codegen::ExprAST;
using codegen::FunctionAST;
using codegen::IfAST;
using codegen::NumberExprAST;
using codegen::PrototypeAST;
using codegen::VariableExprAST;
using diagnostic::Diagnostic;
using diagnostic::IssueCode;
using lexer::Token;

const std::unordered_map<char, int> Parser::BIN_OP_PRECEDENCE = {
    {'<', 10}, {'+', 20}, {'-', 20}, {'*', 40}, {'/', 50}};

// UTILITY
inline bool isDatatype(int tok) {
  return (tok == Token::tok_float || tok == Token::tok_int);
}
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
  }
}

ExprAST *Parser::parseDeclaration() {
  // if we have a declaration token, something like int, float etc we might have
  // several cases like, we might have a variable definition, we might have
  // a function definition etc, this require a bit of look ahead!

  lex->lookAhead(2);
  // lex->gettok(); // eating datatype
  // looking ahead 2 tokens, which should give us the identifier
  // and the next token

  // const lexer::MovableToken& nextTok= lex->lookAheadToken[0];
  if (lex->lookAheadToken[0].token == Token::tok_identifier) {
    // std::string identifier = lex->identifierStr;
    // lex->gettok(); // eat identifier;
    // we got an identifier great, now the next token will
    // tell us whether is a function prototype or a variable
    switch (lex->lookAheadToken[1].token) {
    default: {
      // error
      return nullptr;
    }
    case Token::tok_open_round: {
      return parseFunction();
    }
    case Token::tok_assigment_operator: {
      // this can be, a direct value assignment
      // like int x = 10;
      // it can be a math expression like :
      // int x = 10 + y;
      // it can be a function call
      // int x = getMagicNumber();

      // here we need to eat a bit of token and proces the assigment operator
      int datatype = lex->currtok;
      lex->gettok(); // eat datatype;

      // next we eat the identifier;
      std::string identifier = lex->identifierStr;
      lex->gettok(); // eat identifier;

      lex->gettok(); // eat = operator
      auto *RHS = parseExpression();
      ExprAST *node = factory->allocVariableAST(identifier, RHS, datatype);
      node->flags.isDefinition = true;

      return node;
    }
      // not supported yet, declaring a function without init
      // int x; should be easy to do having default values
      // case Token::tok_end_statement:
      //    return parseVariableDefinition();
    }
  }

  // error
  return nullptr;
}

ExprAST *Parser::parseStatement() {

  ExprAST *exp = nullptr;
  bool expectSemicolon = true;
  if (lex->currtok == Token::tok_eof) {
    return nullptr;
  } else if (lex->currtok == Token::tok_extern) {
    exp = parseExtern();
  } else if (lex->currtok == Token::tok_return) {
    lex->gettok(); // eat return
    exp = parseExpression();
    exp->flags.isReturn = true;
  } else if (isDeclarationToken(lex->currtok)) {
    exp = parseDeclaration();
    if (exp->nodetype == codegen::FunctionNode) {
      expectSemicolon = false;
    }
  }

  else if (lex->currtok == Token::tok_identifier) {
    exp = parseExpression();
  } else if (lex->currtok == Token::tok_if) {
    exp = parseIfStatement();
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
  std::string argName;
  while (true) {
    if (lex->currtok == Token::tok_close_round) {
      // no args or done with args
      lex->gettok(); // eat )
      return true;
    }
    // we expect to see seqence of data_type identifier comma
    if (!isDatatype(lex->currtok)) {
      logParserError("expected data type identifier for argument got:" +
                         std::to_string(lex->currtok),
                     lex, IssueCode::EXPECTED_DATATYPE_FUNCTION_ARG);
      return false;
    }
    // saving datatype
    datatype = lex->currtok;
    lex->gettok(); // eat datatype

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
    args->emplace_back(Argument(datatype, argName));
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
  const int SECURITY = 2000;
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
    logParserError("error, expected } got EOF" ,
                   this, IssueCode::EXPECTED_TOKEN);
      return nullptr;
    }
  }

  // here we should have all the statements, expected }
  // if not it means something went wrong or we hit the
  // security limit
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

  auto *ifBranch = parseStatement();
  if (ifBranch == nullptr) {
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

  ExprAST *elseBody = nullptr;
  if (lex->currtok == Token::tok_else) {
    lex->gettok(); // esting else
    // now at this point we can have two possiblities, either directly the else
    // body or the if for the next  contidion
    if (lex->currtok == Token::tok_if) {
      // TODO(giordi) support if else statement to make life easier
      // for the programmer
      logParserError("error : else if construct currently not supported:", this,
                     IssueCode::UNEXPECTED_TOKEN_IN_EXPRESSION);
      return nullptr;
    }

    else if (lex->currtok == Token::tok_open_curly) {
      // if we have an open curly we need to parse the body of
      // the branch
      lex->gettok(); // eat {
      elseBody = parseStatement();
      if (elseBody == nullptr) {
        // TODO(giordi), verify error should be handled in expression
        return nullptr;
      }

      if (lex->currtok != Token::tok_close_curly) {
        std::cout << "expected } at end of statement" << std::endl;
        return nullptr;
      }
      lex->gettok(); // eat }
    } else {
      std::cout << "" << std::endl;

      // TODO(giordi) support else if construct
      logParserError("expected  { after else keyword , got :" +
                         std::to_string(lex->currtok),
                     this, IssueCode::EXPECTED_TOKEN);
      return nullptr;
    }
  }

  return factory->allocIfAST(condition, ifBranch, elseBody);
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

  if (lex->currtok != Token::tok_identifier) {
    logParserError("expected identifier name after function prototype return "
                   "datatype, got :" +
                       std::to_string(lex->currtok),
                   this, IssueCode::EXPECTED_IDENTIFIER_NAME);
    return nullptr;
  }
  // we know that we need a function call so we get started
  std::string funName = lex->identifierStr;
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
  return factory->allocPrototypeAST(datatype, funName, args, true);
}

} // namespace parser
} // namespace babycpp
