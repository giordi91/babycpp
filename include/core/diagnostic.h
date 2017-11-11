#pragma once
#include <diagnosticCodes.h>

#include <deque>
#include <sstream>

namespace babycpp {
namespace diagnostic {

struct Issue {
  const std::string message{""};
  int line = 0;
  int column = 0;
  IssueType type = IssueType::NONE;
  IssueCode code = IssueCode::NONE;
};

class Diagnostic {
public:
  inline int hasErrors() const { return errors.size(); }
  inline const Issue &peakError() const { return errors.front(); }
  inline Issue getError() {
    auto err = errors.front();
    errors.pop_front();
    return err;
  }
  inline void pushError(Issue &issue) { errors.push_back(issue); }
  inline std::string printErorr(Issue &issue) {
    //
    std::ostringstream oss;
    oss << "[ERROR ";

    std::string errorType;
    auto typeFound = issueTypeLookUp.find(issue.type);
    if (typeFound != issueTypeLookUp.end()) {
      errorType = typeFound->second;
    } else {
      errorType = "UNKNOWN_ERROR_TYPE";
    }
	oss << errorType << ": ";

    std::string errorCode;
    auto found = issueCodeLookUp.find(issue.code);
    if (found != issueCodeLookUp.end()) {
      errorCode = found->second;
    } else {
      errorCode = "UNKNOWN_ERROR_CODE";
    }
	oss << errorCode << " ] at line "<< issue.line<< " column "<<issue.column<<": " << issue.message;
	return oss.str();
  }

  inline int hasWarning() const { return warnings.size(); }
  inline const Issue &peakWarning() const { return warnings.front(); }
  inline Issue getWarning() {
    auto wan = warnings.front();
    warnings.pop_front();
    return wan;
  }
  inline void pushWarning(Issue &issue) { warnings.push_back(issue); }

  void clear() {
    errors.resize(0);
    warnings.resize(0);
  }

private:
  std::deque<Issue> errors;
  std::deque<Issue> warnings;
};

} // namespace diagnostic
} // namespace babycpp
