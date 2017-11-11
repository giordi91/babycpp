#pragma once
#include <unordered_map>

namespace babycpp {
namespace diagnostic {

enum class IssueType { NONE = -1, LEXER = 0, PARSER, AST, CODEGEN };

enum class IssueCode {
  NONE = -1,

  // 0-999 lexer codes

  // 1000-1999 parser codes
  MISSING_OPEN_ROUND_OR_COMMA_IN_FUNC_CALL = 1000,

  // 2000-2999 code gen codes
};

/**
 * \brief map from code to string representation of IssueType
 */
static const std::unordered_map<IssueType, std::string> issueTypeLookUp{
    {IssueType::NONE, "[LEXER]"},
    {IssueType::LEXER, "[PARSER]"},
    {IssueType::AST, "[AST]"},
    {IssueType::CODEGEN, "[CODEGEN]"}};
/**
 * \brief map from code to IssueCode string representation
 */
static const std::unordered_map<IssueCode, std::string> issueCodeLookUp{

    {IssueCode::MISSING_OPEN_ROUND_OR_COMMA_IN_FUNC_CALL,
     "MISSING_OPEN_ROUND_OR_COMMA_IN_FUNC_CALL"}};

} // namespace diagnostic
} // namespace babycpp
