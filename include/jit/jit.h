#pragma once
#include "codegen.h"

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Target/TargetMachine.h>

namespace babycpp {
namespace jit {

class BabycppJIT {
public:
  explicit BabycppJIT();

  // data
private:
  // here we had to initialize everything as pointers, so we can initalize them
  // in the constructor and do explicit initialization of the native machine
  // assembler etc, if we doit outside the class for some reason did not work
  const llvm::DataLayout *datalayout;
  llvm::orc::RTDyldObjectLinkingLayer *objectLayer;
  llvm::orc::IRCompileLayer<llvm::orc::RTDyldObjectLinkingLayer,
                            llvm::orc::SimpleCompiler> *compileLayer;
  std::unique_ptr<llvm::TargetMachine> tm;

public:
  using ModuleHandle =
      llvm::orc::IRCompileLayer<llvm::orc::RTDyldObjectLinkingLayer,
                                llvm::orc::SimpleCompiler>::ModuleHandleT;
  ModuleHandle addModule(std::shared_ptr<llvm::Module> m);

  inline llvm::JITSymbol findSymbol(const std::string Name) {
    std::string MangledName;
    llvm::raw_string_ostream MangledNameStream(MangledName);
    llvm::Mangler::getNameWithPrefix(MangledNameStream, Name, *datalayout);
    // here the false is really important, it stands for exportedSymbol only.
    // if you have that set to true, you won't be able to find symbols on
    // windows  since they are not exported by default.
    return compileLayer->findSymbol(MangledNameStream.str(), false);
  }

  inline llvm::JITTargetAddress getSymbolAddress(const std::string Name) {
    return cantFail(findSymbol(Name).getAddress());
  }

  inline void removeModule(ModuleHandle h) {
    llvm::cantFail(compileLayer->removeModule(h));
  }
};

} // namespace jit
} // namespace babycpp