#pragma once

namespace babycpp {
namespace lexer {
struct Lexer;
}
    namespace codegen{
        struct Codegenerator;
    }
namespace repl {

int lookAheadStatement(lexer::Lexer *lex);
void loop( codegen::Codegenerator* gen);

} // end repl
} // end babycpp