// P3 independent ACL harness.
//
// Purpose: load the FROZEN P2 kernel.o through real CANN ACL, look up
// `main_kernel`, launch it with the frozen ABI/config, and verify the float32
// VADD numerically against the Verifier's independent oracle.
//
// Gate structure (docs/p3-goal-1016.md sections 5-7):
//   --stage=A : loader + lookup only.            P3-A gate.
//   --stage=B : A, plus allocate/launch/sync.    P3-B gate.
//   --stage=C : A+B, plus full numerical check.  P3-C gate.
//
// Hard constraints this file is built to satisfy:
//   CR-ID-2  the file is opened ONCE; the SAME in-memory buffer is hashed and
//            handed to aclrtBinaryLoadFromData.
//   CR-ID-3  any identity mismatch stops BEFORE aclInit.
//   CR-LCH-2 launch carries DYN_UBUF_SIZE=196608 and nothing else.
//   CR-MEM-1 C is initialized with the literal bytes 45 23 c1 7f.
//   CR-X-4   C is never read before a successful stream sync.
//   CR-X-5   on a failed/timed-out sync, nothing is freed, unloaded or reset;
//            evidence is flushed and the process exits under the watchdog with
//            cleanup_incomplete recorded.
//   CR-X-11  the reference is READ FROM THE ORACLE'S FILES, never computed here.
//
// Every numeric constant comes from p3_frozen_constants.h, which is generated
// by p3/gen_frozen_constants.py directly from the frozen acceptance criteria.
// Nothing is hand-transcribed.

#include <cinttypes>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#include "acl/acl.h"
// ACL_ERROR_RT_* codes come from error_codes/rt_error_codes.h, which acl.h pulls
// in transitively via acl_base.h. Included explicitly so the classification
// below does not depend on that transitive path.
#include "acl/error_codes/rt_error_codes.h"

#include "acl_log.h"
#include "p3_frozen_constants.h"
#include "sha256.h"

// ---------------------------------------------------------------- ABI (CR-ABI-*)

struct HostArgs {
  void* a;
  void* b;
  void* c;
};
static_assert(sizeof(void*) == 8, "P3 requires a 64-bit host ABI");
static_assert(sizeof(HostArgs) == 24, "CR-ABI-2: sizeof(HostArgs) must be 24");
static_assert(offsetof(HostArgs, a) == 0, "CR-ABI-2: offsetof(A) must be 0");
static_assert(offsetof(HostArgs, b) == 8, "CR-ABI-2: offsetof(B) must be 8");
static_assert(offsetof(HostArgs, c) == 16, "CR-ABI-2: offsetof(C) must be 16");

namespace p3 {
namespace {

// Exit codes. 0 = success for the requested stage; others are classified.
enum ExitCode {
  kExitOk = 0,
  kExitBinaryIdentityBlocked = 20,
  kExitEnvBlocked = 21,
  kExitLoaderBlocked = 22,
  kExitLaunchBlocked = 23,
  kExitTimeout = 24,
  kExitNumericsFail = 25,
  kExitHarnessError = 26,
  kExitOracleMismatch = 27,
  kExitDeviceFault = 28,
  // The run's numerical comparison completed but a transfer that the comparison
  // DEPENDS ON failed, so its result is not admissible as evidence.
  kExitDataTransferFail = 29,
  // The numerical work completed but resource release did not.
  kExitCleanupFail = 30,
  kExitWatchdogTimeout = 124,
};

bool g_pending_work = false;  // a kernel may still be executing
std::string g_stage;
AclLog g_log;
std::string g_report_path;

// --------------------------------------------------------------- JSON helpers

std::string JsonEscape(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          char buf[8];
          std::snprintf(buf, sizeof(buf), "\\u%04x", c);
          out += buf;
        } else {
          out += c;
        }
    }
  }
  return out;
}

std::string HexPtr(const void* p) {
  char buf[32];
  std::snprintf(buf, sizeof(buf), "0x%" PRIxPTR,
                reinterpret_cast<uintptr_t>(p));
  return std::string(buf);
}

// Appends to the report file. The report is written incrementally so that a
// watchdog kill still leaves every measured fact on disk.
void ReportAppend(const std::string& text) {
  FILE* fp = std::fopen(g_report_path.c_str(), "a");
  if (fp == nullptr) return;
  std::fwrite(text.data(), 1, text.size(), fp);
  std::fclose(fp);
}

void ReportLine(const std::string& text) { ReportAppend(text + "\n"); }

void JsonEvent(const char* key, const std::string& value) {
  ReportLine("{\"event\":\"" + std::string(key) + "\",\"value\":\"" +
             JsonEscape(value) + "\"}");
}

void JsonEventInt(const char* key, long long value) {
  ReportLine("{\"event\":\"" + std::string(key) + "\",\"value\":" +
             std::to_string(value) + "}");
}

// Names an ACL error code so the evidence record is not just a number.
std::string AclErrorName(aclError rc) {
  switch (rc) {
    case ACL_ERROR_RT_AICORE_EXCEPTION: return "ACL_ERROR_RT_AICORE_EXCEPTION";
    case ACL_ERROR_RT_AICORE_TIMEOUT: return "ACL_ERROR_RT_AICORE_TIMEOUT";
    case ACL_ERROR_RT_AICORE_TRAP_EXCEPTION:
      return "ACL_ERROR_RT_AICORE_TRAP_EXCEPTION";
    case ACL_ERROR_RT_AICPU_EXCEPTION: return "ACL_ERROR_RT_AICPU_EXCEPTION";
    case ACL_ERROR_RT_AICPU_TIMEOUT: return "ACL_ERROR_RT_AICPU_TIMEOUT";
    case ACL_ERROR_RT_STREAM_SYNC_TIMEOUT: return "ACL_ERROR_RT_STREAM_SYNC_TIMEOUT";
    case ACL_ERROR_RT_SYS_DMA: return "ACL_ERROR_RT_SYS_DMA";
    case ACL_ERROR_RT_TASK_TIMEOUT: return "ACL_ERROR_RT_TASK_TIMEOUT";
    default: return "UNMAPPED_ACL_ERROR";
  }
}

// True for codes that mean the DEVICE raised a fault while executing the kernel,
// as opposed to the stream simply not finishing in time. Conflating these would
// report a hardware/execution defect as TIMEOUT (which the criteria forbid).
bool IsDeviceFault(aclError rc) {
  return rc == ACL_ERROR_RT_AICORE_EXCEPTION ||
         rc == ACL_ERROR_RT_AICORE_TIMEOUT ||
         rc == ACL_ERROR_RT_AICORE_TRAP_EXCEPTION ||
         rc == ACL_ERROR_RT_AICPU_EXCEPTION ||
         rc == ACL_ERROR_RT_AICPU_TIMEOUT ||
         rc == ACL_ERROR_RT_SYS_DMA;
}

// ------------------------------------------------------------ watchdog

// Hard wall-clock bound (CR: outer process watchdog, default 120 s, expiry =>
// TIMEOUT). In-process so that a wedged ACL call is still bounded. It writes
// evidence and calls _exit, which deliberately does NOT run destructors: on the
// pending-work path nothing may be freed, unloaded or reset.
void WatchdogThread(unsigned seconds) {
  for (unsigned i = 0; i < seconds * 10; ++i) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 100 * 1000 * 1000;  // 100 ms
    nanosleep(&ts, nullptr);
  }
  g_log.Note("WATCHDOG_EXPIRED",
             "wall-clock budget " + std::to_string(seconds) +
                 "s exceeded; exiting without cleanup");
  JsonEvent("timeout", "watchdog expired after " + std::to_string(seconds) + "s");
  JsonEvent("classification", "TIMEOUT");
  JsonEvent("cleanup_incomplete",
            g_pending_work ? "true (kernel may still be executing)"
                           : "true (watchdog kill, unwinding suppressed)");
  ReportLine("{\"stage\":\"" + g_stage + "\",\"result\":\"TIMEOUT\"}");
  g_log.Flush();
  std::fflush(nullptr);
  _exit(kExitWatchdogTimeout);
}

// --------------------------------------------------------------- file helpers

bool ReadWholeFile(const std::string& path, std::vector<uint8_t>* out,
                   std::string* err) {
  const int fd = ::open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    *err = "open failed: " + path;
    return false;
  }
  std::vector<uint8_t> buf;
  uint8_t chunk[1 << 16];
  for (;;) {
    const ssize_t n = ::read(fd, chunk, sizeof(chunk));
    if (n < 0) {
      *err = "read failed: " + path;
      ::close(fd);
      return false;
    }
    if (n == 0) break;
    buf.insert(buf.end(), chunk, chunk + n);
  }
  ::close(fd);
  *out = std::move(buf);
  return true;
}

// --------------------------------------------------------- frozen identities

const PayloadIdentity* FindIdentity(const std::string& case_name,
                                    const std::string& slug) {
  for (size_t i = 0; i < kNumPayloadIdentities; ++i) {
    if (case_name == kPayloadIdentities[i].case_name &&
        slug == kPayloadIdentities[i].slug) {
      return &kPayloadIdentities[i];
    }
  }
  return nullptr;
}

// Loads an oracle payload and binds it to the frozen CR-DATA-7 digest. A
// mismatch is an oracle/formula discrepancy (CR-DATA-7) and blocks P3-C.
bool LoadOraclePayload(const std::string& dir, const std::string& case_name,
                       const std::string& slug, std::vector<uint8_t>* out,
                       std::string* err) {
  const std::string path = dir + "/" + case_name + "." + slug + ".bin";
  if (!ReadWholeFile(path, out, err)) return false;
  const PayloadIdentity* id = FindIdentity(case_name, slug);
  if (id == nullptr) {
    *err = "no frozen identity row for " + case_name + "/" + slug;
    return false;
  }
  if (out->size() != id->bytes) {
    *err = path + ": size " + std::to_string(out->size()) + " != frozen " +
           std::to_string(id->bytes);
    return false;
  }
  const std::string got = Sha256::Hex(out->data(), out->size());
  if (got != id->sha256) {
    *err = path + ": sha256 " + got + " != frozen " + id->sha256;
    return false;
  }
  return true;
}

// ------------------------------------------------------------ device / alloc

struct GuardedBuffer {
  void* dev_raw = nullptr;      // pointer returned by aclrtMalloc
  void* dev_base = nullptr;     // 512-aligned region start (>= dev_raw)
  void* dev_payload = nullptr;  // dev_base + kGuardBytes
  size_t alloc_bytes = 0;       // requested size (kAllocBytes + 512)
  std::vector<uint8_t> host;    // host mirror, same layout as dev_base
};

uint8_t* HostPayload(GuardedBuffer* b) {
  return b->host.data() + kGuardBytes;
}

void FillGuards(GuardedBuffer* b) {
  std::memset(b->host.data(), kGuardByte, kGuardBytes);
  std::memset(b->host.data() + kGuardBytes + kPayloadBytes, kGuardByte,
              kGuardBytes);
}

// ------------------------------------------------------------------ results

struct BoundaryEntry {
  size_t idx;
  uint32_t dev_bits;
  uint32_t ref_bits;
  bool match;
};

struct RunResult {
  std::string case_name;
  int run_index = 0;
  aclError launch_rc = -1;
  aclError sync_rc = -1;
  aclError h2d_rc = -1;
  aclError d2h_rc = -1;
  // Per-array D2H rcs. The comparison reads C for the result and A/B for the
  // preservation check, so ALL THREE must be recorded and all three must be
  // zero for any of those readings to be evidence.
  aclError d2h_rc_a = -1;
  aclError d2h_rc_b = -1;
  aclError d2h_rc_c = -1;
  size_t mismatches = 0;
  size_t sentinel_residue = 0;
  size_t nan_or_inf = 0;
  bool a_unmodified = false;
  bool b_unmodified = false;
  bool guards_ok = false;
  double max_abs_err = 0.0;
  std::vector<size_t> block_mismatch;  // size kGrid
  std::vector<std::pair<size_t, uint32_t>> first_mismatches;  // idx, device bits
  std::vector<BoundaryEntry> boundary;
  size_t compared_elements = 0;  // 0 means "no comparison was performed"
  // Set by CompareAll from the transfer verdict: whether the comparison's
  // readings describe device bytes, and whether the preservation/guard checks
  // were actually measured. Distinct from `compared_elements`, which says
  // whether a comparison was attempted at all.
  bool comparison_attempted = false;
  bool comparison_admissible = false;
  bool preservation_measured = false;
  bool guards_measured = false;
  std::string transfer_failure_reason;
  bool executed = false;
  bool passed = false;
};

// ------------------------------------------------------- admissibility rules
//
// These are the harness's two "may I believe this?" predicates. They are PURE
// functions of rc values and are the SINGLE source of truth used both by the
// real run path and by the host-only fault-injection self-test below. The point
// of the self-test is only meaningful if the injected failure travels through
// the same predicate the device path uses; a copied predicate would prove
// nothing about the device path.

// CR-MEM-4 / CR-X-4: a run's numerical observations are admissible only if the
// kernel finished (sync rc 0) and EVERY transfer the comparison reads succeeded.
// The A/B host mirrors already contain the input bytes written before H2D, so if
// an A/B D2H fails and leaves the destination untouched, memcmp against those
// mirrors would report "unmodified" for bytes that were never read back from the
// device. That is a false PASS, so a failed A/B (or C) D2H must make the whole
// run inadmissible rather than merely be logged.
struct TransferVerdict {
  bool admissible = false;      // comparison may be believed
  bool preservation_measured = false;  // a_unmodified/b_unmodified are evidence
  bool guards_measured = false;        // guards_ok is evidence
  std::string reason;                  // first failing transfer, empty if none
};

