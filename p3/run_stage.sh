#!/usr/bin/env bash
# Run one P3 stage on the real device.
#
# Serializes NPU access: this script is the only entry point for device work and
# it records the device state before and after. Device 0 only (goal 5.1);
# device 1 belongs to another user and is never touched.
#
# Usage: run_stage.sh <stage A|B|C> <harness_bin> <kernel.o> <oracle_dir> <run_dir>
set -uo pipefail

STAGE="$1"
HARNESS="$2"
KERNEL="$3"
ORACLE="$4"
RUNDIR="$5"

mkdir -p "$RUNDIR"

echo "=== device state BEFORE (read-only; no reset, no other user's processes) ==="
npu-smi info 2>&1 | sed -n '1,30p' | tee "$RUNDIR/npu-smi.before.txt"

# Idleness assertion on device 0. A busy device 0 means ENV_BLOCKED, not a run.
BEFORE_UTIL="$(npu-smi info -t usages -i 0 2>/dev/null | grep -i 'AICore\|Util' | head -3 || true)"
echo "$BEFORE_UTIL" > "$RUNDIR/npu0-util.before.txt"

echo "=== stage $STAGE: harness identity ==="
sha256sum "$HARNESS" | tee "$RUNDIR/harness.sha256"
stat -c '%s bytes %n' "$HARNESS" | tee -a "$RUNDIR/harness.sha256"

echo "=== frozen binary identity at run time ==="
sha256sum "$KERNEL" | tee "$RUNDIR/kernel.sha256"

START_UTC="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "start_utc=$START_UTC" | tee "$RUNDIR/timing.txt"

REPORT="$RUNDIR/report.jsonl"
ACL_LOG="$RUNDIR/acl.log"
rm -f "$REPORT" "$ACL_LOG"

# The outer watchdog mirrors the in-process one: a wedged ACL call must not hang
# the session. SIGTERM then SIGKILL after a short grace.
set +e
timeout --signal=TERM --kill-after=5 150 \
  "$HARNESS" --stage="$STAGE" --binary="$KERNEL" --oracle-dir="$ORACLE" \
             --report="$REPORT" --log="$ACL_LOG" \
  >"$RUNDIR/stdout.txt" 2>"$RUNDIR/stderr.txt"
RC=$?
set -e

END_UTC="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "end_utc=$END_UTC" | tee -a "$RUNDIR/timing.txt"
echo "exit_code=$RC" | tee -a "$RUNDIR/timing.txt"

echo "=== stdout ==="
cat "$RUNDIR/stdout.txt"
echo "=== stderr ==="
cat "$RUNDIR/stderr.txt"

echo "=== device state AFTER ==="
npu-smi info 2>&1 | sed -n '1,30p' | tee "$RUNDIR/npu-smi.after.txt"

if [ "$RC" = "124" ] || [ "$RC" = "137" ]; then
  echo "STAGE $STAGE RESULT: TIMEOUT (outer watchdog)"
elif [ "$RC" = "0" ]; then
  echo "STAGE $STAGE RESULT: COMPLETE rc=0"
else
  echo "STAGE $STAGE RESULT: FAILED exit_code=$RC (see report.jsonl classification)"
fi
exit "$RC"
