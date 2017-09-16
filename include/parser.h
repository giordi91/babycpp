#pragma once
#include "lexer.h"
#include <unordered_map>
namespace babycpp {
namespace parser {

using lexer::Lexer;
using lexer::Number;

// base struct for all ast expr
struct ExprAST {
  ExprAST() = default;
  ExprAST(int type) : datatype(type) {}
  int datatype = 0;
  virtual ~ExprAST() = default;
};

struct NumberExprAST : public ExprAST {
  NumberExprAST(Number val) : val(val) {}

  // keeping it public mainly for testabiliy purposes
  Number val;
};

struct VariableExprAST : public ExprAST {
  std::string name;
  ExprAST *value;
  VariableExprAST(const std::string &name, ExprAST *invalue, int type)
      : ExprAST(type), name{name}, value(invalue) {}
};

struct BinaryExprAST : public ExprAST {
  std::string op;
  ExprAST *lhs, *rhs;
  BinaryExprAST(std::string &op, ExprAST *lhs, ExprAST *rhs)
      : op(op), lhs(lhs), rhs(rhs) {}
};

struct CallExprAST : public ExprAST {
  std::string callee;
  std::vector<ExprAST *> args;

  CallExprAST(const std::string &callee, std::vector<ExprAST *> &args)
      : callee(callee), args(args) {}
};

struct Argument {
  Argument(int datatype, std::string &argName)
      : type(datatype), name(argName) {}
  int type;
  std::string name;
};

struct PrototypeAST : public ExprAST {
  std::string name;
  std::vector<Argument> args;
  bool isExtern = false;

  PrototypeAST(int retType, const std::string &name,
               const std::vector<Argument> &args, bool externProto)
      : ExprAST(retType), name(name), args(args), isExtern(externProto) {}
};

struct FunctionAST : public ExprAST {
  PrototypeAST *proto;
  ExprAST *body;

  FunctionAST(PrototypeAST *proto, ExprAST *body) : proto(proto), body(body) {}
};

struct ParserFlags {
    bool processed_assigment :1;
};

struct Parser {
  explicit Parser(Lexer *inputLexer) : lex(inputLexer)
  {
    flags.processed_assigment=false;
  }

  NumberExprAST *parseNumber();
  ExprAST *parseIdentifier();
  ExprAST *parseExpression();
  ExprAST *parseBinOpRHS(int givenPrec, ExprAST *LHS);
  ExprAST *parsePrimary();

  int getTokPrecedence();
  ExprAST *parseStatement();
  PrototypeAST *parseExtern();
  bool parseArguments(std::vector<Argument> &args);
  ExprAST *parseFunction();
  ExprAST *parseVariableDefinition();
  ExprAST *parseDeclaration();
  ExprAST *parseParen();
  // this function defines whether or not a token is a declaration
  // token or not, meaning defining an external function or datatype
  // interesting to think of cating as "anonymous declaration maybe?"
  bool isDeclarationToken();
  bool isDatatype();
  const static std::unordered_map<char, int> BIN_OP_PRECEDENCE;
  Lexer *lex;
  ParserFlags flags;
};

} // namespace parser
} // namespace babycpp
