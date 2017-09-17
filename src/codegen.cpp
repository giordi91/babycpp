#include "codegen.h"
#include <iostream>

namespace babycpp {
namespace codegen {
Value *NumberExprAST::codegen() const {

  if (val.type == lexer::NumberType::FLOAT) {

  } else if (val.type == lexer::NumberType::INTEGER) {
  }

  // this should not be triggered, we should find this errors at
  // parsing time
  std::cout << "Error unrecognized type nuber on code gen" << std::endl;
  return nullptr;
}
} // namespace codegen
} // namespace babycpp
