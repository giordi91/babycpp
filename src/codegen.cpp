#include "codegen.h"
#include <iostream>

#include <llvm/ADT/APFloat.h>



namespace babycpp {
namespace codegen {

using llvm::Value;

Value *NumberExprAST::codegen(Codegenerator* gen) const {

  if (val.type == lexer::NumberType::FLOAT) {
	  return llvm::ConstantFP::get(gen->context, llvm::APFloat(val.floatNumber));

  } else if (val.type == lexer::NumberType::INTEGER) {
	  return llvm::ConstantInt::get(gen->context, llvm::APInt(32,val.integerNumber));
  }
  // this should not be triggered, we should find this errors at
  // parsing time
  std::cout << "Error unrecognized type number on code gen" << std::endl;
  return nullptr;
}
Codegenerator::Codegenerator(): lexer(),parser(&lexer), context()
                                 , builder(context), module("",context)
                                {
}
llvm::Value * VariableExprAST::codegen(Codegenerator * gen) const
{
	return nullptr;
}
} // namespace codegen
} // namespace babycpp
