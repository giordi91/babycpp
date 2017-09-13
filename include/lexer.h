#pragma once
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>
#include <queue>

namespace babycpp {
namespace lexer {

static const std::regex MAIN_REGEX(
    R"([ \t]*([[:alpha:]]\w*\b[*]?))" // here we try to catch a common
                                      // identifier either
    R"(|[ \t]*([\d.]+))"              // here we match digits
    R"(|[ \t]*([\(\)\{\}\+-/\*;,<]))" // parsing supported ascii
    R"(|[ \s]([\r\n|\r|\n]))"         // catching new line combinations

);

enum Token {
  tok_eof = -1,
  // functions
  tok_extern = -2,

  // datatypes
  tok_int = -3,
  tok_float = -4,
  tok_string = -5,

  // data
  tok_identifier = -6,
  tok_number = -7,
  // misc
  tok_operator = -8,
  // punctuation
  tok_open_curly = -9,
  tok_close_curly = -10,
  tok_open_round = -11,
  tok_close_round = -12,
  tok_end_statement = -13,
  tok_comma = -14,

  // error codes
  tok_empty_lexer = -2000,
  tok_no_match = -2001,
  tok_malformed_number = -2002,
  tok_unsupported_char = -2003,
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

static const std::unordered_map<std::string, Token> KEYWORDS{
	{"int", tok_int},       {"float", tok_float},    {"string", tok_string},
	{"+", tok_operator},    {"-", tok_operator},     {"*", tok_operator},
	{"<", tok_operator},    {"/", tok_operator},     {"{", tok_open_curly},
	{"}", tok_close_curly}, {"(", tok_open_round},   {")", tok_close_round},
	{"extern", tok_extern}, {";", tok_end_statement}, {",",tok_comma} };
// aliases
using Charmatch = std::match_results<const char *>;
// int gettok(Database &D);

struct MovableToken
{
    int token;
    std::string identifierStr;
    Number value;
};

struct Lexer {

  Lexer() : expr(MAIN_REGEX) {}
  Lexer(std::regex &reg) : expr(reg) {}

  void inline initFromStr(const std::string &str) {
    data = str;
    start = data.c_str();
    lineNumber = 1;
  }
  void gettok();
  bool lookAhead(uint32_t count);

  // lexed data
  int currtok = -1;
  std::string identifierStr;
  Number value;
  uint32_t lineNumber = 1;

  // regexd classes
  std::regex expr;
  std::string data;
  Charmatch matcher;
  const char *start = nullptr;

  //act as a buffer for when doing look ahead
  std::queue<MovableToken> lookAheadToken;
};

} // namespace lexer
} // namespace babycpp
