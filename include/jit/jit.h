#pragma once
#include "codegen.h"

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/Mangler.h>

namespace babycpp {
namespace jit {

class BabycppJIT {
public:
  explicit BabycppJIT();

  // data
private:
  const llvm::DataLayout* datalayout;
  llvm::orc::RTDyldObjectLinkingLayer* objectLayer;
  llvm::orc::IRCompileLayer<llvm::orc::RTDyldObjectLinkingLayer, llvm::orc::SimpleCompiler>*
      compileLayer;
  std::unique_ptr<llvm::TargetMachine> tm;



public:
using ModuleHandle = llvm::orc::IRCompileLayer<llvm::orc::RTDyldObjectLinkingLayer, llvm::orc::SimpleCompiler>::ModuleHandleT;
  ModuleHandle addModule(std::shared_ptr<llvm::Module> m);

    llvm::JITSymbol findSymbol(const std::string Name) {
      std::string MangledName;
      llvm::raw_string_ostream MangledNameStream(MangledName);
      llvm::Mangler::getNameWithPrefix(MangledNameStream, Name, *datalayout);
      return compileLayer->findSymbol(MangledNameStream.str(), true);
    }

    llvm::JITTargetAddress getSymbolAddress(const std::string Name) {
      return cantFail(findSymbol(Name).getAddress());
    }

    void removeModule(ModuleHandle h) {
      llvm::cantFail(compileLayer->removeModule(h));
    }
};

} // namespace jit
} // namespace babycpp