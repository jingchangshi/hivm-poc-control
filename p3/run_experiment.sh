#!/usr/bin/env bash
# Bounded discriminating experiment for the P3-B device fault.
# Declared BEFORE running in .../impl/P3B-BOUNDED-EXPERIMENT.txt.
#
# Each variant: same frozen ELF, same grid=64, same dynamic UB=196608, device 0,
# one stream, one launch. Sync bound 45 s, process bound 90 s. Nothing is read
# after a failed sync; nothing is freed/unloaded/reset on that path.
#
# Usage: run_experiment.sh <harness> <kernel.o> <oracle> <out_dir>
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
  grep -oE '"event":"(launch_rc|sync_rc|sync_error_name|classification|loader_rc|lookup_rc)"[^}]*' \
    "$dir/report.jsonl" 2>/dev/null | sed 's/^/  /'
  # The device fault message, if any.
  if grep -q "aicore error" "$dir/acl.log" 2>/dev/null; then
    local cores
    cores="$(grep -oP 'core id is \K[0-9]+' "$dir/acl.log" | sort -n | uniq | tr '\n' ' ')"
    local pcs
    pcs="$(grep -oP 'current: 0x\K[0-9a-f]+' "$dir/acl.log" | sort -u | tr '\n' ' ')"
    echo "  DEVICE_FAULT cores=[$cores]"
    echo "  DEVICE_FAULT fault_pcs=[$pcs]"
    grep -oP 'errorStr: \K[^,]*' "$dir/acl.log" | sort -u | head -2 | sed 's/^/  /'
  fi
  if grep -q '"gate":"P3-B","result":"PASS"' "$dir/report.jsonl" 2>/dev/null; then
    echo "  P3-B PASS"
  fi
  echo
}

run_variant E0_control_frozen_config
run_variant E1_loader_magic_vector_core  --loader-magic=vector-core
run_variant E2_engine_type_aiv           --engine-type-aiv
run_variant E3_magic_and_engine_aiv      --loader-magic=vector-core --engine-type-aiv

echo "=== experiment complete; raw logs under $OUT ==="
