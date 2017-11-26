#include <codegen.h>
#include <iostream>

#include "jit.h"
#include <llvm/IR/Verifier.h>

namespace babycpp {
namespace repl {

// hardcoded function names used in the jitting
static const std::string ANONYMOUS_FUNCTION{"__anonymous__"};
static const std::string DUMMY_FUNCTION{"__dummy__"};

using babycpp::codegen::Codegenerator;
using babycpp::codegen::ExprAST;
using babycpp::codegen::FunctionAST;
using babycpp::jit::BabycppJIT;

int lookAheadStatement(babycpp::Parser *parser) {
  {
    auto *lex = parser->lex;
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
    if (parser->isDeclarationToken(lex->currtok, lex->identifierStr)) {
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
    if (lex->currtok == Token::tok_number) {
      return Token::tok_expression_repl;
    }
    if (lex->currtok == Token::tok_open_round) {
      return Token::tok_expression_repl;
    }
    return Token::tok_invalid_repl;
  }
}
void handleExpression(codegen::Codegenerator *gen, BabycppJIT *jit,
                      std::shared_ptr<llvm::Module> anonymousModule,
                      std::shared_ptr<llvm::Module> staticModule) {

  // here we set the current module we want to work on, expression
  // will be added to the anoymous module, so that we can nuke this module
  // easily without worrying about deleting maybe other function definitions
  gen->setCurrentModule(anonymousModule);

  // the main issue here is that i need to know the resulting type of an
  // operation the code deals with that by letting the type bubble up the AST
  // from the leaves, this mean until code gen time I won't know the type. the
  // trick is to add a dummy function  let the code being generated there then
  // move the block in the final function, that will let me generate the wrapper
  // function after the body is generated, so I can extract the type
  codegen::PrototypeAST *dummy = gen->factory.allocPrototypeAST(
      0, DUMMY_FUNCTION, std::vector<codegen::Argument>(), 0);
  auto *dummyFunc = static_cast<llvm::Function *>(dummy->codegen(gen));
  // Create a new basic block to start insertion into.
  using llvm::BasicBlock;
  BasicBlock *block = BasicBlock::Create(gen->context, "entry", dummyFunc);
  gen->builder.SetInsertPoint(block);

  // here we generate the code for our expression
  ExprAST *res = gen->parser.parseExpression();
  llvm::Value *val = res->codegen(gen);
  // now that we have the code we know the returning type and can act
  // accordingly
  int ret_type = Token::tok_int;
  if (val->getType()->isFloatTy()) {
    ret_type = Token::tok_float;
  }
  // generating the final return function
  codegen::PrototypeAST *final = gen->factory.allocPrototypeAST(
      ret_type, ANONYMOUS_FUNCTION, std::vector<codegen::Argument>(), 0);
  auto *finalFunc = static_cast<llvm::Function *>(final->codegen(gen));
  using llvm::BasicBlock;
  // moving the dummy block under the final function with correct return type
  block->removeFromParent();
  block->insertInto(finalFunc);
  // now we have the body under the right function
  // we need to add the return
  gen->builder.CreateRet(val);
  verifyFunction(*finalFunc);

  // proceeding in the jitting
  babycpp::jit::BabycppJIT::ModuleHandle handle = jit->addModule(gen->module);

  // retrieving and evaluating the function
  if (ret_type == Token::tok_int) {
    auto *func = (int (*)())(intptr_t)llvm::cantFail(
        jit->findSymbol(ANONYMOUS_FUNCTION).getAddress());
    // making sure the function has been found
    if (func != nullptr) {
      std::cout << ">>> " << func() << std::endl;
    }
  } else {
    auto *func = (float (*)())(intptr_t)llvm::cantFail(
        jit->findSymbol(ANONYMOUS_FUNCTION).getAddress());
    // making sure the function has been found
    if (func != nullptr) {
      std::cout << ">>> " << func() << std::endl;
    }
  }
  // no point in keeping anonymous functions alive
  // and house keeping
  block->eraseFromParent();
  dummyFunc->eraseFromParent();
  finalFunc->eraseFromParent();
  jit->removeModule(handle);
}

void handleFunction(codegen::Codegenerator *gen, jit::BabycppJIT *jit,
                    std::shared_ptr<llvm::Module> anonymousModule,
                    std::shared_ptr<llvm::Module> staticModule) {
  // here we add the code to the permanent module
  gen->setCurrentModule(staticModule);
  // generating code
  FunctionAST *res = gen->parser.parseFunction();
  if (res == nullptr) {
    return;
  }
  // generating the code and is added to the module
  res->codegen(gen);

  // adding function to the jit so it gets compiled
  jit->addModule(gen->module);
}

void loop(Codegenerator *gen, BabycppJIT *jit,
          std::shared_ptr<llvm::Module> anonymousModule,
          std::shared_ptr<llvm::Module> staticModule) {
  // this is the main loop of the repl
  std::string str;
  while (true) {
    std::cout << ">>> ";
    std::cout.flush();
    // here we get the input for the user, automatically waits
    // unitl input is not provided
    getline(std::cin, str);

    // we initialize the code generator
    gen->initFromString(str);
    // we use our code to look ahead withouth invalidating token
    // to trying to understand what we are dealing with
    // and act accordinly
    int tok = lookAheadStatement(&gen->parser);
    switch (tok) {
    default:
      continue;
    case Token::tok_invalid_repl: {
      std::cout << ">>> I did not understand that" << std::endl;
      continue;
    }
    case Token::tok_expression_repl: {
      handleExpression(gen, jit, anonymousModule, staticModule);
      break;
    }
    case Token::tok_function_repl: {
      handleFunction(gen, jit, anonymousModule, staticModule);
      break;
    }
    }
  }
}

} // namespace repl
} // namespace babycpp