inline TransferVerdict JudgeTransfers(aclError d2h_a, aclError d2h_b,
                                      aclError d2h_c) {
  TransferVerdict v;
  const char* bad = nullptr;
  aclError rc = ACL_SUCCESS;
  if (d2h_c != ACL_SUCCESS) { bad = "C"; rc = d2h_c; }
  else if (d2h_a != ACL_SUCCESS) { bad = "A"; rc = d2h_a; }
  else if (d2h_b != ACL_SUCCESS) { bad = "B"; rc = d2h_b; }
  if (bad != nullptr) {
    v.admissible = false;
    // A/B feed the preservation check, C feeds the guards and the numerics.
    v.preservation_measured = (d2h_a == ACL_SUCCESS) && (d2h_b == ACL_SUCCESS);
    v.guards_measured = (d2h_c == ACL_SUCCESS);
    v.reason = std::string("D2H of array ") + bad + " failed rc=" +
               std::to_string(rc);
    return v;
  }
  v.admissible = true;
  v.preservation_measured = true;
  v.guards_measured = true;
  return v;
}

// CR: cleanup failures must prohibit the stage PASS while PRESERVING the first
// error. The numerical observations still stand and are still reported; what
// fails is the stage verdict.
struct CleanupVerdict {
  bool ok = false;
  std::string first_failure;  // empty if none
};

inline CleanupVerdict JudgeCleanup(const std::vector<std::pair<const char*, aclError>>& ops) {
  CleanupVerdict v;
  for (const auto& op : ops) {
    if (op.second != ACL_SUCCESS) {
      v.ok = false;
      v.first_failure = std::string(op.first) + " rc=" + std::to_string(op.second);
      return v;
    }
  }
  v.ok = true;
  return v;
}

// The stage gate itself. The frozen criteria put resource release INSIDE every
// stage gate (P3-A "resource release explainable"; P3-B "all resource lifetimes
// controlled"), so the gate is the AND of the stage's work half and the release
// half -- and it is not computable until the release has been judged.
//
// This is why the gate row is emitted AFTER run_cleanup() and why
// `cleanup_judged` exists: a gate row built before the release has no second
// operand, and an equality whose second operand is absent must not read as
// PASS. An unjudged release yields NOT_EVALUATED, never PASS.
struct StageGateVerdict {
  bool cleanup_judged = false;  // a release verdict exists
  bool work_ok = false;         // the launch/sync (B) or numerics (C) half
  bool cleanup_ok = false;      // the release half
  bool pass = false;            // the stage gate: work AND release
  std::string first_failure;    // release's first failure, empty if none
};

inline StageGateVerdict JudgeStageGate(bool work_ok, const CleanupVerdict& cv) {
  StageGateVerdict v;
  v.cleanup_judged = true;
  v.work_ok = work_ok;
  v.cleanup_ok = cv.ok;
  v.first_failure = cv.first_failure;
  v.pass = work_ok && cv.ok;
  return v;
}

// The final exit classification for a stage-C run whose release succeeded.
//
// PRECEDENCE RULE, stated here because it is a decision and not an observation:
//   DATA_TRANSFER_FAIL outranks NUMERICS_FAIL.
// A failed D2H means the readings the comparison would use describe host state,
// not device state. Such a run cannot support the claim "the NUMBERS were
// wrong": the more fundamental failure is that no admissible measurement was
// taken. Returning NUMERICS_FAIL there would assert a numerical result the
// harness explicitly refused to compute, which is the same false claim the
// admissibility gate exists to prevent -- it would merely move it from the
// verdict field to the exit code.
//
// In this harness at most one classification is ever recorded per process (the
// run loop breaks on the first failure and the outer loop's guard then exits),
// so `classified_failure` carries the single recorded failure. The rule is
// written as a precedence anyway, so that it stays correct if that ever changes
// and so that it can be exercised by the self-test.
inline int FinalClassify(bool work_ok, int classified_failure) {
  if (work_ok) return kExitOk;
  if (classified_failure == kExitDataTransferFail) return kExitDataTransferFail;
  return kExitNumericsFail;
}

// Builds the stage gate row. `extra_fields` carries the gate-specific middle
// (launch/sync rcs and grid for B, run counts for C). Split from the write so
// the fault-injection self-test can assert on the TEXT this would emit.
inline std::string BuildGateJson(const char* gate, const StageGateVerdict& gv,
                                 const std::string& extra_fields) {
  auto tri = [](bool measured, bool value) -> std::string {
    if (!measured) return "null";
    return value ? "true" : "false";
  };
  std::string s = "{\"gate\":\"" + std::string(gate) + "\",\"result\":";
  if (!gv.cleanup_judged) {
    // No release verdict: the gate has not been decided. Emitting PASS here is
    // exactly the false-PASS path this function exists to make unrepresentable.
    s += "\"NOT_EVALUATED\"";
  } else {
    s += std::string(gv.pass ? "\"PASS\"" : "\"FAIL\"");
  }
  s += ",\"work_ok\":" + tri(gv.cleanup_judged, gv.work_ok);
  s += ",\"cleanup_ok\":" + tri(gv.cleanup_judged, gv.cleanup_ok);
  s += ",\"cleanup_first_failure\":";
  if (!gv.cleanup_judged || gv.first_failure.empty()) {
    s += "null";
  } else {
    s += "\"" + JsonEscape(gv.first_failure) + "\"";
  }
  if (!extra_fields.empty()) s += "," + extra_fields;
  s += "}";
  return s;
}

// ------------------------------------------- host-only fault-injection self-test
//
// Reproduces, WITHOUT a device, the two false-PASS paths the Challenger named,
// by driving the SAME predicates the device path drives. A self-test that used
// a copy of the logic would prove nothing about the device path, so this calls
// JudgeTransfers / JudgeCleanup / CompareAll directly.

// Defined below; the self-test drives the real comparison, not a copy of it.
bool CompareAll(const GuardedBuffer& cbuf, const std::vector<uint8_t>& ref,
                const GuardedBuffer& abuf, const GuardedBuffer& bbuf,
                const std::vector<uint8_t>& a_sent,
                const std::vector<uint8_t>& b_sent, const TransferVerdict& tv,
                RunResult* r);

