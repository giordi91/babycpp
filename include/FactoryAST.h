#pragma once

#include "codegen.h"
#include "slabAllocator.h"
#include <vector>

namespace babycpp {

namespace codegen {
struct ExprAST;
struct VariableExprAST;
}

namespace memory {
struct FactoryAST {

  FactoryAST() = default;

  // in general I am not a super fan of hardcore or complex templates
  // but I decided to experiment a little with it.
  // This is the inner function, a generic function getting an arbitrary
  // number of arguments and forwarding them to the constructor
  template <typename T, typename... Args> T *allocASTNode(Args... args) {
    return new (allocator.alloc(sizeof(T))) T(args...);
  }

  // generating aliases for the different nodes
  template <typename... Args>
  codegen::VariableExprAST *allocVariableAST(Args &&... args) {
    return allocASTNode<codegen::VariableExprAST>(args...);
  }
  template <typename... Args>
  codegen::NumberExprAST *allocNuberAST(Args ... args) {
    return allocASTNode<codegen::NumberExprAST>(args...);
  }
  template <typename... Args>
  codegen::BinaryExprAST *allocBinaryAST(Args &&... args) {
    return allocASTNode<codegen::BinaryExprAST>(args...);
  }
  template <typename... Args>
  codegen::CallExprAST*allocCallexprAST(Args &&... args) {
    return allocASTNode<codegen::CallExprAST>(args...);
  }
    template <typename... Args>
    codegen::PrototypeAST*allocPrototypeAST(Args &&... args) {
      return allocASTNode<codegen::PrototypeAST>(args...);
    }
    template <typename... Args>
    codegen::FunctionAST*allocFunctionAST(Args &&... args) {
      return allocASTNode<codegen::FunctionAST>(args...);
    }

  SlabAllocator allocator;
};

} // namespace memory
} // namespace babycpp
