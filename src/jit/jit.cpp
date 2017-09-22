#include "jit.h"
#include <iostream>
#include <memory>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/DynamicLibrary.h>

namespace babycpp {
namespace jit {
BabycppJIT::BabycppJIT(): tm(llvm::EngineBuilder().selectTarget()),
                          datalayout(tm->createDataLayout()), 
						  objectLayer([]() {return std::make_shared<llvm::SectionMemoryManager>(); }),
						  compileLayer(objectLayer, llvm::orc::SimpleCompiler(*tm))
{
	//when passing null ptr to load lib, it will load the exported symbols of the
	//host process itself making them available for execution
	llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

} // namespace jit
} // namespace babycpp