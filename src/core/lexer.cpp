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
  int matcherSize = matcher.size();
  for (int i = 1; i < matcherSize; ++i) {
    if (matcher[i].matched) {
      (*offset) = matcher.length();
      return matcher[i];
    }
  }
  return "";
}

int processNumber(const std::string &str, Lexer *L) {
  const char *ptr = str.c_str();
  int cLen = str.length();
  bool dotFound = false;
  for (int i = 0; i < cLen; ++i) {
    const char c = (*(ptr + i));
    if (isdigit(c) != 0) {
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
    L->value.floatNumber = std::stof(str);
    L->value.type = Token::tok_float;
  } else {
    L->value.integerNumber = std::stoi(str);
    L->value.type = Token::tok_int;
  }
  return tok_number;
}

inline bool isNewLine(const std::string &str) {
  return (str[0] == '\r' || str[0] == '\n');
}

void Lexer::gettok(bool forceNew) {
  // making sure the lexer is initialized
  if (start == nullptr) {
    currtok = tok_empty_lexer;
    return;
  }

  if (!lookAheadToken.empty() && !forceNew) {
    const MovableToken &mov = lookAheadToken.front();
    currtok = mov.token;
    identifierStr = mov.identifierStr;
    value = mov.value;
    columnNumber += mov.columnOffset;
    lookAheadToken.pop_front();
    return;
  }

  bool gotMatch = std::regex_search(start, matcher, expr,
                                    std::regex_constants::match_continuous);

  // in the offset variable we are going to store how many char will be
  // eaten by the token
  int offset = 0;
  std::string extractedString = extractStringFromMatch(matcher, &offset);
  // handling case of not match
  if (!gotMatch) {
    currtok = handleNoMatchFromRegex(start);
    return;
  }

  if (isNewLine(extractedString)) {
    lineNumber++;
    columnNumber = 0;
    start += offset; // eating the token;

    // we skipped the new line and spaces and we go to the new
    // token
    gettok();
    return;
  }

  // handling builtin word
  int tok = isBuiltInKeyword(extractedString);
  if (tok != tok_no_match) {
    start += offset;        // eating the token;
    columnNumber += offset; // adding the offset to the column
    identifierStr = extractedString;
    currtok = tok;
    return;
  }

  // if is not a built in word it must be an identifier or an ascii value
  if (isdigit(extractedString[0]) != 0) {
    // procerssing number since variables are not allowed to start with a number
    start += offset;        // eating the token;
    columnNumber += offset; // adding the offset to the column
    currtok = processNumber(extractedString, this);
    return;
  }
  if (isalpha(extractedString[0]) != 0) {
    start += offset;        // eating the token;
    columnNumber += offset; // adding the offset to the column
    identifierStr = extractedString;
    currtok = tok_identifier;
    return;
  }
}
bool Lexer::lookAhead(int count) {

  // if we already have enough token in the list we just get out
  if (count <= lookAheadToken.size()) {
    return true;
  }

  // if we already have token in the look ahead we just get the difference
  bool forceNew = false;
  if (lookAheadToken.size() != 0) {
    count -= lookAheadToken.size();
    // here we set forceNew so the gettok() method wont be popping from the
    // queue  but only getting extra new token that are needed, that is why we do the
	//difference
    forceNew = true;
  }

  std::string old = identifierStr;
  int old_tok = currtok;
  Number oldNumb = value;

  // here we need a temp buffer, we cannot push directly inside
  // the lookAheadToken queue, otherwise it will be popped right
  // away from the next gettok(). We write to a temp buffer and then
  // transfer to the queue, not super pretty, might be a better way
  std::vector<MovableToken> tempBuffer;
  tempBuffer.reserve(count);

  for (int t = 0; t < count; ++t) {
    gettok(forceNew);
    if (currtok == tok_eof || currtok == tok_no_match) {
      return false;
    }
    // TODO(giordi): here we are passing 0 as column offset which is wrong and
    // might be misleading need to find a nice way to get that offset, probably
    // having gettok() returning the number of char read
    tempBuffer.emplace_back(MovableToken{currtok, identifierStr, value, 0});
  }

  for (int t = 0; t < count; ++t) {
    lookAheadToken.push_back(tempBuffer[t]);
  }
  identifierStr = old;
  currtok = old_tok;
  value = oldNumb;
  return true;
}

} // namespace lexer
} // namespace babycpp
