#pragma once
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>

namespace babycpp {
namespace lexer {

static const std::regex MAIN_REGEX(
    R"(\s*([[:alpha:]]\w*\b[*]?))" // here we try to catch a common identifier
                                   // either
    R"(|\s*([+-]?[\d.]+))");       // here we match digits

enum Token {
  tok_eof = -1,
  // functions
  tok_extern = -2,
  
  // datatypes
  tok_int = -3,
  tok_float = -4,
  tok_string = -5,
  tok_identifier = -6,
  //misc
  tok_operator = -7,

  // error codes
  tok_empty_lexer = -2000,
  tok_no_match = -2001
};

enum class NumberType { INTEGER = 0, FLOAT = 1 };

struct Number {
  // using a anonymous union to be able to read the data
  // both as float or as int based on the type enum
  union {
    float floatNumber;
    int integerNumber;
  };
  NumberType type;
};

static const std::unordered_map<std::string, Token> KEYWORDS {
                                                        {"int", tok_int}, 
                                                        {"float", tok_float}, 
                                                        {"string",tok_string},
                                                        {"+",tok_operator},
                                                        {"-",tok_operator},
                                                        {"*",tok_operator},
                                                        {"/",tok_operator}
                                                    };
//aliases
using Charmatch = std::match_results<const char*>;
// int gettok(Database &D);

struct Lexer {

  Lexer() : expr(MAIN_REGEX) {}
  Lexer(std::regex &reg) : expr(reg) {}

  void inline initFromStr(const std::string &str) {
    m_data = str;
    start = m_data.c_str();
  }
  int gettok();

  // lexed data
  int currtok = -1;
  std::string identifierStr;
  Number value;

  // regexd classes
  std::regex expr;
  std::string m_data;
  Charmatch  matcher;
  const char *start = nullptr;
};

} // namespace lexer
} // namespace babycpp
