#include "jit.h"
#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>

namespace babycpp {
namespace jit {
BabycppJIT::BabycppJIT() {

  //here we do the global initialization for llvm
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  //setupping the jit with the memory and required layers
  tm.reset(llvm::EngineBuilder().selectTarget());
  datalayout = new llvm::DataLayout(tm->createDataLayout());
  objectLayer = new llvm::orc::RTDyldObjectLinkingLayer(
      []() { return std::make_shared<llvm::SectionMemoryManager>(); });
  compileLayer =
      new llvm::orc::IRCompileLayer<llvm::orc::RTDyldObjectLinkingLayer,
                                    llvm::orc::SimpleCompiler>(
          *objectLayer, llvm::orc::SimpleCompiler(*tm));
  // when passing null ptr to load lib, it will load the exported symbols of the
  // host process itself making them available for execution, really useful for 
  //exported symbols in the process
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

BabycppJIT::ModuleHandle
BabycppJIT::addModule(std::shared_ptr<llvm::Module> m) {
  // Build our symbol resolver:
  // lambda solvers are symbol solver that needs to find symbols to know if they
  // are already defined, this is easy to do with lambdas, that LLVM provides
  // a way to create such symbol look up out of lambdas
  // Lambda 1: Look back into the JIT itself to find symbols that are part of
  //           the same "logical dylib".
  // Lambda 2: Search for external symbols in the host process.
  auto Resolver = llvm::orc::createLambdaResolver(
      [&](const std::string &name) {
        if (auto Sym = compileLayer->findSymbol(name, false)) {
          return Sym;
        }
        return llvm::JITSymbol(nullptr);
      },
      [](const std::string &Name) {
        if (auto SymAddr =
                llvm::RTDyldMemoryManager::getSymbolAddressInProcess(Name)) {
          return llvm::JITSymbol(SymAddr, llvm::JITSymbolFlags::Exported);
        }
        return llvm::JITSymbol(nullptr);
      });

  // Add the set to the JIT with the resolver we created above and a newly
  // created SectionMemoryManager, this step will trigger compilation right
  // away, meaning it wont be lazy
  return llvm::cantFail(
      compileLayer->addModule(std::move(m), std::move(Resolver)));
}

} // namespace jit
} // namespace babycpp