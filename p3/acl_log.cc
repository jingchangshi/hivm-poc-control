#include "acl_log.h"

#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <ctime>

namespace p3 {

std::string UtcNow() {
  using namespace std::chrono;
  const auto now = system_clock::now();
  const auto secs = time_point_cast<seconds>(now);
  const auto ms = duration_cast<milliseconds>(now - secs).count();
  const std::time_t t = system_clock::to_time_t(now);
  std::tm tm{};
  gmtime_r(&t, &tm);
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                tm.tm_min, tm.tm_sec, static_cast<int>(ms));
  return std::string(buf);
}

std::string MonotonicNow() {
  using namespace std::chrono;
  const auto now = steady_clock::now().time_since_epoch();
  const auto us = duration_cast<microseconds>(now).count();
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64, us / 1000000,
                us % 1000000);
  return std::string(buf);
}

bool AclLog::Open(const std::string& path) {
  fp_ = std::fopen(path.c_str(), "wx");  // exclusive: write-once
  if (fp_ == nullptr) {
    fp_ = std::fopen(path.c_str(), "w");
  }
  if (fp_ == nullptr) return false;
  std::setvbuf(fp_, nullptr, _IOLBF, 0);
  return true;
}

void AclLog::Emit(const char* level, const char* op, const std::string& detail,
                  aclError rc, bool has_rc) {
  if (fp_ == nullptr) return;
  if (has_rc) {
    std::fprintf(fp_, "%s mono=%s utc=%s level=%s op=%s rc=%d%s%s\n",
                 MonotonicNow().c_str(), MonotonicNow().c_str(), UtcNow().c_str(),
                 level, op, rc, detail.empty() ? "" : " detail=",
                 detail.c_str());
  } else {
    std::fprintf(fp_, "%s utc=%s level=%s op=%s%s%s\n", MonotonicNow().c_str(),
                 UtcNow().c_str(), level, op, detail.empty() ? "" : " detail=",
                 detail.c_str());
  }
  std::fflush(fp_);
}

aclError AclLog::Log(const char* op, aclError rc, const std::string& detail) {
  ++call_count_;
  // Copy the recent error message IMMEDIATELY: any later ACL call may replace it.
  const char* msg = aclGetRecentErrMsg();
  const std::string msg_s = (msg == nullptr) ? "<null>" : std::string(msg);

  std::string full = detail;
  full += " recent_err_msg=\"";
  full += msg_s;
  full += "\"";

  if (rc != ACL_SUCCESS) {
    ++nonzero_rc_count_;
    Emit("ERROR", op, full, rc, true);
    RecordFirstFailure(std::string(op) + " rc=" + std::to_string(rc) +
                       " recent_err_msg=\"" + msg_s + "\"");
  } else {
    Emit("INFO", op, full, rc, true);
  }
  return rc;
}

void AclLog::Note(const char* op, const std::string& detail) {
  Emit("NOTE", op, detail, 0, false);
}

void AclLog::RecordFirstFailure(const std::string& what) {
  if (first_failure_.empty()) {
    first_failure_ = what;
  }
}

void AclLog::Flush() {
  if (fp_ != nullptr) std::fflush(fp_);
}

}  // namespace p3
