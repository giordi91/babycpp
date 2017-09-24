#include <codegen.h>
#include <iostream>

#include "jit.h"
#include <llvm/IR/Verifier.h>

namespace babycpp {
namespace repl {

const std::string ANONYMOUS_FUNCTION = "__anonymous__";
const std::string DUMMY_FUNCTION = "__dummy__";

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
    if (lex->currtok == Token::tok_number)
    {
      return Token::tok_expression_repl;
    }
    if (lex->currtok == Token::tok_open_round) {
      return Token::tok_expression_repl;
    }
    return Token::tok_invalid_repl;
  }
}
void handleExpression(codegen::Codegenerator *gen, BabycppJIT *jit) {

  // the main issue here is that i need to know the resulting type.
  // the code deals with that letting the type bubble up the AST from
  // the leaves, this mean until code gen time I won't know the type.
  // the trick is to add a dummy block of code, let the code being generated
  // there then move the block in the final function, that will let me
  // generate the wrapper function after the body is generated, so I
  // can extract the type
  codegen::PrototypeAST *dummy = gen->factory.allocPrototypeAST(
      0, DUMMY_FUNCTION, std::vector<codegen::Argument>(), 0);
  auto *dummyFunc = static_cast<llvm::Function *>(dummy->codegen(gen));
  using llvm::BasicBlock;
  // Create a new basic block to start insertion into.
  BasicBlock *block = BasicBlock::Create(gen->context, "dummy", dummyFunc);
  gen->builder.SetInsertPoint(block);

  ExprAST *res = gen->parser.parseExpression();
  llvm::Value *val = res->codegen(gen);
  int ret_type = Token::tok_int;
  if (val->getType()->isFloatTy()) {
    ret_type = Token::tok_float;
  }
  // generating the final return function
  codegen::PrototypeAST * final = gen->factory.allocPrototypeAST(
      ret_type, ANONYMOUS_FUNCTION, std::vector<codegen::Argument>(), 0);
  auto *finalFunc = static_cast<llvm::Function *>(final->codegen(gen));
  using llvm::BasicBlock;
  // Create a new basic block to start insertion into.
  BasicBlock *finalblock = BasicBlock::Create(gen->context, "entry", finalFunc);
  gen->builder.SetInsertPoint(finalblock);
  // moving the dummy block
  block->moveAfter(finalblock);
  // now we have the body under the right function
  // deleting the temp one
  block->eraseFromParent();
  dummyFunc->eraseFromParent();
  //we need to add the return
  gen->builder.CreateRet(val);
  std::cout<<gen->printLlvmData(finalFunc)<<std::endl;
  llvm:verifyFunction(*finalFunc);

  babycpp::jit::BabycppJIT::ModuleHandle handle =
  jit->addModule(gen->module);

  if(ret_type == Token::tok_int)
  {
    auto* func = (int(*)())(intptr_t)llvm::cantFail(jit->findSymbol(ANONYMOUS_FUNCTION).getAddress());
    std::cout<<">>> "<<func()<<std::endl;
  }
  //no point in keeping anonymous functions alive
  finalFunc->eraseFromParent();
  jit->removeModule(handle);


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
      std::cout<<"expr"<<std::endl;
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
