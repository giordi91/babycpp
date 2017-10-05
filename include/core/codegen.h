#pragma once
#include <memory>
#include <unordered_map>

#include "factoryAST.h"
#include "lexer.h"
#include "parser.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

namespace babycpp {

using lexer::Lexer;
using lexer::Token;
using parser::Parser;

namespace codegen {

using lexer::Number;
/**
 * @brief simple struct defining AST flags
 */
struct ASTFlags {
  /** defines wheter or not this is a return AST,
   * meaning it returns a value from a function */
  bool isReturn : 1;
  /** define wheter or not the AST defines a type or a function*/
  bool isDefinition : 1;
};

/**Simple structure defining an argument type for function
 * declaration
 */
struct Argument {
  Argument(int datatype, std::string &argName)
      : type(datatype), name(argName) {}
  /**Token datatype , like tok_int etc*/
  int type;
  std::string name;
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
  FunctionNode = 6
};

struct Codegenerator;

/**
 * @brief base class for all the AST nodes
 */
struct ExprAST {
  ExprAST() {
    flags.isReturn = false;
    flags.isDefinition = false;
  };
  ExprAST(int type) : datatype(type) {
    flags.isReturn = false;
    flags.isDefinition = false;
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
  llvm::Value *codegen(Codegenerator *gen) override;
};

/** @brief complete function definition, with a body and
 * prototype
 */
struct FunctionAST : public ExprAST {
  PrototypeAST *proto;
  /** the body is defined by a series of statment, each of them
   * has its own ast node */
  std::vector<ExprAST *> body;

  explicit FunctionAST(PrototypeAST *inproto, std::vector<ExprAST *> &inbody)
      : ExprAST(), proto(inproto), body(inbody) {
    nodetype = FunctionNode;
  }
  llvm::Value *codegen(Codegenerator *gen) override;
};

/** This class is the heavy lifter in the compiler, it
 * is an agglomerate of parser and lexer, its job is to
 * coordinate the job from getting the source code to emitting
 * final IR
 */
struct Codegenerator {

  explicit Codegenerator();
  /**This function sets the current llvm module into wich
   * we will be generating code, this allow the flexibility
   * to have repl with discardable module
   * @param mod : the module we want to work on
   */
  void setCurrentModule(std::shared_ptr<llvm::Module> mod);
  /**
   * @brief initializes the lexer with the given string
   * @param str: the source code on which to peform lexical
   * analysis
   */
  inline void initFromString(const std::string &str) {
    lexer.initFromStr(str);
    // getting first token so the parser is ready to go
    lexer.gettok();
  };
  /**@brief utility function convert llvm values to
   * a the string representation
   * @param v: the value to be printed
   * @return the resulting string
   */
  static std::string printLlvmData(llvm::Value *v) {
    std::string outs;
    llvm::raw_string_ostream os(outs);
    v->print(os, false);
    os.flush();
    return outs;
  }

  /** Similar to print string but dumps into a file
   * in the same folder of the executable
   * @param v: the value to be written in the file
   * @param path: string representing the path or file name to save
   *              into
   */
  static void dumpLlvmData(llvm::Value *v, const std::string &path) {
    const std::string outs = printLlvmData(v);
    std::ofstream out(path);
    out << outs;
    out.close();
  }
  /**Utility function to crate an alloca (stack variable) in the
   * entry point of a function
   * @param function: the function into wich we will perform the alloca
   * @param varName the name of the variable defining the alloca
   * @param type : datatype of the alloca
   * @return :return alloca pointer
   */
  llvm::AllocaInst *createEntryBlockAlloca(llvm::Function *function,
                                           const std::string &varName,
                                           int type);

  /**This function takes in two datatpes and figures out the result of
   * the operation
   * @param leftAST: left hand side node of the binary operation
   * @param rightAST: right hand side node of the binary operation
   * @param leftValue: this is a pointer to an llvm value, if any kind of
   *                   casting operation is done the pointer to the llvm value
   *                   will be overritten with the new llvm value after the
   *                   cast
   * @param rightValue: same as leftValue
   * @return : returns what is the token type of the resulting operation
   *                   taking into account possible cast done
   */
  int omogenizeOperation(ExprAST *leftAST, ExprAST *rightAST,
                         llvm::Value **leftValue, llvm::Value **rightValue);
  /**Checks wheter the given token representing a datatype is the
   * same datatype in llvm
   * @param astArg : token type representing the argument, one of tok_int...
   * @param llvmArg : llvm type
   * @return: wheter or not the two tokens represent the same type
   */
  static bool compareASTArgWithLLVMArg(ExprAST *astArg,
                                       llvm::Argument *llvmArg);
  /**Factory in charge to allocate the factory nodes, it owns the
   * memory */
  memory::FactoryAST factory;

  lexer::Lexer lexer;
  parser::Parser parser;

  /// llvm classes needed for IR gen
  llvm::LLVMContext context;
  llvm::IRBuilder<> builder;
  std::shared_ptr<llvm::Module> module;

  /// map holding variable names defined in the scope
  std::unordered_map<std::string, llvm::AllocaInst *> namedValues;
  /**mapping from lexer types to LLCM types*/
  static const std::unordered_map<int, int> AST_LLVM_MAP;
  /** if we are in a scope that is the fucntion representing the
   * scope, nullptr otherwise */

  llvm::Function *currentScope = nullptr;

  void generateModuleContent();
  /**Utility function to check wheter a function is created or
   * needs to be generated from the proto
   * @param name : function we need to get a handle to
   * @return , pointer to Function, null if not found
   */
  llvm::Function *getFunction(const std::string &name);

  /** This function keeps tracks of the proto crated, so we can
   * generate the corresponding function on the fly */
  std::unordered_map<std::string, PrototypeAST *> functionProtos;
};

} // namespace codegen
} // namespace babycpp
