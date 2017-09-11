#pragma once
#include <fstream>
#include <regex>
#include <string>

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
  // tok_def = -3,
  // datatypes
  tok_int = -3,
  tok_float = -4,
  tok_string = -5,
  tok_identifier = -6,

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
  std::match_results<const char *> matcher;
  const char *start = nullptr;
};

} // namespace lexer
} // namespace babycpp
