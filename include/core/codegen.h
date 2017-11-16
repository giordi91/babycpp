#pragma once
#include <fstream>
#include <memory>
#include <unordered_map>

#include "diagnostic.h"
#include "AST.h"
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
    lexer.initFromString(str);
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


  std::string printDiagnostic();


  /**Factory in charge to allocate the factory nodes, it owns the
   * memory */
  memory::FactoryAST factory;

  diagnostic::Diagnostic diagnostic;
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

inline llvm::Type *getType(int type, Codegenerator *gen) {
  if (type == Token::tok_float) {
    return llvm::Type::getFloatTy(gen->context);
  }
  return llvm::Type::getInt32Ty(gen->context);
}
} // namespace codegen
} // namespace babycpp
