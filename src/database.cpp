#include "database.h"

const std::map<char, int > Database::binopPrecedence ={ {'<', 10}, {'+', 20}, {'-', 20}, {'*', 40} };
