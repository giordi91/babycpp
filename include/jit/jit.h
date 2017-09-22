#pragma once
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/Target/TargetMachine.h>

namespace babycpp {
namespace jit {

class BabycppJIT {
public:
  explicit BabycppJIT();

// data
private:
  const llvm::DataLayout datalayout;
  const llvm::orc::RTDyldObjectLinkingLayer objectLayer;
  const llvm::orc::IRCompileLayer<decltype(objectLayer),
                                  llvm::orc::SimpleCompiler>
      compileLayer;

public:
  using ModuleHandle = decltype(compileLayer)::ModuleHandleT;
  std::unique_ptr<llvm::TargetMachine> tm;
};

} // namespace jit
} // namespace babycpp