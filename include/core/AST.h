#pragma once
#include "lexer.h"
#include <string>

#include <llvm/IR/Value.h>

namespace babycpp {

using lexer::Number;

namespace codegen {
/**
 * @brief simple struct defining AST flags
 */
struct ASTFlags {
  /** defines wheter or not this is a return AST,
   * meaning it returns a value from a function */
  bool isReturn : 1;
  /** define wheter or not the AST defines a type or a function*/
  bool isDefinition : 1;
  bool isPointer : 1;
  bool isNull : 1;
};

/**Simple structure defining an argument type for function
 * declaration
 */
struct Argument {
  Argument(int datatype, std::string &argName, bool inIsPointer)
      : type(datatype), name(argName), isPointer(inIsPointer) {}
  /**Token datatype , like tok_int etc*/
  int type;
  std::string name;
  bool isPointer;
};

/**
 * This struct is used to tag the ast node for a specific type
 * I tried to keep as much as possible the polymorfish clean and
 * innocent, unluckily in order to deduce the type in the repl
 * enviroment I had to conceed the type
 */
enum NodeType {
  NumberNode = 1,
  VariableNode = 2,
  BinaryNode = 3,
  CallNode = 4,
  PrototypeNode = 5,
  FunctionNode = 6,
  IfNode = 7,
  ForNode = 8,
  DereferenceNode = 9,
  ToPointerAssigmentNode = 10,
  CastASTNode = 11,
  StructMemberNode = 12,
  StructNode = 13,
  StructInstanceNode = 14,
};

struct Codegenerator;

/**
 * @brief base class for all the AST nodes
 */
struct ExprAST {
  ExprAST() {
    flags.isReturn = false;
    flags.isDefinition = false;
    flags.isPointer = false;
  };
  ExprAST(int type) : datatype(type) {
    flags.isReturn = false;
    flags.isDefinition = false;
    flags.isPointer = false;
  }
  virtual ~ExprAST() = default;
  /**
   * @brief generates IR for the given node
   * @param gen : pointer to the generator to use to
   *              generate the needed IR
   * @return  *val last generated instruction from the IR
   */
  virtual llvm::Value *codegen(Codegenerator *gen) = 0;

  /** what datatype the node represnts, 0 it means type is
   * not known */
  int datatype = 0;
  NodeType nodetype;
  ASTFlags flags;
};

/**  @brief This AST node represents a numerical type */
struct NumberExprAST : public ExprAST {
  explicit NumberExprAST(Number val) : ExprAST(), val(val) {
    // TODO(giordi) add assert for it to be a datatype token
    datatype = val.type;
    nodetype = NumberNode;
  }
  virtual ~NumberExprAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;

  Number val;
};

/**
 * @brief represents a variable
 * This node is used to define a variable, there can be several
 * instances in which this AST node is used, it can be a simple
 * variable reference, it can be a variable assigment, in that
 * case the value ast represents the value to assign
 * If the variable is a definition aswell like int x = 1;
 * the flag is definition will be set when the AST node is
 * created
 */
struct VariableExprAST : public ExprAST {
  /** actual name of the variable in the source code, there
   * is no mangling */
  std::string name;
  /**If not none, this value holds the value to assign to the
   * variable */
  ExprAST *value;
  explicit VariableExprAST(const std::string &name, ExprAST *invalue, int type)
      : ExprAST(type), name{name}, value(invalue) {
    nodetype = VariableNode;
  }
  virtual ~VariableExprAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
};

/** Represents a binary expression, with a lhr,rhs and the operator */
struct BinaryExprAST : public ExprAST {
  /** one of the supported operator, it is not a char, in this way
   * we can support varying length operators like >= or similar
   * although not supported yet */
  std::string op;
  /**left and right side of the binary, this can be anything,
   * function call or sub expression */
  ExprAST *lhs, *rhs;
  explicit BinaryExprAST(std::string &op, ExprAST *lhs, ExprAST *rhs)
      : ExprAST(), op(op), lhs(lhs), rhs(rhs) {
    nodetype = BinaryNode;
  }

  llvm::Value *codegen(Codegenerator *gen) override;
};

/**@brief Function invocation */
struct CallExprAST : public ExprAST {
  /** name of the function to be called, not mangled*/
  std::string callee;
  /** list of arguments node*/
  std::vector<ExprAST *> args;

  explicit CallExprAST(const std::string &callee, std::vector<ExprAST *> &args)
      : ExprAST(), callee(callee), args(args) {
    nodetype = CallNode;
  }
  virtual ~CallExprAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
};

/**@brief defines the signature of a function */
struct PrototypeAST : public ExprAST {
  /** name of the function to be called, not mangled*/
  std::string name;
  /** Array of arguments of the function can be empty*/
  std::vector<Argument> args;
  /**This bool defines wheter is a forward declarsation for a
   * c function, regular forward declaration is not supported */
  bool isExtern = false;

