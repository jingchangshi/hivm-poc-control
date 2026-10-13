#!/usr/bin/env bash
set -euo pipefail

REPO=/home/s00653124/workspace/tilelang
OLD="$REPO/tests/tests/golden/hivm/vadd_f32_single_tile"
NEW="$REPO/tests/golden/hivm/vadd_f32_single_tile"
EXPECTED_JSON=576fa7a37bcaf1748e387efbd26508652e2e4b18790fa21aea874a7b756c0fe5
EXPECTED_TIR=4194733b8412484c9bb8caf74ba3285348345f92ad61684784fe8c3a79e12a18

for DIR in "$OLD" "$NEW"; do
  [ -e "$DIR" ] || continue
  printf 'Directory: %s
' "$DIR"
  sha256sum "$DIR/golden_codegen_input.json" "$DIR/golden_codegen_input.tir"
  printf '%s  %s
' "$EXPECTED_JSON" "$DIR/golden_codegen_input.json" | sha256sum -c -
  printf '%s  %s
' "$EXPECTED_TIR" "$DIR/golden_codegen_input.tir" | sha256sum -c -
  python3 - "$DIR/manifest.json" "$EXPECTED_JSON" <<'PY2'
import json,sys
with open(sys.argv[1],encoding='utf-8') as fh:
    d=json.load(fh)
assert d['golden_sha256']==sys.argv[2], 'manifest JSON SHA mismatch'
assert d['ub_offsets_f32']=={'a':0,'b':16384,'c':32768}, 'bad element-offset contract'
print('Manifest invariants: PASS')
PY2
done

git -C "$REPO" status --short
git -C "$REPO" diff --cached --name-status
