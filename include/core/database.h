#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

//#include "llvm/IR/IRBuilder.h"
//#include "llvm/IR/LLVMContext.h"
//#include "llvm/IR/Module.h"

struct Database {

  //Database() : context(), builder(context), module("",context), namedValues(){};
  Database() =default;
  ~Database() = default;

  // deleted copy constructor and assigment operator
  Database(Database const &) = delete;
  Database &operator=(Database const &) = delete;

  inline char getchar() { return stream.get(); }

  bool openFile(const std::string &path) {
    // looks like ifstream does not accept a string view in the constructor
    auto file = std::ifstream(path, std::ios::in);
    if (file) {
      reset();
      stream << file.rdbuf();
      file.close();
      // initializing the first char so we are good to go
      lastChar = this->getchar();
      return true;
    }
    return false;
  }

  void initFromStr(const std::string str) {
    if (!str.empty()) {
      reset();
      stream << str;
      lastChar = this->getchar();
    }
  }

  void reset() {
    stream = std::stringstream();
    lastChar = -1;
    currtok = -1;
    numVal = 0.0;
    identifierStr.clear();
  }
  inline int getTokPrecedence() const {
    auto found = binopPrecedence.find(currtok);
    if (found != binopPrecedence.end())
      return found->second;

    return -1;
  }

  // Data
  int lastChar = -1;
  int currtok = -1;
  double numVal = 0.0;
  std::string identifierStr;
  const static std::map<char, int> binopPrecedence;
  std::stringstream stream;

  //used for IR code gen
  //llvm::LLVMContext context;
  //llvm::IRBuilder<> builder;
  //llvm::Module module;
  //std::map<std::string, llvm::Value *> namedValues;
};
