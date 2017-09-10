#pragma once

#include "database.h"

namespace babycpp {
namespace lexer {

enum Token {
  tok_eof = -1,
  // functions
  tok_extern = -2,
  //tok_def = -3,
  // datatypes
  tok_int = -3,
  tok_float = -4,
  tok_string = -5,
  tok_identifier = -6
};

int gettok(Database &D);
} // namespace lexer
} // namespace babycpp