// Also defined below: the exact text the device path writes for a run.
std::string BuildRunJson(const RunResult& r);
//
// Injection 1: A's D2H returns failure and leaves its destination unchanged. In
// the device path the A mirror then still holds the input bytes written before
// H2D, so the preservation memcmp would compare those bytes against themselves
// and report "unmodified" for data never read back from the device. Required
// outcome: the run is inadmissible, the preservation check is UNEVALUATED, and
// no numerical PASS is reachable.
//
// Injection 2: every numerical observation is correct but a cleanup op returns
// failure. Required outcome: the stage verdict fails while the first error is
// preserved verbatim.
bool RunFaultInjectionSelfTest(std::string* detail) {
  std::vector<std::string> lines;

  // ---- injection 1: failed A D2H, destination unchanged --------------------
  {
    const aclError kInjectedFail = ACL_ERROR_INVALID_PARAM;
    // A and B mirrors hold exactly the bytes that were written before H2D, which
    // is what "a failed D2H left the destination unchanged" looks like.
    GuardedBuffer A, B, C;
    A.host.assign(kAllocBytes, 0);
    B.host.assign(kAllocBytes, 0);
    C.host.assign(kAllocBytes, 0);
    std::vector<uint8_t> a_sent(kPayloadBytes, 0x11);
    std::vector<uint8_t> b_sent(kPayloadBytes, 0x22);
    std::vector<uint8_t> c_ref(kPayloadBytes, 0x33);
    // The device "would have" produced c_ref; the C D2H succeeded.
    std::memcpy(C.host.data() + kGuardBytes, c_ref.data(), kPayloadBytes);
    std::memcpy(A.host.data() + kGuardBytes, a_sent.data(), kPayloadBytes);
    std::memcpy(B.host.data() + kGuardBytes, b_sent.data(), kPayloadBytes);
    FillGuards(&A);
    FillGuards(&B);
    FillGuards(&C);

    const TransferVerdict tv = JudgeTransfers(kInjectedFail, ACL_SUCCESS, ACL_SUCCESS);
    if (tv.admissible) {
      lines.push_back("INJ1 FAIL: a failed A D2H was judged admissible");
    } else if (tv.preservation_measured) {
      lines.push_back("INJ1 FAIL: preservation check still claimed as measured");
    } else {
      lines.push_back("INJ1 OK: A D2H failure => inadmissible, preservation "
                      "unevaluated, reason=\"" + tv.reason + "\"");
    }

    // Drive the real comparison with that verdict. Note the A mirror is
    // byte-identical to a_sent, so the OLD code would have reported
    // a_unmodified=true from bytes that were never read back.
    RunResult r;
    CompareAll(C, c_ref, A, B, a_sent, b_sent, tv, &r);
    if (r.passed) {
      lines.push_back("INJ1 FAIL: comparison reported PASS on an inadmissible run");
    } else if (r.compared_elements != 0) {
      lines.push_back("INJ1 FAIL: comparison claimed " +
                      std::to_string(r.compared_elements) + " compared elements");
    } else if (!r.preservation_measured) {
      lines.push_back("INJ1 OK: no numerical PASS reachable; compared_elements=0, "
                      "preservation_measured=false, verdict would be FAIL");
    } else {
      lines.push_back("INJ1 FAIL: preservation still flagged measured");
    }
    // The specific lie this guard exists to stop: the mirror matches a_sent, so
    // the raw memcmp result is "true". Assert the guard suppresses it.
    const bool raw_memcmp_would_say_true =
        (std::memcmp(A.host.data() + kGuardBytes, a_sent.data(), kPayloadBytes) == 0);
    if (!raw_memcmp_would_say_true) {
      lines.push_back("INJ1 INVALID: the injection did not reproduce the "
                      "unchanged-destination condition");
    } else {
      lines.push_back("INJ1 OK: reproduced the trap -- raw memcmp WOULD say "
                      "unmodified=true, and the verdict suppresses it");
    }

    // The record is what a reader believes. Assert the emitted TEXT carries no
    // numerical claim: no PASS, no measured-looking numbers.
    const std::string js = BuildRunJson(r);
    struct Probe { const char* needle; const char* what; };
    const Probe must_not_appear[] = {
        {"\"verdict\":\"PASS\"", "a PASS verdict"},
        {"\"comparison_performed\":true", "comparison_performed true"},
        {"\"mismatches\":0", "a zero mismatch count"},
        {"\"a_unmodified\":true", "a true A-preservation claim"},
        {"\"guards_ok\":true", "a true guard claim"},
        {"\"compared_elements\":524288", "a full comparison count"},
    };
    for (const Probe& p : must_not_appear) {
      if (js.find(p.needle) != std::string::npos) {
        lines.push_back(std::string("INJ3 FAIL: the record still claims ") +
                        p.what + " on an inadmissible run");
      }
    }
    if (js.find("\"verdict\":\"FAIL\"") == std::string::npos) {
      lines.push_back("INJ3 FAIL: the record does not say verdict FAIL");
    } else if (js.find("\"comparison_admissible\":false") == std::string::npos) {
      lines.push_back("INJ3 FAIL: the record does not say comparison_admissible false");
    } else if (js.find("\"preservation_measured\":null") == std::string::npos) {
      lines.push_back("INJ3 FAIL: the record does not mark preservation unmeasured");
    } else {
      lines.push_back("INJ3 OK: the record says FAIL / inadmissible / unmeasured "
                      "and carries no numerical claim");
    }
  }

  // ---- injection 2: cleanup failure after correct numerics -----------------
  {
    const aclError kInjectedFail = ACL_ERROR_INVALID_PARAM;
    // First failure preserved: an earlier op fails too, and a LATER one must not
    // replace it.
    std::vector<std::pair<const char*, aclError>> ops = {
        {"aclrtFree(A)", ACL_SUCCESS},
        {"aclrtFree(B)", ACL_SUCCESS},
        {"aclrtFree(C)", kInjectedFail},
        {"aclrtBinaryUnLoad", kInjectedFail},
        {"aclFinalize", ACL_SUCCESS},
    };
    const CleanupVerdict cv = JudgeCleanup(ops);
    if (cv.ok) {
      lines.push_back("INJ2 FAIL: a failing cleanup was judged ok");
    } else if (cv.first_failure.find("aclrtFree(C)") == std::string::npos) {
      lines.push_back("INJ2 FAIL: first error not preserved, got \"" +
                      cv.first_failure + "\"");
    } else {
      lines.push_back("INJ2 OK: cleanup failure => stage FAIL, first error "
                      "preserved verbatim \"" + cv.first_failure + "\"");
    }
    // The all-clean case must still pass, otherwise the guard is a constant.
    const CleanupVerdict ok_cv = JudgeCleanup({{"aclrtFree(A)", ACL_SUCCESS},
                                               {"aclFinalize", ACL_SUCCESS}});
    if (!ok_cv.ok) {
      lines.push_back("INJ2 FAIL: clean cleanup was judged failed");
    } else {
      lines.push_back("INJ2 OK: clean cleanup still judged ok (guard is not a "
                      "constant)");
    }
    // And the stage verdict must be the AND of numerics and cleanup.
    const bool numerics_pass = true;
    if (numerics_pass && cv.ok) {
      lines.push_back("INJ2 FAIL: stage PASS on a failed cleanup");
    } else {
      lines.push_back("INJ2 OK: stage verdict = numerics AND cleanup = FAIL");
    }
  }

  // ---- injection 4: the stage-B record claims no measurement ----------------
  // The stage-B path (launch/sync only) performs no comparison. Its record is
  // what a P3-B PASS emits, so it must carry no numerical claim at all.
  {
    RunResult r;
    r.case_name = "stage_b_probe";
    r.executed = true;
    r.passed = true;  // the launch/sync gate passed
    const std::string js = BuildRunJson(r);
    if (js.find("\"verdict\":\"NOT_EVALUATED\"") == std::string::npos) {
      lines.push_back("INJ4 FAIL: a not-attempted run does not say NOT_EVALUATED");
    } else if (js.find("\"comparison_attempted\":false") == std::string::npos) {
      lines.push_back("INJ4 FAIL: comparison_attempted not false");
    } else if (js.find("\"verdict\":\"PASS\"") != std::string::npos) {
      lines.push_back("INJ4 FAIL: a not-attempted run claims a numerical PASS");
    } else if (js.find("\"mismatches\":0") != std::string::npos ||
               js.find("\"guards_ok\":true") != std::string::npos ||
               js.find("\"compared_elements\":524288") != std::string::npos) {
      lines.push_back("INJ4 FAIL: a not-attempted run carries measured-looking "
                      "numbers");
    } else if (js.find("\"block_mismatch\":[]") != std::string::npos) {
      lines.push_back("INJ4 FAIL: empty array instead of null (reads as "
                      "'measured, nothing found')");
    } else {
      lines.push_back("INJ4 OK: stage-B record says NOT_EVALUATED with every "
                      "comparison field null and no numerical claim");
    }
  }

  // ---- injection 5: a cleanup failure must reach the GATE row --------------
  // Injections 1-4 prove the predicates and the run record. None of them proves
  // the ORDER in which the device path writes its rows. The frozen criteria put
  // resource release inside every stage gate (P3-B: "all resource lifetimes
  // controlled"), so a gate row written BEFORE the release is judged can say
  // PASS on a run whose release then failed -- the PASS is already on disk and
  // nothing rewrites it. Required outcome: the gate row and the release verdict
  // are derived from ONE verdict, so a failed release makes the gate row FAIL
  // while the numerical work half stays PASS and the first error is verbatim.
  {
    const aclError kInjectedFail = ACL_ERROR_INVALID_PARAM;
    const CleanupVerdict bad = JudgeCleanup({{"aclrtFree(A)", ACL_SUCCESS},
                                             {"aclrtFree(C)", kInjectedFail},
                                             {"aclFinalize", kInjectedFail}});
    // The work half is clean here: launch/sync succeeded. Only the release failed.
    const StageGateVerdict gv = JudgeStageGate(/*work_ok=*/true, bad);
    const std::string js = BuildGateJson("P3-B", gv,
                                         "\"launch_rc\":0,\"sync_rc\":0,"
                                         "\"grid\":64,\"dyn_ub_bytes\":196608");
    if (js.find("\"result\":\"PASS\"") != std::string::npos) {
      lines.push_back("INJ5 FAIL: the gate row says PASS on a failed release: " + js);
    } else if (js.find("\"result\":\"FAIL\"") == std::string::npos) {
      lines.push_back("INJ5 FAIL: the gate row does not say FAIL: " + js);
    } else if (js.find("\"work_ok\":true") == std::string::npos) {
      lines.push_back("INJ5 FAIL: the work half is not preserved as PASS: " + js);
    } else if (js.find("aclrtFree(C) rc=") == std::string::npos) {
      lines.push_back("INJ5 FAIL: the release's first error is not in the gate row: " +
                      js);
    } else {
      lines.push_back("INJ5 OK: a failed release makes the GATE row FAIL while "
                      "the work half stays PASS, first error verbatim in the row");
    }
    // The all-clean case must still say PASS, otherwise the gate is a constant.
    const StageGateVerdict ok_gv =
        JudgeStageGate(/*work_ok=*/true, JudgeCleanup({{"aclFinalize", ACL_SUCCESS}}));
    const std::string ok_js = BuildGateJson("P3-B", ok_gv, "");
    if (ok_js.find("\"result\":\"PASS\"") == std::string::npos) {
      lines.push_back("INJ5 FAIL: a clean work half plus a clean release did not "
                      "give PASS: " + ok_js);
    } else {
      lines.push_back("INJ5 OK: clean work + clean release still gives PASS "
                      "(the gate is not a constant)");
    }
    // And the unjudged case must be NOT_EVALUATED, never PASS: this is the shape
    // of the defect being fixed (a gate row built before the release exists).
    StageGateVerdict unjudged;
    const std::string uj = BuildGateJson("P3-B", unjudged, "\"launch_rc\":0");
    if (uj.find("\"result\":\"NOT_EVALUATED\"") == std::string::npos) {
      lines.push_back("INJ5 FAIL: a gate row with no release verdict is not "
                      "NOT_EVALUATED: " + uj);
    } else if (uj.find("\"result\":\"PASS\"") != std::string::npos) {
      lines.push_back("INJ5 FAIL: a gate row with no release verdict says PASS");
    } else if (uj.find("\"cleanup_ok\":null") == std::string::npos) {
      lines.push_back("INJ5 FAIL: an unjudged release is not reported as null: " + uj);
    } else {
      lines.push_back("INJ5 OK: a gate row written before the release is judged "
                      "says NOT_EVALUATED with cleanup_ok null, never PASS");
    }
  }

  // ---- injections 6/7: the ADMISSIBLE states, so the guards are not constants
  // Injections 1-5 all drive the INADMISSIBLE or not-attempted paths. A guard
  // that rejects everything would satisfy every one of them and still be
  // useless, so the two admissible states must be exercised too: a clean run
  // (must reach PASS with real measured values) and a run with one wrong
  // element (must reach FAIL while still carrying its real readings -- the
  // third state, "attempted and admissible and failing", is distinct from
  // "not evaluated" and must not collapse into it).
  {
    // Builds a run whose three mirrors hold plausible device bytes and whose
    // transfers all succeeded, then perturbs C at `bad_idx` (-1 = no error).
    auto build = [](long bad_idx, RunResult* r) -> GuardedBuffer {
      GuardedBuffer A, B, C;
      A.host.assign(kAllocBytes, 0);
      B.host.assign(kAllocBytes, 0);
      C.host.assign(kAllocBytes, 0);
      std::vector<uint8_t> a_sent(kPayloadBytes, 0x11);
      std::vector<uint8_t> b_sent(kPayloadBytes, 0x22);
      std::vector<uint8_t> c_ref(kPayloadBytes, 0x33);
      std::memcpy(A.host.data() + kGuardBytes, a_sent.data(), kPayloadBytes);
      std::memcpy(B.host.data() + kGuardBytes, b_sent.data(), kPayloadBytes);
      std::memcpy(C.host.data() + kGuardBytes, c_ref.data(), kPayloadBytes);
      if (bad_idx >= 0) {
        // One wrong 4-byte word, at a boundary point so the per-block report has
        // something real to say.
        const size_t off = kGuardBytes + static_cast<size_t>(bad_idx) * 4;
        C.host[off] = 0x44;
      }
      FillGuards(&A);
      FillGuards(&B);
      FillGuards(&C);
      const TransferVerdict tv = JudgeTransfers(ACL_SUCCESS, ACL_SUCCESS, ACL_SUCCESS);
      // Drives the REAL comparison, not a copy of it.
      CompareAll(C, c_ref, A, B, a_sent, b_sent, tv, r);
      return C;
    };

    // ---- injection 6: admissible and clean => the record says PASS ---------
    {
      RunResult r;
      build(/*bad_idx=*/-1, &r);
      const std::string js = BuildRunJson(r);
      if (!r.passed) {
        lines.push_back("INJ6 FAIL: a clean admissible run was judged not passed");
      } else if (js.find("\"verdict\":\"PASS\"") == std::string::npos) {
        lines.push_back("INJ6 FAIL: a clean admissible run does not say PASS: " + js);
      } else if (js.find("\"comparison_performed\":true") == std::string::npos ||
                 js.find("\"compared_elements\":524288") == std::string::npos ||
                 js.find("\"mismatches\":0") == std::string::npos ||
                 js.find("\"a_unmodified\":true") == std::string::npos ||
                 js.find("\"b_unmodified\":true") == std::string::npos ||
                 js.find("\"guards_ok\":true") == std::string::npos ||
                 js.find("\"sentinel_residue\":0") == std::string::npos) {
        lines.push_back("INJ6 FAIL: a clean admissible run does not carry its real "
                        "measured values: " + js);
      } else if (js.find("\"verdict\":\"NOT_EVALUATED\"") != std::string::npos) {
        lines.push_back("INJ6 FAIL: a clean admissible run was reported as "
                        "not evaluated");
      } else {
        lines.push_back("INJ6 OK: a clean admissible run reaches PASS and carries "
                        "real measured values (the verdict line is not a constant)");
      }
    }

    // ---- injection 7: admissible but one wrong element => FAIL with readings -
    {
      RunResult r;
      build(/*bad_idx=*/8191, &r);
      const std::string js = BuildRunJson(r);
      if (r.passed) {
        lines.push_back("INJ7 FAIL: a run with a wrong element was judged passed");
      } else if (r.mismatches != 1) {
        lines.push_back("INJ7 FAIL: expected exactly 1 mismatch, got " +
                        std::to_string(r.mismatches));
      } else if (js.find("\"verdict\":\"FAIL\"") == std::string::npos) {
        lines.push_back("INJ7 FAIL: a failing admissible run does not say FAIL: " + js);
      } else if (js.find("\"mismatches\":1") == std::string::npos ||
                 js.find("\"compared_elements\":524288") == std::string::npos) {
        lines.push_back("INJ7 FAIL: a failing admissible run does not carry its "
                        "real readings: " + js);
      } else if (js.find("\"block_mismatch\":null") != std::string::npos) {
        lines.push_back("INJ7 FAIL: a failing admissible run reported null block "
                        "data (that is the not-evaluated state)");
      } else {
        lines.push_back("INJ7 OK: an admissible run with one wrong element FAILS "
                        "while still carrying its real readings -- the third state "
                        "does not collapse into not-evaluated");
      }
    }
  }

  // ---- injection 8: the gate must exercise FAILED work, not only failed release
  // Every gate case above passes work_ok=true, so a mutation that drops the work
  // operand (`v.pass = cv.ok`) would survive all of them by inspection. That
  // mutation is not cosmetic: it makes a C gate PASS after NUMERICAL FAILURE as
  // long as the release was clean. Required outcome: work_ok=false fails the gate
  // whether or not the release was clean, and work_ok is preserved in the row.
  {
    const CleanupVerdict clean = JudgeCleanup({{"aclFinalize", ACL_SUCCESS}});
    const CleanupVerdict dirty =
        JudgeCleanup({{"aclrtFree(C)", ACL_ERROR_INVALID_PARAM}});

    const StageGateVerdict wf_cok = JudgeStageGate(/*work_ok=*/false, clean);
    const std::string js = BuildGateJson("P3-C", wf_cok,
                                         "\"runs_executed\":0,\"runs_required\":15");
    if (js.find("\"result\":\"PASS\"") != std::string::npos) {
      lines.push_back("INJ8 FAIL: work_ok=false with a clean release gave PASS "
                      "(the work operand is not in the conjunction): " + js);
    } else if (js.find("\"result\":\"FAIL\"") == std::string::npos) {
      lines.push_back("INJ8 FAIL: work_ok=false with a clean release is not FAIL: " + js);
    } else if (js.find("\"work_ok\":false") == std::string::npos) {
      lines.push_back("INJ8 FAIL: work_ok is not preserved as false: " + js);
    } else if (js.find("\"cleanup_ok\":true") == std::string::npos) {
      lines.push_back("INJ8 FAIL: the clean release is not reported as ok: " + js);
    } else {
      lines.push_back("INJ8 OK: work_ok=false + clean release => gate FAIL, with "
                      "work_ok false and cleanup_ok true both preserved");
    }

    const StageGateVerdict wf_cbad = JudgeStageGate(/*work_ok=*/false, dirty);
    const std::string js2 = BuildGateJson("P3-C", wf_cbad, "");
    if (js2.find("\"result\":\"FAIL\"") == std::string::npos) {
      lines.push_back("INJ8 FAIL: work_ok=false + failed release is not FAIL: " + js2);
    } else if (js2.find("\"work_ok\":false") == std::string::npos ||
               js2.find("\"cleanup_ok\":false") == std::string::npos) {
      lines.push_back("INJ8 FAIL: both false halves are not preserved: " + js2);
    } else {
      lines.push_back("INJ8 OK: work_ok=false + failed release => gate FAIL, both "
                      "halves false");
    }

    // The gate must still be satisfiable, or it is a constant that rejects all.
    const StageGateVerdict tt = JudgeStageGate(/*work_ok=*/true, clean);
    if (!tt.pass) {
      lines.push_back("INJ8 FAIL: work_ok=true + clean release did not pass");
    } else {
      lines.push_back("INJ8 OK: work_ok=true + clean release still passes (the "
                      "gate is not a constant)");
    }
  }

  // ---- injections 9/10: the OTHER per-array D2H failures --------------------
  // Only A-failure (INJ1) and all-success (INJ6) were injected, so the B and C
  // arms of JudgeTransfers had no injection behind them. They are not
  // interchangeable: B feeds the preservation check and C feeds the guards and
  // the numerics, so a B failure and a C failure must each make the run
  // inadmissible while reporting the OTHER check as still measured.
  {
    auto report = [&](const char* tag, const TransferVerdict& tv,
                      bool want_pres, bool want_guards) {
      if (tv.admissible) {
        lines.push_back(std::string(tag) + " FAIL: an inadmissible transfer set "
                        "was judged admissible");
      } else if (tv.preservation_measured != want_pres) {
        lines.push_back(std::string(tag) + " FAIL: preservation_measured=" +
                        (tv.preservation_measured ? "true" : "false") +
                        ", wanted " + (want_pres ? "true" : "false"));
      } else if (tv.guards_measured != want_guards) {
        lines.push_back(std::string(tag) + " FAIL: guards_measured=" +
                        (tv.guards_measured ? "true" : "false") + ", wanted " +
                        (want_guards ? "true" : "false"));
      } else if (tv.reason.find(tag == std::string("INJ9") ? "array B"
                                                           : "array C") ==
                 std::string::npos) {
        lines.push_back(std::string(tag) + " FAIL: the reason does not name the "
                        "failing array: " + tv.reason);
      } else {
        lines.push_back(std::string(tag) + " OK: " + tv.reason +
                        " => inadmissible, preservation_measured=" +
                        (want_pres ? "true" : "false") + ", guards_measured=" +
                        (want_guards ? "true" : "false"));
      }
    };

    // B-only failure: A and C were read back, so the guard check IS measured;
    // the preservation check is NOT (B was never read back).
    const TransferVerdict b_only = JudgeTransfers(ACL_SUCCESS, ACL_ERROR_INVALID_PARAM,
                                                  ACL_SUCCESS);
    report("INJ9", b_only, /*want_pres=*/false, /*want_guards=*/true);

    // C-only failure: A and B were read back, so preservation IS measured; the
    // guard check and the numerics are NOT.
    const TransferVerdict c_only = JudgeTransfers(ACL_SUCCESS, ACL_SUCCESS,
                                                  ACL_ERROR_INVALID_PARAM);
    report("INJ10", c_only, /*want_pres=*/true, /*want_guards=*/false);

    // And each must produce a record with no numerical claim at all.
    //
    // The buffers are FULLY SIZED and their mirrors hold matching bytes, on
    // purpose. An injection that handed CompareAll empty buffers would only be
    // safe while the admissibility predicate rejects: if that predicate were
    // mutated to accept, CompareAll would read past the empty vector and the
    // process would die of SIGSEGV before reporting anything. A crash is a
    // nonzero exit, so it would still "kill" the mutant, but it would hide WHICH
    // check killed it. With real buffers the mutant instead produces a record
    // that claims a numerical result, and the assertion below names it.
    GuardedBuffer A, B, C;
    A.host.assign(kAllocBytes, 0);
    B.host.assign(kAllocBytes, 0);
    C.host.assign(kAllocBytes, 0);
    std::vector<uint8_t> a_sent(kPayloadBytes, 0x11);
    std::vector<uint8_t> b_sent(kPayloadBytes, 0x22);
    std::vector<uint8_t> c_ref(kPayloadBytes, 0x33);
    std::memcpy(A.host.data() + kGuardBytes, a_sent.data(), kPayloadBytes);
    std::memcpy(B.host.data() + kGuardBytes, b_sent.data(), kPayloadBytes);
    std::memcpy(C.host.data() + kGuardBytes, c_ref.data(), kPayloadBytes);
    FillGuards(&A);
    FillGuards(&B);
    FillGuards(&C);
    for (const std::pair<const char*, TransferVerdict>& pr :
         {std::make_pair("INJ9", b_only), std::make_pair("INJ10", c_only)}) {
      RunResult r;
      CompareAll(C, c_ref, A, B, a_sent, b_sent, pr.second, &r);
      const std::string js = BuildRunJson(r);
      if (js.find("\"verdict\":\"PASS\"") != std::string::npos ||
          js.find("\"mismatches\":0") != std::string::npos ||
          js.find("\"compared_elements\":524288") != std::string::npos) {
        lines.push_back(std::string(pr.first) + " FAIL: the record claims a "
                        "numerical result on an inadmissible run: " + js);
      } else if (js.find("\"verdict\":\"FAIL\"") == std::string::npos ||
                 js.find("\"compared_elements\":null") == std::string::npos) {
        lines.push_back(std::string(pr.first) + " FAIL: the record is not FAIL with "
                        "null readings: " + js);
      } else {
        lines.push_back(std::string(pr.first) + " OK: the record is FAIL with every "
                        "reading null and no numerical claim");
      }
    }

    // The stage-B row shape: launch/sync PASSED, the D2H did not. The B gate is
    // launch/sync + release and is NOT changed, but the row must not read as a
    // clean run -- it has to name the transfer failure and say that no numerical
    // reading was taken.
    {
      RunResult r;
      r.case_name = "stage_b_probe";
      r.executed = true;
      r.passed = true;  // launch and sync succeeded
      r.launch_rc = ACL_SUCCESS;
      r.sync_rc = ACL_SUCCESS;
      r.d2h_rc_a = ACL_SUCCESS;
      r.d2h_rc_b = ACL_ERROR_INVALID_PARAM;  // the B-only failure
      r.d2h_rc_c = ACL_SUCCESS;
      r.d2h_rc = r.d2h_rc_c;
      r.transfer_failure_reason = b_only.reason;
      const std::string js = BuildRunJson(r);
      if (js.find("\"verdict\":\"NOT_EVALUATED\"") == std::string::npos) {
        lines.push_back("INJ9 FAIL: the stage-B row with a failed D2H is not "
                        "NOT_EVALUATED: " + js);
      } else if (js.find("\"d2h_rc_b\":" +
                         std::to_string(ACL_ERROR_INVALID_PARAM)) ==
                 std::string::npos) {
        lines.push_back("INJ9 FAIL: the failed B transfer is not visible in the "
                        "row: " + js);
      } else if (js.find("D2H of array B failed") == std::string::npos) {
        lines.push_back("INJ9 FAIL: the row does not NAME the transfer failure: " + js);
      } else if (js.find("\"mismatches\":0") != std::string::npos ||
                 js.find("\"guards_ok\":true") != std::string::npos) {
        lines.push_back("INJ9 FAIL: the row carries a measured-looking value on an "
                        "unmeasured run: " + js);
      } else {
        lines.push_back("INJ9 OK: a stage-B row with a failed D2H says "
                        "NOT_EVALUATED, names the transfer failure, and carries no "
                        "measured-looking value");
      }
    }
  }

  // ---- injection 11: the DATA_TRANSFER_FAIL classification must survive -------
  // The run loop classifies a failed D2H as DATA_TRANSFER_FAIL, but the final
  // return used to hand back NUMERICS_FAIL for every failed C result, so the
  // classification was lost exactly when the release had succeeded. Required
  // outcome: a DATA_TRANSFER_FAIL run that cleaned up correctly exits 29, not 25;
  // a genuine numerics failure still exits 25; a clean run exits 0.
  {
    const int xfer = FinalClassify(/*work_ok=*/false, kExitDataTransferFail);
    const int numer = FinalClassify(/*work_ok=*/false, kExitNumericsFail);
    const int clean = FinalClassify(/*work_ok=*/true, kExitOk);
    if (xfer != kExitDataTransferFail) {
      lines.push_back("INJ11 FAIL: a DATA_TRANSFER_FAIL run exited " +
                      std::to_string(xfer) + ", wanted " +
                      std::to_string(kExitDataTransferFail));
    } else if (xfer == kExitNumericsFail) {
      lines.push_back("INJ11 FAIL: the transfer classification collapsed into "
                      "NUMERICS_FAIL");
    } else if (numer != kExitNumericsFail) {
      lines.push_back("INJ11 FAIL: a numerics failure exited " +
                      std::to_string(numer) + ", wanted " +
                      std::to_string(kExitNumericsFail));
    } else if (clean != kExitOk) {
      lines.push_back("INJ11 FAIL: a clean run exited " + std::to_string(clean));
    } else {
      lines.push_back("INJ11 OK: classification survives -- DATA_TRANSFER_FAIL "
                      "=> " + std::to_string(xfer) + ", NUMERICS_FAIL => " +
                      std::to_string(numer) + ", clean => " + std::to_string(clean) +
                      " (the rule is not a constant)");
    }
  }

  // ---- injection 12: the P3-A gate row, in the three states the freeze names --
  // The frozen criteria make "resource release explainable" a condition of P3-A
  // PASS, so the A gate row must be FAIL on a failed release and must not exist
  // before the release is judged. These three rows are emitted through the same
  // BuildGateJson the device path calls, so the text quoted in the evidence is
  // MEASURED here rather than transcribed by hand. The only field that varies at
  // run time is binary_sha256, shown here as a stand-in.
  {
    const std::string extra =
        "\"loader_rc\":0,\"lookup_rc\":0,\"handle_valid\":true,"
        "\"binary_sha256\":\"<measured-at-run-time>\"";
    const CleanupVerdict a_clean = JudgeCleanup({{"aclFinalize", ACL_SUCCESS}});
    const CleanupVerdict a_bad =
        JudgeCleanup({{"aclrtBinaryUnLoad", ACL_ERROR_INVALID_PARAM}});

    const std::string r1 =
        BuildGateJson("P3-A", JudgeStageGate(/*work_ok=*/true, a_clean), extra);
    const std::string r2 =
        BuildGateJson("P3-A", JudgeStageGate(/*work_ok=*/true, a_bad), extra);
    const std::string r3 =
        BuildGateJson("P3-A", JudgeStageGate(/*work_ok=*/false, a_clean), extra);

    if (r1.find("\"result\":\"PASS\"") == std::string::npos) {
      lines.push_back("INJ12 FAIL: loader-ok + release-ok is not PASS: " + r1);
    } else if (r2.find("\"result\":\"FAIL\"") == std::string::npos ||
               r2.find("aclrtBinaryUnLoad rc=") == std::string::npos) {
      lines.push_back("INJ12 FAIL: loader-ok + release-fail is not FAIL with the "
                      "first error: " + r2);
    } else if (r3.find("\"result\":\"FAIL\"") == std::string::npos ||
               r3.find("\"work_ok\":false") == std::string::npos) {
      lines.push_back("INJ12 FAIL: work-false + release-ok is not FAIL: " + r3);
    } else {
      lines.push_back("INJ12 OK: the P3-A gate row is PASS only when the loader "
                      "half AND the release half both hold");
      lines.push_back("INJ12 ROW loader-ok+release-ok: " + r1);
      lines.push_back("INJ12 ROW loader-ok+release-fail: " + r2);
      lines.push_back("INJ12 ROW work-false+release-ok: " + r3);
    }
  }

  std::string joined;
  bool all_ok = true;
  for (const std::string& l : lines) {
    joined += l;
    joined += "\n";
    if (l.find(" FAIL:") != std::string::npos ||
        l.find(" INVALID:") != std::string::npos) {
      all_ok = false;
    }
  }
  *detail = joined;
  return all_ok;
}