  explicit PrototypeAST(int retType, const std::string &name,
                        const std::vector<Argument> &args, bool externProto)
      : ExprAST(retType), name(name), args(args), isExtern(externProto) {
    nodetype = PrototypeNode;
  }
  virtual ~PrototypeAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
};

/** @brief complete function definition, with a body and
 * prototype
 */
struct FunctionAST : public ExprAST {
  PrototypeAST *proto;
  /** the body is defined by a series of statment, each of them
   * has its own AST node */
  std::vector<ExprAST *> body;

  explicit FunctionAST(PrototypeAST *inproto, std::vector<ExprAST *> &inbody)
      : ExprAST(), proto(inproto), body(inbody) {
    nodetype = FunctionNode;
  }
  virtual ~FunctionAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
};

struct IfAST : public ExprAST {
  ExprAST *condition;
  std::vector<ExprAST *> ifExpr;
  std::vector<ExprAST *> elseExpr;
  explicit IfAST(ExprAST *inCondition, std::vector<ExprAST *> inIfExpr,
                 std::vector<ExprAST *> inElseExpr)
      : ExprAST(), condition(inCondition), ifExpr(inIfExpr),
        elseExpr(inElseExpr) {
    nodetype = IfNode;
  }
  virtual ~IfAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
};

struct ForAST : public ExprAST {
  ExprAST *initialization;
  ExprAST *condition;
  ExprAST *increment;
  std::vector<ExprAST *> body;
  explicit ForAST(ExprAST *inInitialization, ExprAST *inCondition,
                  ExprAST *inIncrement, std::vector<ExprAST *> inBody)
      : ExprAST(), initialization(inInitialization), condition(inCondition),
        increment(inIncrement), body(inBody) {
    nodetype = ForNode;
  }
  virtual ~ForAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
};

struct DereferenceAST : public ExprAST {
  std::string identifierName;
  explicit DereferenceAST(const std::string &inIdentifierName)
      : ExprAST(), identifierName(inIdentifierName) {
    nodetype = DereferenceNode;
    flags.isPointer = true;
  }
  virtual ~DereferenceAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
};

struct ToPointerAssigmentAST : public ExprAST {

  std::string identifierName;
  ExprAST *rhs;
  explicit ToPointerAssigmentAST(std::string inIdentifierName, ExprAST *inRhs)
      : ExprAST(), identifierName(inIdentifierName), rhs(inRhs) {
    nodetype = ToPointerAssigmentNode;
    flags.isPointer = true;
  }
  virtual ~ToPointerAssigmentAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
};

struct CastAST : public ExprAST {

  ExprAST *rhs;
  explicit CastAST(int inDatatype, bool isPointer, ExprAST *inRhs)
      : ExprAST(), rhs(inRhs) {
    nodetype = CastASTNode;
    flags.isPointer = isPointer;
    datatype = inDatatype;
  }
  virtual ~CastAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
};

struct StructMemberAST : public ExprAST {

  std::string identifierName;
  int biteOffset = -1;
  explicit StructMemberAST(int inDatatype, bool isPointer,
                           std::string& inIdentifierName)
      : ExprAST(), identifierName(inIdentifierName) {
    nodetype = StructMemberNode;
    flags.isPointer = isPointer;
    datatype = inDatatype;
  }
  virtual ~StructMemberAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
  llvm::Type *codegenType(Codegenerator *gen);
  int getTypeSize();
};

struct StructAST : public ExprAST {

  std::string identifierName;
  int byteSize= -1;
  std::vector<StructMemberAST *> members;
  explicit StructAST(std::string& inIdentifierName,
                     std::vector<StructMemberAST *> &inMembers)
      : ExprAST(), identifierName(inIdentifierName), members(inMembers) {
    nodetype = StructNode;
  }
  virtual ~StructAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override;
  llvm::Type *codegenType(Codegenerator *gen);
};

//TODO(giordi) tag structs as custom datatype
struct StructInstanceAST : public ExprAST {

  std::string structType;
  std::string identifierName;
  explicit StructInstanceAST(std::string& inStructType, std::string& inIdentifierName )
      : ExprAST(), structType(inStructType), identifierName(inIdentifierName) {
    nodetype = StructInstanceNode;
  }
  virtual ~StructInstanceAST() = default;
  llvm::Value *codegen(Codegenerator *gen) override ;
};


} // namespace codegen
} // namespace babycpp
