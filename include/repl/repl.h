#pragma once

namespace babycpp {

// forward declarations
namespace lexer {
struct Lexer;
}
namespace parser{
struct Parser;
}
namespace codegen {
struct Codegenerator;
}
namespace jit {
class BabycppJIT;
}

namespace repl {

/**
 * @brief look ahead to understand what we are dealing
 * In order for the repl to behaver correctly, we need to know if we
 * are dealing with a function declaration, an expression and assignemnt
 * etc, and trying to generate supplementary code to deal with it
 * @param lex , lexer pointer
 * @return a repl token represening what the string is
 */
int lookAheadStatement(parser::Parser *parser);

/**
 * @brief main repl loop
 * @param gen
 */
void loop(codegen::Codegenerator *gen, jit::BabycppJIT *jit,
          std::shared_ptr<llvm::Module> anonymousModule,
          std::shared_ptr<llvm::Module> staticModule);

/**
* @brief parses an expression and jit compiles it
* @param gen: pointer to the code generator used for parsing and IR gen
* @param jit: pointer to the jit class which will ingest and compile the IR
* @param anonymousModule: module containing the anonymous expression code
* @param staticModule: this module will contain all the defined functions that
                       can be used in expressions
*/
void handleExpression(codegen::Codegenerator *gen, jit::BabycppJIT *jit,
                      std::shared_ptr<llvm::Module> anonymousModule,
                      std::shared_ptr<llvm::Module> staticModule);

/**
* @brief parses  a functions and jit compiles it, ready to be called
* @param gen: pointer to the code generator used for parsing and IR gen
* @param jit: pointer to the jit class which will ingest and compile the IR
* @param anonymousModule: module containing the anonymous expression code
* @param staticModule: this module will contain all the defined functions that
                       can be used in expressions
*/
void handleFunction(codegen::Codegenerator *gen, jit::BabycppJIT *jit,
                    std::shared_ptr<llvm::Module> anonymousModule,
                    std::shared_ptr<llvm::Module> staticModule);
} // namespace repl
} // namespace babycpp