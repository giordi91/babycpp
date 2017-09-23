#include <iostream>
#include <codegen.h>

namespace babycpp {
    namespace repl {


using babycpp::codegen::Codegenerator;

int lookAheadStatement(babycpp::Lexer *lex) {
  {
    // invalid tokens
    if (lex->currtok == Token::tok_eof) {
      return Token::tok_invalid_repl;
    }
    if (lex->currtok == Token::tok_extern) {
      return Token::tok_extern;
    }
    if (lex->currtok == Token::tok_return) {
      return Token::tok_invalid_repl;
    }

    // parsing a declaration
    if (Parser::isDeclarationToken(lex->currtok)) {
      bool res = lex->lookAhead(2);
      if (!res) {
        return Token::tok_invalid_repl;
      }
      if (lex->lookAheadToken[0].token == Token::tok_identifier &&
          lex->lookAheadToken[1].token == Token::tok_assigment_operator) {
        return Token::tok_assigment_repl;
      }
      if (lex->lookAheadToken[0].token == Token::tok_identifier &&
          lex->lookAheadToken[1].token == Token::tok_open_round) {
        return Token::tok_function_repl;
      }
    }

    // if just an identifier we deal with it as an expression
    if (lex->currtok == Token::tok_identifier) {

      bool res = lex->lookAhead(2);
      if (!res) {
        return Token::tok_invalid_repl;
      }
      if (lex->lookAheadToken[0].token == Token::tok_assigment_operator) {
        return Token::tok_anonymous_assigment_repl;
      }
      return Token::tok_expression_repl;
    }
    if (lex->currtok == Token::tok_open_round) {
      return Token::tok_expression_repl;
    }
    return Token::tok_invalid_repl;
  }
}

void loop( Codegenerator* gen)
{
  std::string str;
  while(true) {
    std::cout << ">>> ";
    std::cout.flush();
    getline(std::cin, str);

    gen->initFromString(str);
    int tok = lookAheadStatement(&gen->lexer);
  }
}


    } // end repl
} // end babycpp