// CR-BND-1: the 128 block first/last indices (which already contain the five
// named points 8191/8192/16383/16384/524287).
std::vector<size_t> BoundaryIndices() {
  std::vector<size_t> v;
  for (size_t b = 0; b < kGrid; ++b) {
    v.push_back(b * kElemsPerBlock);
    v.push_back((b + 1) * kElemsPerBlock - 1);
  }
  return v;
}

// ---------------------------------------------------------- pending-work exit

// CR-X-5 / goal section 7.4: after a failed or timed-out sync the kernel may
// still be executing. Do not D2H, free, unload, reset or unwind. Flush what we
// have and leave the process under the watchdog.
[[noreturn]] void ExitWithPendingWork(int code, const char* classification,
                                      const std::string& why) {
  JsonEvent("classification", classification);
  JsonEvent("sync_failure_detail", why);
  JsonEvent("cleanup_incomplete",
            "true: pending work after failed/timed-out sync; no D2H, no free, "
            "no unload, no reset, no destructor unwinding");
  g_log.Note("PENDING_WORK_EXIT",
             std::string("classification=") + classification + " detail=" + why +
                 " exit_code=" + std::to_string(code) +
                 "; cleanup deliberately incomplete");
  g_log.Flush();
  std::fflush(nullptr);
  _exit(code);
}

// ------------------------------------------------------------------ compare

bool CompareAll(const GuardedBuffer& cbuf, const std::vector<uint8_t>& ref,
                const GuardedBuffer& abuf, const GuardedBuffer& bbuf,
                const std::vector<uint8_t>& a_sent,
                const std::vector<uint8_t>& b_sent, const TransferVerdict& tv,
                RunResult* r) {
  r->comparison_attempted = true;
  r->comparison_admissible = tv.admissible;
  r->preservation_measured = tv.preservation_measured;
  r->guards_measured = tv.guards_measured;
  r->transfer_failure_reason = tv.reason;

  // The elementwise comparison reads the C host mirror, the preservation check
  // reads the A/B mirrors, and the guard check reads all three. If any of those
  // D2H copies failed, the mirror still holds the bytes written before H2D, so
  // every one of those readings would describe host state, not device state.
  // Refuse the whole comparison rather than report a partial one: a "no
  // mismatches" produced from a stale mirror is exactly the false PASS this
  // guard exists to prevent.
  if (!tv.admissible) {
    r->compared_elements = 0;  // NOT_EVALUATED: no reading was taken
    r->passed = false;
    return false;
  }

  const uint8_t* dev_c = cbuf.host.data() + kGuardBytes;
  const uint8_t* dev_a = abuf.host.data() + kGuardBytes;
  const uint8_t* dev_b = bbuf.host.data() + kGuardBytes;

  r->block_mismatch.assign(kGrid, 0);

  for (size_t i = 0; i < kElems; ++i) {
    uint32_t dv, rv;
    std::memcpy(&dv, dev_c + i * 4, 4);
    std::memcpy(&rv, ref.data() + i * 4, 4);
    const uint32_t exp = ((dv >> 23) & 0xFFu);
    if (exp == 0xFFu) ++r->nan_or_inf;  // NaN or Inf in device output
    if (dv == kSentinelU32) ++r->sentinel_residue;

    // CR-DATA-6: bitwise equal, OR both zero (+0 == -0). No tolerance.
    const bool both_zero =
        ((dv & 0x7FFFFFFFu) == 0) && ((rv & 0x7FFFFFFFu) == 0);
    if (dv != rv && !both_zero) {
      ++r->mismatches;
      r->block_mismatch[i / kElemsPerBlock] += 1;
      if (r->first_mismatches.size() < 16) {
        r->first_mismatches.emplace_back(i, dv);
      }
      float df, rf;
      std::memcpy(&df, &dv, 4);
      std::memcpy(&rf, &rv, 4);
      const double d =
          std::fabs(static_cast<double>(df) - static_cast<double>(rf));
      if (d > r->max_abs_err) r->max_abs_err = d;
    }
  }

  // CR-BND-1: report every block first/last index individually.
  for (size_t idx : BoundaryIndices()) {
    BoundaryEntry e{};
    e.idx = idx;
    std::memcpy(&e.dev_bits, dev_c + idx * 4, 4);
    std::memcpy(&e.ref_bits, ref.data() + idx * 4, 4);
    const bool both_zero = ((e.dev_bits & 0x7FFFFFFFu) == 0) &&
                           ((e.ref_bits & 0x7FFFFFFFu) == 0);
    e.match = (e.dev_bits == e.ref_bits) || both_zero;
    r->boundary.push_back(e);
  }

  // CR-MEM-4c: A and B payloads bit-identical to what was written. Reached only
  // when BOTH A and B D2H succeeded (tv.admissible already required it), so the
  // mirrors hold device bytes and the check is evidence.
  r->a_unmodified = (std::memcmp(dev_a, a_sent.data(), kPayloadBytes) == 0);
  r->b_unmodified = (std::memcmp(dev_b, b_sent.data(), kPayloadBytes) == 0);

  // CR-MEM-4d: all six guard regions byte-identical to kGuardByte. Guard bytes
  // for C come from C's D2H, which tv.admissible already required.
  bool guards = true;
  for (const GuardedBuffer* b : {&abuf, &bbuf, &cbuf}) {
    for (size_t k = 0; k < kGuardBytes; ++k) {
      if (b->host[k] != kGuardByte ||
          b->host[kGuardBytes + kPayloadBytes + k] != kGuardByte) {
        guards = false;
      }
    }
  }
  r->guards_ok = guards;
  r->compared_elements = kElems;

  r->passed = (r->mismatches == 0) && (r->sentinel_residue == 0) &&
              (r->nan_or_inf == 0) && r->a_unmodified && r->b_unmodified &&
              r->guards_ok;
  return r->passed;
}

