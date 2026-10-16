#!/usr/bin/env bash
# Negative controls for the P3 harness. Each control must FAIL in the specific
# way named, otherwise the corresponding guard is not real coverage.
#
# Run INSIDE the container, before any device work. Controls 1-3 never reach the
# device; control 4 is the pre-device oracle preflight.
#
# Usage: negative_controls.sh <harness_bin> <kernel.o> <oracle_dir> <out_dir>
set -uo pipefail

HARNESS="$1"
KERNEL="$2"
ORACLE="$3"
OUT="$4"

mkdir -p "$OUT"
PASS=0
FAIL=0

run_control() {
  local name="$1" expect_token="$2" expect_exit="$3"; shift 3
  local rep="$OUT/$name.report.jsonl" log="$OUT/$name.acl.log"
  rm -f "$rep" "$log"
  "$HARNESS" "$@" --report="$rep" --log="$log" >"$OUT/$name.stdout" 2>"$OUT/$name.stderr"
  local rc=$?
  local tok=0
  if [ -f "$rep" ] && grep -q "$expect_token" "$rep"; then tok=1; fi
  if [ "$rc" = "$expect_exit" ] && [ "$tok" = "1" ]; then
    echo "NEGATIVE_CONTROL PASS: $name (exit=$rc token=$expect_token found)"
    PASS=$((PASS+1))
  else
    echo "NEGATIVE_CONTROL FAIL: $name (exit=$rc want=$expect_exit; token=$expect_token found=$tok)"
    FAIL=$((FAIL+1))
  fi
}

# --- control 1: a corrupted copy of the binary must be BINARY_IDENTITY_BLOCKED
#     and must NOT reach aclInit.
TAMPER="$OUT/tampered_kernel.o"
cp "$KERNEL" "$TAMPER"
printf '\x01' | dd of="$TAMPER" bs=1 seek=0 conv=notrunc status=none
run_control "tampered_binary" "BINARY_IDENTITY_BLOCKED" 20 \
  --stage=A --binary="$TAMPER" --oracle-dir="$ORACLE"

# The control is only meaningful if the tampered file really differs.
if cmp -s "$KERNEL" "$TAMPER"; then
  echo "NEGATIVE_CONTROL FAIL: tampered copy is byte-identical to the original"
  FAIL=$((FAIL+1))
else
  echo "  (tampered copy differs from the frozen binary as intended)"
fi

# --- control 2: a truncated copy must also be BINARY_IDENTITY_BLOCKED --------
TRUNC="$OUT/truncated_kernel.o"
head -c 4447 "$KERNEL" > "$TRUNC"
run_control "truncated_binary" "BINARY_IDENTITY_BLOCKED" 20 \
  --stage=A --binary="$TRUNC" --oracle-dir="$ORACLE"

# --- control 3: a nonexistent path must be BINARY_IDENTITY_BLOCKED ----------
run_control "missing_binary" "BINARY_IDENTITY_BLOCKED" 20 \
  --stage=A --binary="$OUT/does_not_exist.o" --oracle-dir="$ORACLE"

# --- control 4: a corrupted ORACLE payload must trip the CR-DATA-7 preflight
#     before any device work.
BADORACLE="$OUT/bad_oracle"
rm -rf "$BADORACLE"; mkdir -p "$BADORACLE"
cp "$ORACLE"/*.bin "$BADORACLE"/ 2>/dev/null
# Flip one byte inside the mixed A payload.
printf '\xff' | dd of="$BADORACLE/mixed.A.bin" bs=1 seek=100 conv=notrunc status=none
run_control "corrupted_oracle" "ORACLE_MISMATCH" 27 \
  --stage=C --binary="$KERNEL" --oracle-dir="$BADORACLE"

# --- control 5: the SHA-256 self-test must pass (it is the basis of CR-ID-2) -
if "$HARNESS" --selftest --report="$OUT/selftest.jsonl" --log="$OUT/selftest.acl.log" \
     >"$OUT/selftest.stdout" 2>&1; then
  if grep -q "SHA256_SELFTEST PASS" "$OUT/selftest.stdout"; then
    echo "NEGATIVE_CONTROL PASS: sha256_selftest (KATs verified)"
    PASS=$((PASS+1))
  else
    echo "NEGATIVE_CONTROL FAIL: sha256_selftest (no PASS line)"
    FAIL=$((FAIL+1))
  fi
else
  echo "NEGATIVE_CONTROL FAIL: sha256_selftest (nonzero exit)"
  FAIL=$((FAIL+1))
fi

echo "NEGATIVE_CONTROLS TOTAL pass=$PASS fail=$FAIL"
[ "$FAIL" = "0" ]
