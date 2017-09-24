#pragma once

namespace babycpp {
namespace lexer {
struct Lexer;
}
namespace codegen {
struct Codegenerator;
}
namespace jit {
class BabycppJIT;
}
namespace repl {

static const std::string ANONYMOUS_FUNCTION;
static const std::string DUMMY_FUNCTION;

/**
 * @brief look ahead to understand what we are dealing
 * In order for the repl to behaver correctly, we need to know if we
 * are dealing with a function declaration, an expression and assignemnt
 * etc, and trying to generate supplementary code to deal with it
 * @param lex , lexer pointer
 * @return a repl token represening what the string is
 */
int lookAheadStatement(lexer::Lexer *lex);

/**
 * @brief main repl loop
 * @param gen
 */
void loop(codegen::Codegenerator *gen, jit::BabycppJIT *jit,
          std::shared_ptr<llvm::Module> anonymousModule,
          std::shared_ptr<llvm::Module> staticModule);

void handleExpression(codegen::Codegenerator *gen, jit::BabycppJIT *jit,
                      std::shared_ptr<llvm::Module> anonymousModule,
                      std::shared_ptr<llvm::Module> staticModule);

void handleFunction(codegen::Codegenerator *gen, jit::BabycppJIT *jit,
                      std::shared_ptr<llvm::Module> anonymousModule,
                      std::shared_ptr<llvm::Module> staticModule);
} // end repl
} // end babycpp