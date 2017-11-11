#pragma once
#include <deque>
#include <unordered_map>

#include <diagnosticCodes.h>

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
  inline bool hasWarning() const { return !warnings.empty(); }
  inline bool hasErrors() const { return !errors.empty(); }
  inline const Issue &peakError() const { return errors.front(); }
  inline Issue getError() {
    auto err = errors.front();
    errors.pop_front();
    return err;
  }
  inline void pushError(Issue &issue) { errors.push_back(issue); }
  inline const Issue &peakWarning() const { return warnings.front(); }
  inline Issue getWarning() {
    auto wan = warnings.front();
    warnings.pop_front();
    return wan;
  }
  inline void pushWarning(Issue &issue) { warnings.push_back(issue); }

  void clear() {
    errors.clear();
    warnings.clear();
  }

private:
  std::deque<Issue> errors;
  std::deque<Issue> warnings;
};

} // namespace diagnostic
} // namespace babycpp
