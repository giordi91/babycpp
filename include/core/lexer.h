#pragma once
#include "diagnostic.h"
#include <regex>
#include <string>
#include <unordered_map>

namespace babycpp {
namespace lexer {

// this is the main regex of the whole lexer, let see how compact
// we can keep it meanwhile we add features
static const std::regex MAIN_REGEX(
    R"([ \t]*([[:alpha:]]\w*\b))"      // here we try to catch a common
                                       // identifier either
    R"(|[ \t]*([\d.]+))"               // here we match digits
    R"(|[ \t]*([\(\)\{\}\+-/\*;,<=]))" // parsing supported ascii
    R"(|[ \t]*([\r\n|\r|\n]))"         // catching new line combinations

);

/**
 * @brief Enum defining several useful enums used in the lexer
 * For semplicity it aggregates  other type of tokens.
 * If complexity starts to grow they will be splitted
 * From index -1 to -999 it defines Lexer tokesn
 * From index -1000 to -1999 defines tokens for the repl
 * From index -2000 onward it defines error codes
 */
enum Token {
  tok_eof = -1,
  // functions
  tok_extern = -2,

  // datatypes
  tok_int = -3,
  tok_float = -4,
  tok_string = -5,
  tok_void_ptr = -6,

  // data
  tok_identifier = -7,
  tok_number = -8,
  tok_nullptr = -9,
  // tok_malloc = -10,
  // tok_free = -11,
  // misc
  tok_operator = -12,
  tok_assigment_operator = -13,
  // punctuation
  tok_open_curly = -14,
  tok_close_curly = -15,
  tok_open_round = -16,
  tok_close_round = -17,
  tok_end_statement = -18,
  tok_comma = -19,
  tok_return = -20,

  // flow
  tok_if = -25,
  tok_else = -26,
  tok_else_if = -27,
  tok_for = -28,
  // repl
  tok_invalid_repl = -1000,
  tok_expression_repl = -1001,
  tok_function_repl = -1002,
  tok_assigment_repl = -1003,
  tok_anonymous_assigment_repl = -1004,

  // error codes
  tok_empty_lexer = -2000,
  tok_no_match = -2001,
  tok_malformed_number = -2002,
  tok_unsupported_char = -2003,
};

/**
 * @brief struct representing a 32 bit numerical value
 * At the current time only 32 int and float are supported.
 * Internally an anonymous union is used to be able to read
 * the memory either as float or as int
 */
struct Number {
  // using a anonymous union to be able to read the data
  // both as float or as int based on the type enum
  union {
    /// used to read the data as floating point
    float floatNumber;
    /// used to read the data as signed int
    int integerNumber;
  };
  /// one of the tokens defining  the type of number like
  /// tok_float
  Token type;
};

/**
 * @brief map defining keyword of the language
 * Maps keyword of the language from ascii representation to
 * their token representation
 */
static const std::unordered_map<std::string, Token> KEYWORDS{
    {"int", tok_int},         {"float", tok_float},
    {"string", tok_string},   {"+", tok_operator},
    {"-", tok_operator},      {"*", tok_operator},
    {"<", tok_operator},      {"/", tok_operator},
    {"{", tok_open_curly},    {"}", tok_close_curly},
    {"(", tok_open_round},    {")", tok_close_round},
    {"extern", tok_extern},   {";", tok_end_statement},
    {",", tok_comma},         {"=", tok_assigment_operator},
    {"return", tok_return},   {"if", tok_if},
    {"else", tok_else},       {"for", tok_for},
    {"nullptr", tok_nullptr}, {"void", tok_void_ptr}};

// aliases
using Charmatch = std::match_results<const char *>;

/**
 * @brief struct used to perform look ahead in the lexer
 * This struct is used to save the status of a whole token
 * when gets read. In this way we can store the data whithout
 * actually moving the lexer pointer
 */
struct MovableToken {
  /// current type of token
  int token;
  /// possible name associated with the token
  std::string identifierStr;
  /// possible value associated with the token
  Number value;
  /// offset to add to the column number once we processed it
  int columnOffset;
};

/**
 * @brief struct in charge of the lexical analysis
 * This struct is a regex based lexer, it will progressively work
 * through the data keeping track to where it is, returning the matched
 * tokens meanwhile it proceeds
 */
struct Lexer {

  /** @brief default constructor */
  explicit Lexer(diagnostic::Diagnostic *indiagnostic)
      : expr(MAIN_REGEX), diagnostic(indiagnostic) {}
  /** @brief  constructor
   * @param reg: provide custom regex to apply on the string */
  explicit Lexer(std::regex &reg) : expr(reg) {}

  /**@brief this is the core method to provide data to the lexer
   * @param str: the string we are going to initialize the data
   *             with
   */
  // TODO(giordi) refactor name to be initFromString to be uniform
  // with codegen
  inline void initFromString(const std::string &str) {
    data = str;
    start = data.c_str();
    lineNumber = 1;
    columnNumber = 0;
    lookAheadToken.clear();
  }

  /** @brief process the next token and makes it available to
   *         be analized
   */
  void gettok();

  /** @brief performas a look ahead withouth making the
   * the lexer pointer to move.
   * This means looking ahead  won't change the next token
   * that will be read, intearnally the tokens will be cached
   * and if any token is in the cache it will be took from there
   * rather than be parsed. This method makes the process completely
   * transparent to the user. Once the token are looked ahead can be
   * accessed from the deque "lookAheadToken"
   * @param count: how many tokens we are going to look ahead
   * @return bool, whether or not the look ahead was successiful.
   *               this might fail when the file ends before
   *               the specific amount of tokens are processed
   */
  bool lookAhead(int count);

  // lexed data
  /// the current active token
  int currtok = -1;
  /// possible string value of the processed token
  std::string identifierStr;
  /// possible value of the processed token if it is a numeric value
  Number value;
  /// current line number in the file
  int lineNumber = 1;
  int32_t columnNumber = 1;

  // regex classes
  std::regex expr;
  std::string data;
  Charmatch matcher;
  /// pointer keeping track of where we are in the buffer
  const char *start = nullptr;

  /// buffer of processed tokens for when looking ahead
  std::deque<MovableToken> lookAheadToken;
  diagnostic::Diagnostic *diagnostic;
};

} // namespace lexer
} // namespace babycpp
