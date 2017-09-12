#include "lexer.h"
#include <iostream>

namespace babycpp {
namespace lexer {

int inline handleNoMatchFromRegex(const char *start) {
  if ((*start == EOF) || (*start == 0)) {
    return tok_eof;
  }
  // log ERROR
  return tok_no_match;
}

int inline isBuiltInKeyword(const std::string &str) {
  const auto iter = KEYWORDS.find(str);
  if (iter != KEYWORDS.end()) {
    return iter->second;
  }
  return tok_no_match;
}

inline std::string extractStringFromMatch(const Charmatch &matcher,
                                          int *offset) {
  uint32_t matcherSize = matcher.size();
  for (uint32_t i = 1; i < matcherSize; ++i) {
    if (matcher[i].matched) {
      (*offset) = matcher.length();
      return matcher[i];
    }
  }
  return "";
}

int processNumber(std::string &str, Lexer &L) {
  const char *ptr = str.c_str();
  uint32_t cLen = str.length();
  bool dotFound = false;
  for (uint32_t i = 0; i < cLen; ++i) {
    const char c = (*(ptr + i));
    if (isdigit(c)) {
      continue;
    }
    if (c == '.' && !dotFound) {
      dotFound = true;
      continue;
    }
    return tok_malformed_number;
  }

  // if we get to this point it is a valid number!
  if (dotFound) {
    // it means is a floating point
    L.value.floatNumber = std::stof(str);
    L.value.type = NumberType::FLOAT;
  } else {
    L.value.integerNumber = std::stoi(str);
    L.value.type = NumberType::INTEGER;
  }
  return tok_number;
}

bool isNewLine(std::string &identifierStr) {
  if (identifierStr[0] == '\r' || identifierStr[0] == '\n') {
    return true;
  }
  return false;
}

int Lexer::gettok() {
  // making sure the lexer is initialized
  if (start == nullptr) {
    return tok_empty_lexer;
  }

  bool gotMatch = std::regex_search(start, matcher, expr,
                                    std::regex_constants::match_continuous);

  // in the offset variable we are going to store how many char will be
  // eaten by the token
  int offset;
  std::string extractedString = extractStringFromMatch(matcher, &offset);
  // handling case of not match
  if (!gotMatch) {
    return handleNoMatchFromRegex(start);
  }

  if (isNewLine(extractedString)) {
    lineNumber++;
    start += offset; // eating the token;
    // we skipped the new line and spaces and we go to the new
    // token
    return gettok();
  }

  // handling builtin word
  int tok = isBuiltInKeyword(extractedString);
  if (tok != tok_no_match) {
    start += offset; // eating the token;
    identifierStr = extractedString;
    return tok;
  }

  // if is not a built in word it must be an identifier or an ascii value
  if (isdigit(extractedString[0])) {
    // procerssing number since variables are not allowed to start with a number
    start += offset; // eating the token;
    return processNumber(extractedString, *this);
  } else if (isalpha(extractedString[0])) {
    start += offset; // eating the token;
    identifierStr = extractedString;
    return tok_identifier;
  }

  // should never reach this part of the code since
  // if not supported we should hit the no match branch
  return tok_unsupported_char;
}

} // namespace lexer
} // namespace babycpp