// ------------------------------------------------------------------ reporting

// Builds the run record. Split from the write so the fault-injection self-test
// can assert on the TEXT the device path would emit -- the report is the thing
// a reader believes, so "the guard rejects it" and "the record does not claim
// it" are two different claims and both need proving.
std::string BuildRunJson(const RunResult& r) {
  // Three distinct states, and they must not be conflated:
  //   not attempted (stage B)          -> every comparison field null
  //   attempted but inadmissible       -> every READING null, verdict FAIL
  //                                       (a failed D2H makes the readings
  //                                        describe host state, not device
  //                                        state, so none of them is evidence)
  //   attempted and admissible         -> real values
  // A `0`, a `false` or a "PASS" in the first two states would read as a
  // measured numerical result that does not exist.
  const bool attempted = r.comparison_attempted;
  const bool admissible = attempted && r.comparison_admissible;
  const bool compared = admissible && (r.compared_elements > 0);
  std::string s = "{\"event\":\"run\"";
  s += ",\"case\":\"" + JsonEscape(r.case_name) + "\"";
  s += ",\"run\":" + std::to_string(r.run_index);
  s += ",\"executed\":" + std::string(r.executed ? "true" : "false");
  s += ",\"verdict\":";
  if (!attempted) {
    s += "\"NOT_EVALUATED\"";
  } else if (!admissible) {
    s += "\"FAIL\"";
  } else {
    s += std::string(r.passed ? "\"PASS\"" : "\"FAIL\"");
  }
  s += ",\"launch_rc\":" + std::to_string(r.launch_rc);
  s += ",\"sync_rc\":" + std::to_string(r.sync_rc);
  s += ",\"h2d_rc\":" + std::to_string(r.h2d_rc);
  s += ",\"d2h_rc\":" + std::to_string(r.d2h_rc);
  // Per-array transfer rcs, always real numbers (a transfer either happened and
  // returned something, or the field stays -1 meaning "not attempted").
  s += ",\"d2h_rc_a\":" + std::to_string(r.d2h_rc_a);
  s += ",\"d2h_rc_b\":" + std::to_string(r.d2h_rc_b);
  s += ",\"d2h_rc_c\":" + std::to_string(r.d2h_rc_c);
  auto tri = [](bool measured, bool value) -> std::string {
    if (!measured) return "null";
    return value ? "true" : "false";
  };
  auto num = [](bool measured, size_t value) -> std::string {
    if (!measured) return "null";
    return std::to_string(value);
  };
  s += ",\"comparison_attempted\":" + std::string(attempted ? "true" : "false");
  s += ",\"comparison_admissible\":" + std::string(admissible ? "true" : "false");
  s += ",\"transfer_failure\":";
  if (r.transfer_failure_reason.empty()) {
    s += "null";
  } else {
    s += "\"" + JsonEscape(r.transfer_failure_reason) + "\"";
  }
  s += ",\"comparison_performed\":" + std::string(compared ? "true" : "false");
  s += ",\"compared_elements\":" + num(compared, r.compared_elements);
  s += ",\"mismatches\":" + num(compared, r.mismatches);
  s += ",\"sentinel_residue\":" + num(compared, r.sentinel_residue);
  s += ",\"nan_or_inf\":" + num(compared, r.nan_or_inf);
  // The preservation check needs A AND B read back; the guard check needs C.
  s += ",\"a_unmodified\":" +
       tri(compared && r.preservation_measured, r.a_unmodified);
  s += ",\"b_unmodified\":" +
       tri(compared && r.preservation_measured, r.b_unmodified);
  s += ",\"guards_ok\":" + tri(compared && r.guards_measured, r.guards_ok);
  s += ",\"preservation_measured\":";
  s += tri(compared, r.preservation_measured);
  s += ",\"guards_measured\":";
  s += tri(compared, r.guards_measured);
  s += ",\"sentinel_residue_measured\":" + std::string(compared ? "true" : "false");
  {
    char buf[64];
    if (compared) {
      std::snprintf(buf, sizeof(buf), "%.17g", r.max_abs_err);
      s += ",\"max_abs_err\":";
      s += buf;
    } else {
      s += ",\"max_abs_err\":null";
    }
  }
  // The three arrays below are only ever populated by CompareAll. Emit them as
  // null rather than as empty arrays when no admissible comparison happened:
  // `[]` reads as "measured, nothing found", which is a different claim.
  if (!compared) {
    s += ",\"block_mismatch\":null,\"first_mismatches\":null,\"boundary\":null}";
    return s;
  }
  s += ",\"block_mismatch\":[";
  for (size_t i = 0; i < r.block_mismatch.size(); ++i) {
    if (i) s += ",";
    s += std::to_string(r.block_mismatch[i]);
  }
  s += "]";
  s += ",\"first_mismatches\":[";
  for (size_t i = 0; i < r.first_mismatches.size(); ++i) {
    if (i) s += ",";
    char buf[64];
    std::snprintf(buf, sizeof(buf), "{\"idx\":%zu,\"dev_bits\":%u}",
                  r.first_mismatches[i].first, r.first_mismatches[i].second);
    s += buf;
  }
  s += "]";
  s += ",\"boundary\":[";
  for (size_t i = 0; i < r.boundary.size(); ++i) {
    if (i) s += ",";
    char buf[128];
    std::snprintf(buf, sizeof(buf),
                  "{\"idx\":%zu,\"dev_bits\":%u,\"ref_bits\":%u,\"match\":%s}",
                  r.boundary[i].idx, r.boundary[i].dev_bits,
                  r.boundary[i].ref_bits, r.boundary[i].match ? "true" : "false");
    s += buf;
  }
  s += "]}";
  return s;
}

void EmitRunJson(const RunResult& r) { ReportLine(BuildRunJson(r)); }

// Verifies that the on-disk oracle payloads reproduce the frozen CR-DATA-7
// digests for every case before any device work happens.
bool PreflightOracle(const std::string& dir, std::string* err) {
  for (size_t c = 0; c < kNumCases; ++c) {
    for (const char* slug : {"A", "B", "C_ref"}) {
      std::vector<uint8_t> buf;
      if (!LoadOraclePayload(dir, kCases[c], slug, &buf, err)) return false;
    }
  }
  return true;
}

}  // namespace
}  // namespace p3

// --------------------------------------------------------------- argv parsing

struct Options {
  std::string stage = "A";
  std::string binary_path;
  std::string oracle_dir;
  std::string report_path;
  std::string log_path;
  // Bounded-experiment knobs (CR-LCH-6). Defaults reproduce the frozen config
  // exactly: no loader option, and DYN_UBUF_SIZE as the only launch attribute.
  std::string loader_magic = "none";  // "none" | "vector-core" | "aicore" | "cube"
  bool engine_type_aiv = false;
  bool selftest = false;
  bool fault_injection_selftest = false;
};

static bool ParseArgs(int argc, char** argv, Options* o) {
  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    auto val = [&](const char* name) -> std::string {
      const std::string prefix = std::string(name) + "=";
      if (a.rfind(prefix, 0) == 0) return a.substr(prefix.size());
      return std::string();
    };
    std::string v;
    if ((v = val("--stage")).size()) o->stage = v;
    else if ((v = val("--binary")).size()) o->binary_path = v;
    else if ((v = val("--oracle-dir")).size()) o->oracle_dir = v;
    else if ((v = val("--report")).size()) o->report_path = v;
    else if ((v = val("--log")).size()) o->log_path = v;
    else if ((v = val("--loader-magic")).size()) o->loader_magic = v;
    else if (a == "--engine-type-aiv") o->engine_type_aiv = true;
    else if (a == "--selftest") o->selftest = true;
    else if (a == "--fault-injection-selftest") o->fault_injection_selftest = true;
    else {
      std::fprintf(stderr, "unknown argument: %s\n", a.c_str());
      return false;
    }
  }
  return true;
}

int main(int argc, char** argv) {
  using namespace p3;

  Options opt;
  if (!ParseArgs(argc, argv, &opt)) return kExitHarnessError;

  // --- SHA-256 known-answer self-test, before anything else -----------------
  {
    std::string detail;
    if (!Sha256SelfTest(&detail)) {
      std::fprintf(stderr, "SHA256 SELFTEST FAIL: %s\n", detail.c_str());
      return kExitHarnessError;
    }
    std::fprintf(stdout, "SHA256_SELFTEST PASS: %s\n", detail.c_str());
  }
  if (opt.selftest) {
    std::fprintf(stdout, "SELFTEST_ONLY OK\n");
    return kExitOk;
  }

  // --- host-only fault-injection self-test ---------------------------------
  // No device, no ACL call: it drives the admissibility predicates and the real
  // CompareAll against constructed buffers. Runs before aclInit so a failure
  // here cannot be confused with an environment problem.
  if (opt.fault_injection_selftest) {
    std::string detail;
    const bool ok = RunFaultInjectionSelfTest(&detail);
    std::fprintf(stdout, "%s\n", detail.c_str());
    std::fprintf(stdout, "FAULT_INJECTION_SELFTEST %s\n", ok ? "PASS" : "FAIL");
    return ok ? kExitOk : kExitHarnessError;
  }

  g_stage = opt.stage;
  g_report_path = opt.report_path;
  if (!g_log.Open(opt.log_path)) {
    std::fprintf(stderr, "cannot open log %s\n", opt.log_path.c_str());
    return kExitHarnessError;
  }

  ReportLine("{\"event\":\"start\",\"stage\":\"" + g_stage + "\",\"utc\":\"" +
             UtcNow() + "\"}");
  ReportLine("{\"event\":\"frozen_contract\""
             ",\"kernel_sha256\":\"" + std::string(kKernelSha256) + "\""
             ",\"kernel_bytes\":" + std::to_string(kKernelBytes) +
             ",\"grid\":" + std::to_string(kGrid) +
             ",\"dyn_ub_bytes\":" + std::to_string(kDynUbBytes) +
             ",\"elems\":" + std::to_string(kElems) +
             ",\"payload_bytes\":" + std::to_string(kPayloadBytes) +
             ",\"guard_bytes\":" + std::to_string(kGuardBytes) +
             ",\"sync_timeout_ms\":" + std::to_string(kSyncTimeoutMs) + "}");
  ReportLine("{\"event\":\"abi\",\"sizeof_hostargs\":" +
             std::to_string(sizeof(HostArgs)) +
             ",\"offset_a\":" + std::to_string(offsetof(HostArgs, a)) +
             ",\"offset_b\":" + std::to_string(offsetof(HostArgs, b)) +
             ",\"offset_c\":" + std::to_string(offsetof(HostArgs, c)) +
             ",\"placeholder_count\":0}");
  g_log.Note("HARNESS_START", "stage=" + g_stage);

  // --- CR-ID-2/CR-ID-3: read ONCE, hash the in-memory bytes, refuse on
  //     mismatch BEFORE aclInit is reached. -----------------------------------
  std::vector<uint8_t> binary;
  std::string err;
  if (!ReadWholeFile(opt.binary_path, &binary, &err)) {
    JsonEvent("classification", "BINARY_IDENTITY_BLOCKED");
    JsonEvent("reason", err);
    g_log.Note("BINARY_READ_FAILED", err);
    return kExitBinaryIdentityBlocked;
  }
  const std::string measured_sha = Sha256::Hex(binary.data(), binary.size());
  JsonEvent("binary_path", opt.binary_path);
  JsonEvent("binary_sha256_measured", measured_sha);
  JsonEventInt("binary_bytes_measured", static_cast<long long>(binary.size()));
  JsonEvent("binary_sha256_expected", kKernelSha256);
  JsonEventInt("binary_bytes_expected", static_cast<long long>(kKernelBytes));
  JsonEvent("binary_buffer_ptr", HexPtr(binary.data()));

  if (measured_sha != kKernelSha256 || binary.size() != kKernelBytes) {
    JsonEvent("classification", "BINARY_IDENTITY_BLOCKED");
    g_log.Note("BINARY_IDENTITY_BLOCKED",
               "measured sha256=" + measured_sha +
                   " bytes=" + std::to_string(binary.size()) +
                   " expected sha256=" + kKernelSha256 +
                   " bytes=" + std::to_string(kKernelBytes));
    g_log.Flush();
    return kExitBinaryIdentityBlocked;
  }
  JsonEvent("binary_identity", "MATCH");

  // --- oracle preflight (CR-DATA-7), before any device work -----------------
  if (opt.stage == "C") {
    if (!PreflightOracle(opt.oracle_dir, &err)) {
      JsonEvent("classification", "ORACLE_MISMATCH");
      JsonEvent("reason", err);
      g_log.Note("ORACLE_PREFLIGHT_FAILED", err);
      g_log.Flush();
      return kExitOracleMismatch;
    }
    JsonEvent("oracle_preflight", "15/15 payloads reproduce CR-DATA-7");
  }

  // --- watchdog -------------------------------------------------------------
  {
    std::thread wd(WatchdogThread, kWatchdogSec);
    wd.detach();
  }

  // --- device selection (CR-LCH-4: single device, device 0) -----------------
  int32_t device_id = 0;  // goal 5.1: device 0 only; device 1 is another user's
  uint32_t device_count = 0;
  std::string soc_name;
  int32_t ver_major = -1, ver_minor = -1, ver_patch = -1;

  aclError rc = g_log.Log("aclInit", aclInit(nullptr), "");
  if (rc != ACL_SUCCESS) {
    JsonEvent("classification", "ENV_BLOCKED");
    g_log.Flush();
    return kExitEnvBlocked;
  }
  rc = g_log.Log("aclrtGetDeviceCount", aclrtGetDeviceCount(&device_count), "");
  if (rc != ACL_SUCCESS || device_count == 0) {
    JsonEvent("classification", "ENV_BLOCKED");
    JsonEventInt("device_count", static_cast<long long>(device_count));
    g_log.Flush();
    return kExitEnvBlocked;
  }
  rc = g_log.Log("aclrtSetDevice", aclrtSetDevice(device_id),
                 "device_id=" + std::to_string(device_id));
  if (rc != ACL_SUCCESS) {
    JsonEvent("classification", "ENV_BLOCKED");
    g_log.Flush();
    return kExitEnvBlocked;
  }
  {
    const char* soc = aclrtGetSocName();
    soc_name = (soc == nullptr) ? "<null>" : std::string(soc);
  }
  // aclrtGetVersion is deprecated in this CANN in favour of aclSysGetVersion,
  // but aclSysGetVersion is NOT declared in the installed headers (verified by
  // grep over $ASCEND_HOME_PATH/include in this container). The version string
  // is recorded for the evidence record only; it gates nothing. The suppression
  // is scoped to this single call site.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  g_log.Log("aclrtGetVersion",
            aclrtGetVersion(&ver_major, &ver_minor, &ver_patch), "");
