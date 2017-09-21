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

      //in general I am not a super fan of hardcore or complex templates
      //but I decided to experiment a little with it.
      //This is the inner function, a generic function getting an arbitrary
      //number of arguments and forwarding them to the constructor
    template <typename T, typename... Args> T *allocASTNode(Args... args) {

        return new(allocator.alloc(sizeof(T))) T(args...);
    }

    template <typename... Args>
    codegen::VariableExprAST *allocVariableAST(Args &&... args) {
      return allocASTNode<codegen::VariableExprAST>(
          std::forward<Args...>(args));
    }

    SlabAllocator allocator;
  };

  } // namespace memory
} // namespace babycpp
