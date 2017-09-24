//
// Created by giordi on 23/09/17.
//
#include <iostream>
#include <codegen.h>
#include "repl.h"
#include "jit.h"
#include <string>

using babycpp::codegen::Codegenerator;
int main()
{
  std::cout<<"Babycpp v 0.0.1 ... not my fault if it crash"<<std::endl;
  //creating the code generator
  Codegenerator gen;
  babycpp::jit::BabycppJIT jit;

  babycpp::repl::loop(&gen, &jit);


  return 0;
}
