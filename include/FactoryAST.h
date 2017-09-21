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

    template <typename T, typename... Args> T *allocASTNode(Args... args) {
      return new T(args...);
    }
    template <typename T> T test() { return T(); };

    template <typename... Args>
    codegen::VariableExprAST *allocVariableAST(Args &&... args) {
      return allocASTNode<codegen::VariableExprAST>(
          std::forward<Args...>(args));
    }

    SlabAllocator alloc;
  };

  } // namespace memory
} // namespace babycpp
