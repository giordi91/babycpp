#pragma once

namespace babycpp {
namespace lexer {
struct Lexer;
}
namespace codegen {
struct Codegenerator;
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
int lookAheadStatement(lexer::Lexer *lex);
void loop(codegen::Codegenerator *gen);

} // end repl
} // end babycpp