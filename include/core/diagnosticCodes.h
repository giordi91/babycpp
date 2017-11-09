#pragma once
#include <unordered_map>

namespace babycpp {
namespace diagnostic {

enum class IssueType { NONE = -1, LEXER = 0, PARSER, AST, CODEGEN };

enum class IssueCode { NONE = -1 };

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
static const std::unordered_map<int, std::string> issueCodeLookUp{};

} // namespace diagnostic
} // namespace babycpp
