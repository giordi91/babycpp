#include <codegen.h>
#include <iostream>

#include "jit.h"

namespace babycpp {
namespace repl {

const std::string ANONYMOUS_FUNCTION = "__anonymous__";

using babycpp::codegen::Codegenerator;
using babycpp::codegen::ExprAST;
using babycpp::jit::BabycppJIT;

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
      return Token::tok_invalid_repl;
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
void handleExpression(codegen::Codegenerator *gen, BabycppJIT *jit) {

  ExprAST *res = gen->parser.parseExpression();
  // llvm::Value *val = res->codegen(gen);
  if (res->datatype ==0)
  {
    std::cout<<"error cannot deduce datatype"<<std::endl;
    return;
  }
  codegen::PrototypeAST *proto = gen->factory.allocPrototypeAST(res->datatype,
      ANONYMOUS_FUNCTION, std::vector<codegen::Argument>(),0);

  // faking expression as to be return;
  res->flags.isReturn = true;
  std::vector<ExprAST *> body{res};
  codegen::FunctionAST *func = gen->factory.allocFunctionAST(proto, body);
  func->codegen(gen);

  babycpp::jit::BabycppJIT::ModuleHandle handle = jit->addModule(gen->module);
}

void loop(Codegenerator *gen, BabycppJIT *jit) {
  std::string str;
  while (true) {
    std::cout << ">>> ";
    std::cout.flush();
    getline(std::cin, str);

    gen->initFromString(str);
    int tok = lookAheadStatement(&gen->lexer);
    switch (tok) {
    default:
      continue;
    case Token::tok_invalid_repl: {
      std::cout << ">>> I did not understand that" << std::endl;
      continue;
    }
    case Token::tok_expression_repl: {
      handleExpression(gen, jit);
    }
    }
    if (str == "exit") {
      break;
    }
  }
}

} // end repl
} // end babycpp
