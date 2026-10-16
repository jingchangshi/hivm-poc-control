#!/usr/bin/env bash
# Value-specificity control for the loader magic, and the fix-confirmation rerun.
#
# The E0-E3 matrix (experiment-r1) showed the loader MAGIC option discriminates:
# options=nullptr faults, MAGIC=VECTOR_CORE(0x41415246) passes. That leaves one
# gap: is the discriminating variable the magic VALUE, or merely the PRESENCE of
# an options struct? E4/E5 close it by sending the SAME option type carrying the
# WRONG core type for this AIV image.
#
#   E4  options=MAGIC(AICORE 0x43554245)  -- wrong core type, control
#   E5  options=MAGIC(CUBE   0x41494343)  -- wrong core type, control
#   E1r options=MAGIC(VECTOR_CORE)        -- rerun of the accepted config on the
#                                            rebuilt binary (tri()/compared_elements)
#
# Expected under "the VALUE is the discriminator": E4/E5 do NOT reproduce E1's
# clean sync -- either the load itself fails, or the launch faults as in E0.
# Each variant: one launch, 45 s sync bound, 90 s process bound, device 0, one
# stream, nothing freed/unloaded/reset after a failed sync.
#
# Usage: run_experiment2.sh <harness> <kernel.o> <oracle> <out_dir>
set -uo pipefail

HARNESS="$1"
KERNEL="$2"
ORACLE="$3"
OUT="$4"
mkdir -p "$OUT"

run_variant() {
  local name="$1"; shift
  local dir="$OUT/$name"
  mkdir -p "$dir"
  echo "########## VARIANT $name : $* ##########"
  echo "$HARNESS --stage=B --binary=$KERNEL --oracle-dir=$ORACLE $*" > "$dir/command.txt"
  sha256sum "$HARNESS" > "$dir/harness.sha256"
  sha256sum "$KERNEL"  > "$dir/kernel.sha256"
  date -u +%Y-%m-%dT%H:%M:%SZ > "$dir/timing.txt"

  timeout --signal=TERM --kill-after=5 90 \
    "$HARNESS" --stage=B --binary="$KERNEL" --oracle-dir="$ORACLE" \
               --report="$dir/report.jsonl" --log="$dir/acl.log" "$@" \
    >"$dir/stdout.txt" 2>"$dir/stderr.txt"
  local rc=$?
  date -u +%Y-%m-%dT%H:%M:%SZ >> "$dir/timing.txt"
  echo "exit_code=$rc" >> "$dir/timing.txt"

  echo "  exit=$rc"
  grep -oE '"event":"(loader_rc|lookup_rc|launch_rc|sync_rc|sync_error_name|classification)"[^}]*' \
    "$dir/report.jsonl" 2>/dev/null | sed 's/^/  /'
  grep -oE '"event":"loader_options"[^}]*' "$dir/report.jsonl" 2>/dev/null | head -1 | sed 's/^/  /'
  if grep -q "aicore error" "$dir/acl.log" 2>/dev/null; then
    local cores pcs
    cores="$(grep -oP 'core id is \K[0-9]+' "$dir/acl.log" | sort -n | uniq | tr '\n' ' ')"
    pcs="$(grep -oP 'current: 0x\K[0-9a-f]+' "$dir/acl.log" | sort -u | tr '\n' ' ')"
    echo "  DEVICE_FAULT cores=[$cores]"
    echo "  DEVICE_FAULT fault_pcs=[$pcs]"
  fi
  # A launch-stage PASS is emitted only by the P3-B gate line, never the P3-A one.
  if grep -q '"gate":"P3-B","result":"PASS"' "$dir/report.jsonl" 2>/dev/null; then
    echo "  P3-B PASS"
  fi
  echo
}

run_variant E4_magic_aicore       --loader-magic=aicore
run_variant E5_magic_cube         --loader-magic=cube
run_variant E1r_magic_vector_core --loader-magic=vector-core

echo "=== experiment 2 complete; raw logs under $OUT ==="
