#pragma once

#include "codegen.h"
#include "slabAllocator.h"
#include <vector>

namespace babycpp {
{
  namespace codegen {
  struct ExprAST;
  struct VariableExprAST;
  }

  namespace memory {
  struct FactoryAST {

    FactoryAST() = default;

    template <typename T, typename... Args> T *allocateASTNode(Args... args) {
      return new T(args...);
    }
    template <typename T> T test() { return T(); };
  };

      using allocVariableAST = codegen::VariableExprAST *(allocateASTNode)(
          codegen::VariableExprAST, std::string &, codegen::ExprAST *, int);
      SlabAllocator alloc;
  } // namespace memory
} // namespace babycpp