#pragma GCC diagnostic pop
  JsonEvent("soc_name", soc_name);
  JsonEventInt("device_count", static_cast<long long>(device_count));
  JsonEventInt("device_id", device_id);
  JsonEventInt("acl_version_major", ver_major);
  JsonEventInt("acl_version_minor", ver_minor);
  JsonEventInt("acl_version_patch", ver_patch);

  // The target contract is Ascend950PR. A different product is ENV_BLOCKED
  // (CR-X-13); we never substitute another device model.
  if (soc_name.find("950") == std::string::npos) {
    JsonEvent("classification", "ENV_BLOCKED");
    JsonEvent("reason", "soc_name is not the Ascend950PR target: " + soc_name);
    g_log.Note("ENV_BLOCKED", "unexpected soc_name " + soc_name);
    g_log.Flush();
    return kExitEnvBlocked;
  }

  aclrtContext ctx = nullptr;
  aclrtStream stream = nullptr;
  rc = g_log.Log("aclrtCreateContext", aclrtCreateContext(&ctx, device_id),
                 "owning context for device " + std::to_string(device_id));
  if (rc != ACL_SUCCESS || ctx == nullptr) {
    JsonEvent("classification", "ENV_BLOCKED");
    g_log.Flush();
    return kExitEnvBlocked;
  }
  rc = g_log.Log("aclrtCreateStream", aclrtCreateStream(&stream),
                 "single stream for the whole P3 run (CR-LCH-4)");
  if (rc != ACL_SUCCESS || stream == nullptr) {
    JsonEvent("classification", "ENV_BLOCKED");
    g_log.Flush();
    return kExitEnvBlocked;
  }

  // --- P3-A: loader + lookup ------------------------------------------------
  aclrtBinHandle bin = nullptr;
  aclrtFuncHandle fn = nullptr;
  bool loader_lookup_ok = false;
  {
    // Bounded-experiment loader option (CR-LCH-6 / design section 2).
    // AscendNPU-IR-1/bishengir/lib/Conversion/HACCToLLVM/GetOrCreateUtils.cpp:332
    // defines kAICoreMagicNumberVec = 0x41415246, and the compiler's own
    // host-wrapper path embeds that constant for vector-core images.
    //
    // MEASURED: the frozen 4448-byte ELF contains ZERO occurrences of any of the
    // three magic values (ACL_RT_BINARY_MAGIC_ELF_AICORE / _VECTOR_CORE /
    // _CUBE_CORE, acl_rt.h:471-473). NOT MEASURED: whether the loader reads the
    // ELF's own `.ascend.meta.main_kernel` NOTE section or its
    // `.AIV_Kernel_Type_main_kernel` symbol to decide the core type. The ELF is
    // NOT "not self-describing"; it carries that section and that symbol. The
    // option below is therefore a measured-configuration choice, not a claim
    // about what the ELF lacks.
    //
    // Default "none" == nullptr options, which is the frozen configuration and
    // matches the local TileLang runtime (tilelang/src/ascend/runtime/
    // ascend_module.cc:408).
    aclrtBinaryLoadOption load_opt{};
    aclrtBinaryLoadOptions load_opts{};
    const aclrtBinaryLoadOptions* opts_ptr = nullptr;
    std::string opt_desc = "options=nullptr";
    if (opt.loader_magic == "vector-core") {
      load_opt.type = ACL_RT_BINARY_LOAD_OPT_MAGIC;
      load_opt.value.magic = ACL_RT_BINARY_MAGIC_ELF_VECTOR_CORE;
      load_opts.options = &load_opt;
      load_opts.numOpt = 1;
      opts_ptr = &load_opts;
      opt_desc = "options=MAGIC id=" +
                 std::to_string(static_cast<int>(load_opt.type)) +
                 " (ACL_RT_BINARY_LOAD_OPT_MAGIC) value=0x" +
                 [&] {
                   char b[16];
                   std::snprintf(b, sizeof(b), "%x", load_opt.value.magic);
                   return std::string(b);
                 }() +
                 " (ACL_RT_BINARY_MAGIC_ELF_VECTOR_CORE) rationale: bounded "
                 "experiment E1/E3 -- the compiler's own host wrapper embeds "
                 "exactly this constant for vector-core images; the value, not "
                 "the presence of an options struct, is what the E4/E5 wrong-core "
                 "controls showed to be the discriminating variable";
    } else if (opt.loader_magic == "aicore" || opt.loader_magic == "cube") {
      // Control: the SAME options mechanism carrying the WRONG core type. This
      // proves the discriminating variable is the magic VALUE, not merely the
      // presence of an options struct.
      load_opt.type = ACL_RT_BINARY_LOAD_OPT_MAGIC;
      load_opt.value.magic = (opt.loader_magic == "aicore")
                                 ? ACL_RT_BINARY_MAGIC_ELF_AICORE
                                 : ACL_RT_BINARY_MAGIC_ELF_CUBE_CORE;
      load_opts.options = &load_opt;
      load_opts.numOpt = 1;
      opts_ptr = &load_opts;
      opt_desc = "options=MAGIC id=" +
                 std::to_string(static_cast<int>(load_opt.type)) +
                 " (ACL_RT_BINARY_LOAD_OPT_MAGIC) value=0x" +
                 [&] {
                   char b[16];
                   std::snprintf(b, sizeof(b), "%x", load_opt.value.magic);
                   return std::string(b);
                 }() +
                 " CONTROL: deliberately the WRONG core type for an AIV image; "
                 "expected to reproduce the device fault";
    } else if (opt.loader_magic != "none") {
      JsonEvent("classification", "HARNESS_ERROR");
      JsonEvent("reason", "unknown --loader-magic: " + opt.loader_magic);
      g_log.Flush();
      return kExitHarnessError;
    }
    JsonEvent("loader_options", opt_desc);

    // CR-ID-2: aclrtBinaryLoadFromData receives THE SAME buffer that was hashed.
    const aclError lrc = g_log.Log(
        "aclrtBinaryLoadFromData",
        aclrtBinaryLoadFromData(binary.data(), binary.size(), opts_ptr, &bin),
        "data_ptr=" + HexPtr(binary.data()) +
            " length=" + std::to_string(binary.size()) +
            " sha256=" + measured_sha + " verified_sha256=" +
            std::string(kKernelSha256) + " " + opt_desc);
    JsonEventInt("loader_rc", lrc);
    if (lrc != ACL_SUCCESS || bin == nullptr) {
      JsonEvent("classification", "LOADER_BLOCKED");
      JsonEvent("loader_handle_null", bin == nullptr ? "true" : "false");
      g_log.Note("LOADER_BLOCKED",
                 "aclrtBinaryLoadFromData rc=" + std::to_string(lrc));
      g_log.Flush();
      return kExitLoaderBlocked;
    }
    JsonEvent("loader_handle_valid", "true");

    const aclError frc = g_log.Log(
        "aclrtBinaryGetFunction",
        aclrtBinaryGetFunction(bin, "main_kernel", &fn),
        "kernel_name=\"main_kernel\" (CR-ELF-7: NOT main_kernel$local, NOT a VF "
        "helper such as main_kernel_simd_vf_0.vector.thread)");
    JsonEventInt("lookup_rc", frc);
    if (frc != ACL_SUCCESS || fn == nullptr) {
      JsonEvent("classification", "LOADER_BLOCKED");
      JsonEvent("lookup_handle_null", fn == nullptr ? "true" : "false");
      g_log.Note("LOADER_BLOCKED",
                 "aclrtBinaryGetFunction rc=" + std::to_string(frc));
      g_log.Flush();
      return kExitLoaderBlocked;
    }
    JsonEvent("lookup_handle_valid", "true");
    loader_lookup_ok = true;  // both the load and the lookup succeeded
  }

  // The P3-A gate row is NOT emitted here. The frozen criteria make "resource
  // release explainable" a CONDITION of P3-A PASS (P3-ACCEPTANCE-CRITERIA.txt:
  // 279-284, "PASS only if all hold"), so the gate is the AND of the
  // loader/lookup half and the release half -- and it is not computable until
  // the release has been judged. A gate row written before the release has no
  // second operand, and an equality whose second operand is absent must not read
  // as PASS. This is the same defect class as the P3-B/P3-C gate ordering fixed
  // in r6/r7, and it is fixed the same way: one verdict, through the same shared
  // seam (JudgeStageGate -> BuildGateJson), emitted after the release.

  if (opt.stage == "A") {
    // Normal release path (explainable, ordered, each rc logged AND JUDGED).
    // The rcs are collected rather than ignored: a release that failed must
    // prohibit the P3-A PASS while preserving the first error.
    std::vector<std::pair<const char*, aclError>> a_ops;
    a_ops.emplace_back("aclrtBinaryUnLoad",
                       g_log.Log("aclrtBinaryUnLoad", aclrtBinaryUnLoad(bin), ""));
    a_ops.emplace_back("aclrtDestroyStream",
                       g_log.Log("aclrtDestroyStream", aclrtDestroyStream(stream), ""));
    a_ops.emplace_back("aclrtDestroyContext",
                       g_log.Log("aclrtDestroyContext", aclrtDestroyContext(ctx), ""));
    a_ops.emplace_back("aclrtResetDevice",
                       g_log.Log("aclrtResetDevice", aclrtResetDevice(device_id),
                                 "paired with this process's aclrtSetDevice"));
    a_ops.emplace_back("aclFinalize", g_log.Log("aclFinalize", aclFinalize(), ""));
    const CleanupVerdict cv = JudgeCleanup(a_ops);
    const StageGateVerdict gv = JudgeStageGate(loader_lookup_ok, cv);
    const std::string extra =
        "\"loader_rc\":0,\"lookup_rc\":0,\"handle_valid\":true,\"binary_sha256\":\"" +
        measured_sha + "\"";
    ReportLine("{\"event\":\"cleanup\",\"ok\":" +
               std::string(cv.ok ? "true" : "false") +
               ",\"first_failure\":" +
               (cv.first_failure.empty()
                    ? std::string("null")
                    : "\"" + JsonEscape(cv.first_failure) + "\"") +
               ",\"log_first_failure\":" +
               (g_log.first_failure().empty()
                    ? std::string("null")
                    : "\"" + JsonEscape(g_log.first_failure()) + "\"") +
               ",\"nonzero_rc_count\":" + std::to_string(g_log.nonzero_rc_count()) +
               "}");
    ReportLine(BuildGateJson("P3-A", gv, extra));
    ReportLine("{\"stage\":\"A\",\"result\":\"" +
               std::string(gv.pass ? "PASS" : "FAIL") +
               "\",\"work_result\":\"" +
               std::string(loader_lookup_ok ? "PASS" : "FAIL") +
               "\",\"cleanup_ok\":" + std::string(cv.ok ? "true" : "false") +
               "}");
    std::fprintf(stdout,
                 "P3-A %s loader_rc=0 lookup_rc=0 handle_valid=1 cleanup_ok=%d\n",
                 gv.pass ? "PASS" : "FAIL", cv.ok ? 1 : 0);
    std::fflush(stdout);
    g_log.Note("P3_A_GATE", gv.pass ? "PASS" : "FAIL");
    g_log.Note("HARNESS_END",
               std::string("stage=A result=") + (gv.pass ? "PASS" : "FAIL") +
                   " work=" + (loader_lookup_ok ? "PASS" : "FAIL") +
                   " cleanup_ok=" + (cv.ok ? "true" : "false"));
    g_log.Flush();
    if (!cv.ok) {
      std::fprintf(stdout, "STAGE A CLEANUP FAILED: %s\n", cv.first_failure.c_str());
      std::fflush(stdout);
      return kExitCleanupFail;
    }
    std::fprintf(stdout, "STAGE A COMPLETE\n");
    return gv.pass ? kExitOk : kExitLoaderBlocked;
  }

  // --- P3-B/C: allocate, launch, sync, (compare) -----------------------------
  GuardedBuffer A, B, C;
  auto alloc = [&](GuardedBuffer* b, const char* name) -> bool {
    b->alloc_bytes = kAllocBytes + 512;
    b->host.assign(b->alloc_bytes, 0);
    const aclError arc =
        g_log.Log("aclrtMalloc",
                  aclrtMalloc(&b->dev_raw, b->alloc_bytes,
                              ACL_MEM_MALLOC_HUGE_FIRST),
                  std::string("array=") + name +
                      " bytes=" + std::to_string(b->alloc_bytes));
    if (arc != ACL_SUCCESS || b->dev_raw == nullptr) return false;
    const uintptr_t raw = reinterpret_cast<uintptr_t>(b->dev_raw);
    const uintptr_t aligned = (raw + 511u) & ~static_cast<uintptr_t>(511u);
    b->dev_base = reinterpret_cast<void*>(aligned);
    b->dev_payload = reinterpret_cast<void*>(aligned + kGuardBytes);
    JsonEvent((std::string("alloc_") + name).c_str(),
              "raw=" + HexPtr(b->dev_raw) + " base=" + HexPtr(b->dev_base) +
                  " payload=" + HexPtr(b->dev_payload) +
                  " payload_mod32=" +
                  std::to_string(reinterpret_cast<uintptr_t>(b->dev_payload) % 32) +
                  " payload_mod512=" +
                  std::to_string(reinterpret_cast<uintptr_t>(b->dev_payload) % 512) +
                  " alloc_bytes=" + std::to_string(b->alloc_bytes));
    return true;
  };

  if (!alloc(&A, "A") || !alloc(&B, "B") || !alloc(&C, "C")) {
    JsonEvent("classification", "ENV_BLOCKED");
    JsonEvent("reason", "aclrtMalloc failed");
    g_log.Flush();
    return kExitEnvBlocked;
  }
  // CR-MEM-3 gate: payload base % 32 == 0. Alignment is recorded either way.
  {
    bool align_ok = true;
    const std::pair<GuardedBuffer*, const char*> arrs[] = {
        {&A, "A"}, {&B, "B"}, {&C, "C"}};
    for (const auto& pr : arrs) {
      const uintptr_t p = reinterpret_cast<uintptr_t>(pr.first->dev_payload);
      JsonEventInt((std::string("payload_align32_") + pr.second).c_str(),
                   static_cast<long long>(p % 32));
      JsonEventInt((std::string("payload_align512_") + pr.second).c_str(),
                   static_cast<long long>(p % 512));
      if (p % 32 != 0) align_ok = false;
    }
    if (!align_ok) {
      JsonEvent("classification", "ENV_BLOCKED");
      JsonEvent("reason", "payload alignment not a multiple of 32");
      g_log.Flush();
      return kExitEnvBlocked;
    }
  }

  std::vector<uint8_t> a_bytes, b_bytes, c_ref;
  auto load_case = [&](const std::string& case_name) -> bool {
    if (!LoadOraclePayload(opt.oracle_dir, case_name, "A", &a_bytes, &err))
      return false;
    if (!LoadOraclePayload(opt.oracle_dir, case_name, "B", &b_bytes, &err))
      return false;
    if (!LoadOraclePayload(opt.oracle_dir, case_name, "C_ref", &c_ref, &err))
      return false;
    return true;
  };

  // The literal sentinel bytes (CR-MEM-1): 45 23 c1 7f, little-endian.
  const uint8_t kSentinelLE[4] = {0x45, 0x23, 0xC1, 0x7F};
  {
    uint32_t check = 0;
    std::memcpy(&check, kSentinelLE, 4);
    if (check != kSentinelU32) {
      JsonEvent("classification", "HARNESS_ERROR");
      JsonEvent("reason", "sentinel byte literal does not encode kSentinelU32");
      g_log.Flush();
      return kExitHarnessError;
    }
  }

  int overall = kExitOk;
  bool any_fail = false;
  long runs_executed = 0;
  // Stage-B gate halves, carried out of the loop because the gate row must be
  // written after the release has been judged.
  bool b_work_ok = false;
  aclError b_launch_rc = -1;
  aclError b_sync_rc = -1;
  // Carried out of the loop for the B gate row: a failed D2H does not change the
  // B gate (launch/sync + release) but it must be NAMED in the row, because a
  // row showing only launch_rc=0/sync_rc=0 reads as a clean run.
  bool b_transfer_admissible = true;
  std::string b_transfer_failure;

  const int runs_per_case = (opt.stage == "B") ? 1 : 3;
  const size_t cases_to_run = (opt.stage == "B") ? 1 : kNumCases;

  for (size_t ci = 0; ci < cases_to_run && overall == kExitOk; ++ci) {
    const std::string case_name = kCases[ci];
    if (!load_case(case_name)) {
      JsonEvent("classification", "ORACLE_MISMATCH");
      JsonEvent("reason", err);
      g_log.Note("ORACLE_LOAD_FAILED", err);
      g_log.Flush();
      return kExitOracleMismatch;
    }

    for (int run = 0; run < runs_per_case; ++run) {
      RunResult r;
      r.case_name = case_name;
      r.run_index = run;

      // CR-REP-1: re-write C sentinel + all six guards, and re-write A/B, before
      // H2D and before launching.
      std::memcpy(HostPayload(&A), a_bytes.data(), kPayloadBytes);
      std::memcpy(HostPayload(&B), b_bytes.data(), kPayloadBytes);
      {
        uint8_t* cp = HostPayload(&C);
        for (size_t i = 0; i < kElems; ++i) std::memcpy(cp + i * 4, kSentinelLE, 4);
      }
      FillGuards(&A);
      FillGuards(&B);
      FillGuards(&C);

      // H2D the whole allocation (guards included).
      bool h2d_ok = true;
      const std::pair<GuardedBuffer*, const char*> arrs[] = {
          {&A, "A"}, {&B, "B"}, {&C, "C"}};
      for (const auto& pr : arrs) {
        const aclError hrc = g_log.Log(
            "aclrtMemcpy",
            aclrtMemcpy(pr.first->dev_base, kAllocBytes, pr.first->host.data(),
                        kAllocBytes, ACL_MEMCPY_HOST_TO_DEVICE),
            std::string("array=") + pr.second + " bytes=" +
                std::to_string(kAllocBytes) + " dir=H2D guards_included=1");
        if (std::strcmp(pr.second, "C") == 0) r.h2d_rc = hrc;
        if (hrc != ACL_SUCCESS) h2d_ok = false;
      }
      if (!h2d_ok) {
        JsonEvent("classification", "ENV_BLOCKED");
        JsonEvent("reason", "H2D copy failed before launch");
        g_log.Flush();
        return kExitEnvBlocked;
      }

      // Launch: CR-LCH-2/CR-LCH-5 -- DYN_UBUF_SIZE is mandatory. Any additional
      // attribute is permitted by CR-LCH-6 only when logged with id+value and a
      // rationale; ENGINE_TYPE is gated behind the bounded-experiment flag.
      HostArgs args;
      args.a = A.dev_payload;  // CR-ABI-4: DEVICE GM payload addresses
      args.b = B.dev_payload;
      args.c = C.dev_payload;
      JsonEvent("launch_args", "A=" + HexPtr(args.a) + " B=" + HexPtr(args.b) +
                                   " C=" + HexPtr(args.c) + " sizeof=" +
                                   std::to_string(sizeof(HostArgs)) +
                                   " placeholders=0");

      aclrtLaunchKernelAttr attr[2]{};
      size_t num_attrs = 0;
      attr[num_attrs].id = ACL_RT_LAUNCH_KERNEL_ATTR_DYN_UBUF_SIZE;
      attr[num_attrs].value.dynUBufSize = kDynUbBytes;
      ++num_attrs;
      JsonEvent("launch_attr_0",
                "id=" + std::to_string(static_cast<int>(attr[0].id)) +
                    " (ACL_RT_LAUNCH_KERNEL_ATTR_DYN_UBUF_SIZE) value=" +
                    std::to_string(attr[0].value.dynUBufSize) +
                    " rationale: CR-LCH-2 mandates dynamic UB");
      if (opt.engine_type_aiv) {
        attr[num_attrs].id = ACL_RT_LAUNCH_KERNEL_ATTR_ENGINE_TYPE;
        attr[num_attrs].value.engineType = ACL_RT_ENGINE_TYPE_AIV;
        ++num_attrs;
        JsonEvent("launch_attr_1",
                  "id=" + std::to_string(static_cast<int>(
                             ACL_RT_LAUNCH_KERNEL_ATTR_ENGINE_TYPE)) +
                      " (ACL_RT_LAUNCH_KERNEL_ATTR_ENGINE_TYPE) value=" +
                      std::to_string(static_cast<int>(ACL_RT_ENGINE_TYPE_AIV)) +
                      " (ACL_RT_ENGINE_TYPE_AIV) rationale: bounded experiment "
                      "E2/E3, the wrong-value control for the loader magic. "
                      "CONSISTENT HYPOTHESIS (not a proven mechanism): the frozen "
                      "binary is an AIV/vector-core image "
                      "(.AIV_Kernel_Type_main_kernel, VF/MOV_OUT_TO_UB_ALIGN_V2/"
                      "BAR.ALL), and 11 faulting runs across four binary "
                      "generations all reported exactly 28 cores (0..13, 18..31) "
                      "at one pc, which is the artifact's AI_CORE_COUNT while "
                      "VECTOR_CORE_COUNT is 56. The measured, reproducible fact is "
                      "the fault set and the magic's discriminating effect; the "
                      "core-count explanation is not established here");
      } else {
        JsonEvent("launch_attr_1", "absent (ENGINE_TYPE not used in this variant)");
      }
      aclrtLaunchKernelCfg cfg{};
      cfg.attrs = attr;
      cfg.numAttrs = num_attrs;
      JsonEventInt("launch_attr_count", static_cast<long long>(num_attrs));

      g_pending_work = true;  // from here until a successful sync
      r.launch_rc = g_log.Log(
          "aclrtLaunchKernelWithHostArgs",
          aclrtLaunchKernelWithHostArgs(fn, kGrid, stream, &cfg, &args,
                                        sizeof(args), nullptr, 0),
          "numBlocks=" + std::to_string(kGrid) +
              " argsSize=" + std::to_string(sizeof(args)) +
              " placeHolderNum=0 dynUBufSize=" + std::to_string(kDynUbBytes));
      JsonEventInt("launch_rc", r.launch_rc);

      if (r.launch_rc != ACL_SUCCESS) {
        // Launch rejected: nothing was enqueued, so no work is pending and the
        // normal cleanup path is safe and explainable.
        g_pending_work = false;
        JsonEvent("classification", "LAUNCH_BLOCKED");
        JsonEvent("cleanup_incomplete", "false (launch rejected; no work enqueued)");
        g_log.Log("aclrtBinaryUnLoad", aclrtBinaryUnLoad(bin),
                  "after launch rejection");
        g_log.Flush();
        return kExitLaunchBlocked;
      }

      // CR-LCH-7 / CR-X-4: a launch rc=0 is NOT execution success. Read C only
      // after this sync returns 0.
      r.sync_rc = g_log.Log(
          "aclrtSynchronizeStreamWithTimeout",
          aclrtSynchronizeStreamWithTimeout(stream, kSyncTimeoutMs),
          "timeout_ms=" + std::to_string(kSyncTimeoutMs) +
              " (finite; -1 infinite and 0 immediate-check are both forbidden)");
      JsonEventInt("sync_rc", r.sync_rc);

      if (r.sync_rc != ACL_SUCCESS) {
        // The kernel may still be running. Preserve, flush, exit under the
        // watchdog; do NOT D2H, free, unload, reset or unwind.
        const std::string detail =
            "aclrtSynchronizeStreamWithTimeout rc=" + std::to_string(r.sync_rc) +
            " (" + AclErrorName(r.sync_rc) + ") timeout_ms=" +
            std::to_string(kSyncTimeoutMs);
        JsonEvent("sync_error_name", AclErrorName(r.sync_rc));
        if (IsDeviceFault(r.sync_rc)) {
          // The device reported a fault while executing the kernel. This is NOT
          // a timeout: the task ended, badly. Classified as a device execution
          // failure (goal section 6: "no unexplained device execution fault").
          ExitWithPendingWork(kExitDeviceFault, "DEVICE_FAULT", detail);
        }
        ExitWithPendingWork(kExitTimeout, "TIMEOUT", detail);
      }
      g_pending_work = false;  // sync succeeded: the kernel has completed

      // D2H the whole allocation (guards included) -- only now, and only after
      // a successful sync. ALL THREE rcs are recorded: the comparison reads C
      // for the result and A/B for the preservation check, so a failed A or B
      // copy silently invalidates the preservation check (the host mirror still
      // holds the bytes written before H2D, so memcmp would report "unmodified"
      // for bytes never read back).
      for (const auto& pr : arrs) {
        const aclError drc = g_log.Log(
            "aclrtMemcpy",
            aclrtMemcpy(pr.first->host.data(), kAllocBytes, pr.first->dev_base,
                        kAllocBytes, ACL_MEMCPY_DEVICE_TO_HOST),
            std::string("array=") + pr.second + " bytes=" +
                std::to_string(kAllocBytes) + " dir=D2H after_successful_sync=1");
        if (std::strcmp(pr.second, "A") == 0) r.d2h_rc_a = drc;
        if (std::strcmp(pr.second, "B") == 0) r.d2h_rc_b = drc;
        if (std::strcmp(pr.second, "C") == 0) { r.d2h_rc = drc; r.d2h_rc_c = drc; }
      }
      const TransferVerdict tv = JudgeTransfers(r.d2h_rc_a, r.d2h_rc_b, r.d2h_rc_c);
      if (!tv.admissible) {
        JsonEvent("transfer_failure", tv.reason);
        JsonEvent("classification", "DATA_TRANSFER_FAIL");
        g_log.Note("D2H_INADMISSIBLE", tv.reason +
                                           " preservation_measured=" +
                                           (tv.preservation_measured ? "true" : "false") +
                                           " guards_measured=" +
                                           (tv.guards_measured ? "true" : "false"));
      }

      if (opt.stage == "B") {
        // Stage B is launch/sync only: no comparison is performed, so the
        // D2H rcs above are recorded for the record but are not gating here.
        // (They are taken only after a successful sync, per CR-X-4.)
        //
        // They are not ignored either. A failed D2H in stage B leaves the run
        // record's readings unmeasured (NOT_EVALUATED, every field null) but the
        // record must still NAME the failure: a row that showed only
        // "launch_rc=0 sync_rc=0" would read as a clean run. This does NOT
        // change the B gate, which the frozen criteria define as launch/sync plus
        // resource lifetimes; the classification is carried alongside it.
        if (!tv.admissible) r.transfer_failure_reason = tv.reason;
        //
        // The P3-B gate row is NOT emitted here. The frozen criteria include
        // "all resource lifetimes controlled" in the P3-B gate, so the gate is
        // the AND of this work half and the release half -- and a gate row
        // written before the release has no second operand. It is emitted after
        // run_cleanup() below.
        r.executed = true;
        r.passed = (r.launch_rc == ACL_SUCCESS) && (r.sync_rc == ACL_SUCCESS);
        b_work_ok = r.passed;
        b_launch_rc = r.launch_rc;
        b_sync_rc = r.sync_rc;
        b_transfer_admissible = tv.admissible;
        b_transfer_failure = tv.reason;
        EmitRunJson(r);
        std::fprintf(stdout, "P3-B work %s launch_rc=%d sync_rc=%d d2h_admissible=%d\n",
                     r.passed ? "PASS" : "FAIL", r.launch_rc, r.sync_rc,
                     tv.admissible ? 1 : 0);
        if (!tv.admissible) {
          std::fprintf(stdout, "P3-B NOTE: %s (readings unmeasured; the B gate is "
                               "launch/sync + release and is unchanged)\n",
                       tv.reason.c_str());
        }
        std::fflush(stdout);
        break;
      }

      CompareAll(C, c_ref, A, B, a_bytes, b_bytes, tv, &r);
      r.executed = true;
      EmitRunJson(r);
      if (!tv.admissible) {
        // A transfer the comparison depends on failed. The numerical readings
        // are NOT evidence, so this is not a numerics failure and must not be
        // reported as one: it is a data-transfer failure, and the run result is
        // FAIL (never PASS) with the preservation/guard checks unevaluated.
        any_fail = true;
        overall = kExitDataTransferFail;
        JsonEvent("classification", "DATA_TRANSFER_FAIL");
        JsonEvent("first_failing_run", case_name + ".r" + std::to_string(run));
        g_log.Note("DATA_TRANSFER_FAIL", case_name + ".r" + std::to_string(run) +
                                            " " + tv.reason);
        std::fprintf(stdout, "RUN %s.r%d FAIL_DATA_TRANSFER %s\n",
                     case_name.c_str(), run, tv.reason.c_str());
        std::fflush(stdout);
        break;
      }
      ++runs_executed;
      if (!r.passed) {
        any_fail = true;
        overall = kExitNumericsFail;
        JsonEvent("classification", "NUMERICS_FAIL");
        JsonEvent("first_failing_run", case_name + ".r" + std::to_string(run));
        g_log.Note("NUMERICS_FAIL", case_name + ".r" + std::to_string(run) +
                                        " mismatches=" +
                                        std::to_string(r.mismatches));
        break;
      }
      std::fprintf(stdout,
                   "RUN %s.r%d PASS launch_rc=%d sync_rc=%d compared=%zu "
                   "mismatches=0 sentinel=0 guards_ok=1 a_ok=1 b_ok=1\n",
                   case_name.c_str(), run, r.launch_rc, r.sync_rc, kElems);
      std::fflush(stdout);
    }
  }

  // Full, ordered release. The rcs are COLLECTED and judged, not merely logged:
  // a release that failed must prohibit the stage PASS while preserving the
  // first error. The numerical observations already written to the report stand
  // unchanged -- what fails is the verdict, not the record.
  auto run_cleanup = [&]() -> CleanupVerdict {
    std::vector<std::pair<const char*, aclError>> ops;
    ops.emplace_back("aclrtFree(A)",
                     g_log.Log("aclrtFree", aclrtFree(A.dev_raw), "array=A"));
    ops.emplace_back("aclrtFree(B)",
                     g_log.Log("aclrtFree", aclrtFree(B.dev_raw), "array=B"));
    ops.emplace_back("aclrtFree(C)",
                     g_log.Log("aclrtFree", aclrtFree(C.dev_raw), "array=C"));
    ops.emplace_back("aclrtBinaryUnLoad",
                     g_log.Log("aclrtBinaryUnLoad", aclrtBinaryUnLoad(bin), ""));
    ops.emplace_back("aclrtDestroyStream",
                     g_log.Log("aclrtDestroyStream", aclrtDestroyStream(stream), ""));
    ops.emplace_back("aclrtDestroyContext",
                     g_log.Log("aclrtDestroyContext", aclrtDestroyContext(ctx), ""));
    ops.emplace_back("aclrtResetDevice",
                     g_log.Log("aclrtResetDevice", aclrtResetDevice(device_id),
                               "paired with this process's aclrtSetDevice"));
    ops.emplace_back("aclFinalize", g_log.Log("aclFinalize", aclFinalize(), ""));
    return JudgeCleanup(ops);
  };

  if (opt.stage == "B") {
    // Release FIRST, gate SECOND: the P3-B gate includes "all resource
    // lifetimes controlled", so the gate row cannot be written until the
    // release has been judged. A release failure therefore makes the gate row
    // say FAIL, not PASS -- the two claims are emitted together, from one
    // verdict, and cannot disagree.
    const CleanupVerdict cv = run_cleanup();
    const StageGateVerdict gv = JudgeStageGate(b_work_ok, cv);
    // The first error is preserved by the logger (RecordFirstFailure never
    // overwrites) and reported here verbatim.
    ReportLine("{\"event\":\"cleanup\",\"ok\":" +
               std::string(cv.ok ? "true" : "false") +
               ",\"first_failure\":" +
               (cv.first_failure.empty()
                    ? std::string("null")
                    : "\"" + JsonEscape(cv.first_failure) + "\"") +
               ",\"log_first_failure\":" +
               (g_log.first_failure().empty()
                    ? std::string("null")
                    : "\"" + JsonEscape(g_log.first_failure()) + "\"") +
               ",\"nonzero_rc_count\":" + std::to_string(g_log.nonzero_rc_count()) +
               "}");
    const std::string extra =
        "\"launch_rc\":" + std::to_string(b_launch_rc) +
        ",\"sync_rc\":" + std::to_string(b_sync_rc) +
        ",\"grid\":" + std::to_string(kGrid) +
        ",\"dyn_ub_bytes\":" + std::to_string(kDynUbBytes) +
        ",\"d2h_admissible\":" +
        std::string(b_transfer_admissible ? "true" : "false") +
        ",\"transfer_failure\":" +
        (b_transfer_failure.empty()
             ? std::string("null")
             : "\"" + JsonEscape(b_transfer_failure) + "\"");
    ReportLine(BuildGateJson("P3-B", gv, extra));
    ReportLine("{\"stage\":\"B\",\"result\":\"" +
               std::string(gv.pass ? "PASS" : "FAIL") +
               "\",\"work_result\":\"" +
               std::string(b_work_ok ? "PASS" : "FAIL") +
               "\",\"cleanup_ok\":" + std::string(cv.ok ? "true" : "false") +
               ",\"numerical_claim\":\"none\",\"readings_measured\":" +
               std::string(b_transfer_admissible ? "true" : "false") +
               ",\"transfer_failure\":" +
               (b_transfer_failure.empty()
                    ? std::string("null")
                    : "\"" + JsonEscape(b_transfer_failure) + "\"") +
               "}");
    std::fprintf(stdout, "P3-B %s launch_rc=%d sync_rc=%d cleanup_ok=%d\n",
                 gv.pass ? "PASS" : "FAIL", b_launch_rc, b_sync_rc,
                 cv.ok ? 1 : 0);
    if (!b_transfer_admissible) {
      std::fprintf(stdout,
                   "P3-B NOTE: %s -- the B gate is launch/sync + release and is "
                   "unchanged; this run took no numerical reading\n",
                   b_transfer_failure.c_str());
    }
    std::fflush(stdout);
    g_log.Note("P3_B_GATE", gv.pass ? "PASS" : "FAIL");
    g_log.Note("HARNESS_END",
               std::string("stage=B result=") + (gv.pass ? "PASS" : "FAIL") +
                   " work=" + (b_work_ok ? "PASS" : "FAIL") +
                   " cleanup_ok=" + (cv.ok ? "true" : "false"));
    g_log.Flush();
    if (!cv.ok) {
      std::fprintf(stdout, "STAGE B CLEANUP FAILED: %s\n",
                   cv.first_failure.c_str());
      std::fflush(stdout);
      return kExitCleanupFail;
    }
    std::fprintf(stdout, "STAGE B COMPLETE\n");
    return gv.pass ? kExitOk : kExitLaunchBlocked;
  }

  // --- P3-C verdict ---------------------------------------------------------
  // Numerical verdict and stage verdict are SEPARATE: the stage PASS requires
  // both a clean numerical verdict and a clean release.
  const bool numerics_pass =
      (!any_fail) && (runs_executed == static_cast<long>(kNumCases) * 3);

  const CleanupVerdict cv = run_cleanup();
  const StageGateVerdict gv = JudgeStageGate(numerics_pass, cv);
  const bool c_pass = gv.pass;
  // The classification the run loop recorded (kExitOk if none). Read here, at
  // last: before r9 it was assigned and never read, so a DATA_TRANSFER_FAIL run
  // that cleaned up correctly was reported as NUMERICS_FAIL.
  const int exit_classification = FinalClassify(numerics_pass, overall);
  ReportLine("{\"event\":\"cleanup\",\"ok\":" +
             std::string(cv.ok ? "true" : "false") +
             ",\"first_failure\":" +
             (cv.first_failure.empty()
                  ? std::string("null")
                  : "\"" + JsonEscape(cv.first_failure) + "\"") +
             ",\"log_first_failure\":" +
             (g_log.first_failure().empty()
                  ? std::string("null")
                  : "\"" + JsonEscape(g_log.first_failure()) + "\"") +
             ",\"nonzero_rc_count\":" + std::to_string(g_log.nonzero_rc_count()) +
             "}");
  const std::string extra =
      "\"runs_executed\":" + std::to_string(runs_executed) +
      ",\"runs_required\":" + std::to_string(kNumCases * 3) +
      ",\"compared_elements_per_run\":" + std::to_string(kElems);
  ReportLine(BuildGateJson("P3-C", gv, extra));
  ReportLine("{\"stage\":\"C\",\"result\":\"" +
             std::string(c_pass ? "PASS" : "FAIL") +
             "\",\"numerics_result\":\"" +
             std::string(numerics_pass ? "PASS" : "FAIL") +
             "\",\"cleanup_ok\":" + std::string(cv.ok ? "true" : "false") +
             ",\"exit_classification\":" + std::to_string(exit_classification) +
             ",\"exit_classification_name\":\"" + AclErrorName(exit_classification) +
             "\"}");
  std::fprintf(stdout, "P3-C %s runs_executed=%ld/%zu cleanup_ok=%d\n",
               c_pass ? "PASS" : "FAIL", runs_executed, kNumCases * 3,
               cv.ok ? 1 : 0);
  std::fflush(stdout);
  g_log.Note("P3_C_GATE", c_pass ? "PASS" : "FAIL");
  g_log.Note("HARNESS_END",
             std::string("stage=C result=") + (c_pass ? "PASS" : "FAIL") +
                 " numerics=" + (numerics_pass ? "PASS" : "FAIL") +
                 " cleanup_ok=" + (cv.ok ? "true" : "false"));
  g_log.Flush();
  if (!cv.ok) {
    std::fprintf(stdout, "STAGE C CLEANUP FAILED: %s\n", cv.first_failure.c_str());
    std::fflush(stdout);
    return kExitCleanupFail;
  }
  std::fprintf(stdout, "STAGE C COMPLETE\n");
  return exit_classification;
}
