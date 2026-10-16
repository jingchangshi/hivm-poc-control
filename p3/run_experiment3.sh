#!/usr/bin/env bash
# The COMPLETE discriminating matrix, run on ONE binary generation so that every
# row shares the same instrument. experiment-r1 established the 2x2 on an earlier
# binary; experiment-r2/r3 added the wrong-magic controls. This run re-establishes
# the whole set on the shipped harness so no row is quoted across a rebuild.
#
#   E0  options=nullptr                     attrs={DYN_UBUF}            frozen config
#   E1  MAGIC=VECTOR_CORE 0x41415246        attrs={DYN_UBUF}
#   E2  options=nullptr                     attrs={DYN_UBUF,ENGINE=AIV}
#   E3  MAGIC=VECTOR_CORE 0x41415246        attrs={DYN_UBUF,ENGINE=AIV}
#   E4  MAGIC=AICORE      0x43554245        attrs={DYN_UBUF}            wrong value control
#   E5  MAGIC=CUBE        0x41494343        attrs={DYN_UBUF}            wrong value control
#
# Expected: E1 and E3 sync rc=0. E0/E2/E4/E5 fault identically at pc +0x108 on 28
# cores. If E4/E5 passed, the discriminating variable would be the mere PRESENCE
# of an options struct rather than the magic VALUE -- this matrix is what tells
# those two apart.
#
# Each variant: one launch, 45 s sync bound, 90 s process bound, device 0, one
# stream. Nothing is D2H'd/freed/unloaded/reset after a failed sync.
#
# Usage: run_experiment3.sh <harness> <kernel.o> <oracle> <out_dir>
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

  local cores_n pcs
  cores_n="$(grep -oP 'core id is \K[0-9]+' "$dir/acl.log" 2>/dev/null | sort -n | uniq | wc -l)"
  pcs="$(grep -oP 'current: 0x\K[0-9a-f]+' "$dir/acl.log" 2>/dev/null | sort -u | tr '\n' ' ')"
  printf 'exit=%s  loader_rc=%s  launch_rc=%s  sync_rc=%s  fault_cores=%s  pc=[%s]\n' \
    "$rc" \
    "$(grep -oP '"event":"loader_rc","value":\K[0-9-]+' "$dir/report.jsonl" 2>/dev/null)" \
    "$(grep -oP '"event":"launch_rc","value":\K[0-9-]+' "$dir/report.jsonl" 2>/dev/null)" \
    "$(grep -oP '"event":"sync_rc","value":\K[0-9-]+' "$dir/report.jsonl" 2>/dev/null)" \
    "${cores_n:-0}" "$pcs"
  if grep -q '"gate":"P3-B","result":"PASS"' "$dir/report.jsonl" 2>/dev/null; then
    echo "  P3-B PASS"
  fi
  echo
}

run_variant E0_frozen_config_nullptr_options
run_variant E1_magic_vector_core   --loader-magic=vector-core
run_variant E2_engine_type_aiv     --engine-type-aiv
run_variant E3_magic_and_engine    --loader-magic=vector-core --engine-type-aiv
run_variant E4_magic_aicore        --loader-magic=aicore
run_variant E5_magic_cube          --loader-magic=cube

echo "=== experiment 3 complete; raw logs under $OUT ==="
