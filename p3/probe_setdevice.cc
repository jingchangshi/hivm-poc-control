// Minimal bounded probe: how long does each device-init ACL call take?
//
// Discriminating experiment (declared BEFORE running, per the acceptance
// criteria's LOADER_BLOCKED allowance):
//
//   HYPOTHESIS H1: aclrtSetDevice(0) on this box is slow (order 10 s, as
//     observed in the P3-A run) and the P3-B watchdog expiry was that call
//     exceeding the 120 s wall-clock budget under load from another user's job
//     on device 1.
//   PREDICTION: with a 300 s bound, the probe reaches "setdevice done" and
//     reports a finite elapsed time; the loader path is then reachable again.
//   FALSIFIER: the probe does not reach "setdevice done" within 300 s, i.e.
//     aclrtSetDevice(0) itself is wedged => ENV_BLOCKED, not a harness defect.
//
// This program does NO launch and NO allocation. It is read-only with respect
// to device memory and touches device 0 only.
#include <cstdio>
#include <cstdint>
#include <ctime>

#include "acl/acl.h"

static double NowMs() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static void Step(const char* name, aclError rc, double t0) {
  std::printf("%-28s rc=%-4d elapsed_ms=%.1f\n", name, rc, NowMs() - t0);
  std::fflush(stdout);
}

int main() {
  double t0;
  t0 = NowMs();
  aclError rc = aclInit(nullptr);
  Step("aclInit", rc, t0);
  if (rc != ACL_SUCCESS) return 1;

  uint32_t count = 0;
  t0 = NowMs();
  rc = aclrtGetDeviceCount(&count);
  Step("aclrtGetDeviceCount", rc, t0);
  std::printf("device_count=%u\n", count);
  std::fflush(stdout);

  t0 = NowMs();
  rc = aclrtSetDevice(0);
  Step("aclrtSetDevice(0)", rc, t0);
  if (rc != ACL_SUCCESS) return 2;

  aclrtContext ctx = nullptr;
  t0 = NowMs();
  rc = aclrtCreateContext(&ctx, 0);
  Step("aclrtCreateContext", rc, t0);

  aclrtStream stream = nullptr;
  t0 = NowMs();
  rc = aclrtCreateStream(&stream);
  Step("aclrtCreateStream", rc, t0);

  t0 = NowMs();
  rc = aclrtDestroyStream(stream);
  Step("aclrtDestroyStream", rc, t0);

  t0 = NowMs();
  rc = aclrtDestroyContext(ctx);
  Step("aclrtDestroyContext", rc, t0);

  t0 = NowMs();
  rc = aclrtResetDevice(0);
  Step("aclrtResetDevice(0)", rc, t0);

  t0 = NowMs();
  rc = aclFinalize();
  Step("aclFinalize", rc, t0);

  std::printf("PROBE_COMPLETE\n");
  return 0;
}
