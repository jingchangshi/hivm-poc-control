#!/usr/bin/env bash
# Build the P3 ACL harness. MUST be run inside the container via
# scripts/in-container.sh (the tree is a container build; a host compiler would
# produce objects the device side cannot use, and the CANN headers/libs are only
# present in the container).
#
# Usage: build_harness.sh <p3_src_dir> <out_dir> <frozen_criteria.txt>
set -euo pipefail

SRC="$1"
OUT="$2"
CRITERIA="$3"

: "${ASCEND_HOME_PATH:?ASCEND_HOME_PATH must be set (source cann set_env.sh)}"

CANN_LIB64="$ASCEND_HOME_PATH/x86_64-linux/lib64"
CANN_INC="$ASCEND_HOME_PATH/include"
ACL_SO="$CANN_LIB64/libascendcl.so"

echo "=== toolchain ==="
g++ --version | head -1
echo "ASCEND_HOME_PATH=$ASCEND_HOME_PATH"

echo "=== link library identity (FORMAL lib64, not devlib, not simulator) ==="
ls -la "$ACL_SO"
sha256sum "$ACL_SO"
stat -c '%s bytes' "$ACL_SO"

mkdir -p "$OUT"

# Generate the frozen-constant header FROM the frozen criteria file. Nothing in
# the C++ is hand-transcribed from the contract; the generator refuses to write
# if any required literal is missing or ambiguous.
echo "=== generating p3_frozen_constants.h from the frozen criteria ==="
python3 -I -B "$SRC/gen_frozen_constants.py" "$CRITERIA" "$SRC/p3_frozen_constants.h"
sha256sum "$SRC/p3_frozen_constants.h"

# -ffp-contract=off and no fast-math: CR-DATA-5/6 require one correctly-rounded
# RNE addition, so the compiler must not fuse or reassociate anything.
set -x
g++ -std=c++17 -O2 -Wall -Wextra -Werror \
    -ffp-contract=off -fno-fast-math -fno-unsafe-math-optimizations \
    -I"$SRC" -I"$CANN_INC" \
    "$SRC/p3_harness.cc" "$SRC/sha256.cc" "$SRC/acl_log.cc" \
    -o "$OUT/p3_harness" \
    -L"$CANN_LIB64" -lascendcl \
    -Wl,-rpath,"$CANN_LIB64" \
    -pthread
set +x

echo "=== built binary identity ==="
ls -la "$OUT/p3_harness"
sha256sum "$OUT/p3_harness"
stat -c '%s bytes' "$OUT/p3_harness"

echo "=== dynamic dependencies ==="
ldd "$OUT/p3_harness"

echo "=== resolved libascendcl (must be the lib64 copy) ==="
ldd "$OUT/p3_harness" | grep -i ascendcl || true
