// ACL call logger.
//
// Every ACL call in the P3 harness goes through Log(): the operation name, the
// returned aclError, aclGetRecentErrMsg() copied IMMEDIATELY (before any other
// ACL call can overwrite it), and both a UTC and a monotonic timestamp.
//
// The first failure is preserved and never overwritten by later cleanup errors
// (goal section 7.4 / design section 5).
#pragma once

#include <string>

#include "acl/acl.h"

namespace p3 {

class AclLog {
 public:
  AclLog() = default;

  // Opens the log file. Returns false on failure.
  bool Open(const std::string& path);

  // Records one ACL call. `detail` is free-form extra context (sizes, ids...).
  // Returns rc unchanged so call sites can write `if (Log(...) != ACL_SUCCESS)`.
  aclError Log(const char* op, aclError rc, const std::string& detail);

  // Records a non-ACL event (informational, or a failure of the harness itself).
  void Note(const char* op, const std::string& detail);

  // Records the first failure without overwriting an earlier one.
  void RecordFirstFailure(const std::string& what);

  bool has_failure() const { return !first_failure_.empty(); }
  const std::string& first_failure() const { return first_failure_; }

  // Number of ACL calls logged that returned nonzero.
  long nonzero_rc_count() const { return nonzero_rc_count_; }

  void Flush();

 private:
  void Emit(const char* level, const char* op, const std::string& detail,
            aclError rc, bool has_rc);

  FILE* fp_ = nullptr;
  std::string first_failure_;
  long nonzero_rc_count_ = 0;
  long call_count_ = 0;
};

// UTC (ISO-8601, Z) and monotonic (seconds since an arbitrary epoch) strings.
std::string UtcNow();
std::string MonotonicNow();

}  // namespace p3
