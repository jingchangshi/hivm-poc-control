// Self-contained SHA-256 (FIPS 180-4). Used to bind the frozen kernel.o bytes
// to the identity in the acceptance criteria, and to verify the oracle payload
// files against CR-DATA-7.
//
// Deliberately dependency-free: the goal permits "C++ standard library, CANN
// ACL and a necessary SHA-256 implementation". A system crypto library would
// add a runtime dependency whose identity we would then also have to record.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace p3 {

class Sha256 {
 public:
  Sha256();
  void Update(const void* data, size_t len);
  // Returns the lowercase hex digest. Finalizes; do not Update afterwards.
  std::string HexFinal();

  static std::string Hex(const void* data, size_t len);

 private:
  void Transform(const uint8_t block[64]);
  uint32_t state_[8];
  uint64_t bitlen_;
  uint8_t buf_[64];
  size_t buflen_;
};

// Known-answer self-test (NIST vectors). Returns true if all vectors match.
bool Sha256SelfTest(std::string* detail);

}  // namespace p3
