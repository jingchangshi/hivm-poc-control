#!/usr/bin/env bash
# Materialize the Verifier's frozen oracle payloads as files the harness reads.
#
# The harness NEVER computes the reference; it reads these bytes and binds each
# one to the CR-DATA-7 digest. This script is the only place the oracle is
# invoked, and it re-verifies every digest against the frozen criteria table.
#
# Usage: materialize_oracle.sh <oracle.py> <criteria.txt> <out_dir> <identities.tsv>
set -euo pipefail

ORACLE="$1"
CRITERIA="$2"
OUTDIR="$3"
IDENT_TSV="$4"

PY="$(command -v python3)"

mkdir -p "$OUTDIR"

# The case list comes from the criteria file's CR-DATA-7 table, in file order,
# so a case added to the contract cannot be silently skipped here.
CASES="$("$PY" -I -B - "$CRITERIA" <<'PYEOF'
import re, sys
text = open(sys.argv[1], encoding='utf-8').read()
rows = re.findall(r'^(\S+)\t(\S+)\t([0-9a-f]{64})\t(\d+)\s*$', text, re.M)
order = []
for case, slug, _sha, _n in rows:
    if case not in order:
        order.append(case)
print('\n'.join(order))
PYEOF
)"

if [ -z "$CASES" ]; then
  echo "FATAL: no cases parsed from $CRITERIA" >&2
  exit 1
fi

echo "cases: $(echo "$CASES" | tr '\n' ' ')"

while IFS= read -r case; do
  [ -z "$case" ] && continue
  # gen -> A payload || B payload (2 x 2097152)
  "$PY" -I -B "$ORACLE" gen "$case" > "$OUTDIR/$case.A+B.bin"
  A_SIZE=$(stat -c %s "$OUTDIR/$case.A+B.bin")
  if [ "$A_SIZE" != "4194304" ]; then
    echo "FATAL: $case gen produced $A_SIZE bytes, expected 4194304" >&2
    exit 1
  fi
  head -c 2097152 "$OUTDIR/$case.A+B.bin" > "$OUTDIR/$case.A.bin"
  tail -c 2097152 "$OUTDIR/$case.A+B.bin" > "$OUTDIR/$case.B.bin"
  rm -f "$OUTDIR/$case.A+B.bin"
  "$PY" -I -B "$ORACLE" ref "$case" > "$OUTDIR/$case.C_ref.bin"
done <<< "$CASES"

# Bind every file to the frozen digest table. Any mismatch is a hard stop.
"$PY" -I -B - "$CRITERIA" "$OUTDIR" <<'PYEOF'
import hashlib, os, re, sys
text = open(sys.argv[1], encoding='utf-8').read()
outdir = sys.argv[2]
rows = re.findall(r'^(\S+)\t(\S+)\t([0-9a-f]{64})\t(\d+)\s*$', text, re.M)
bad = 0
for case, slug, sha, nbytes in rows:
    path = os.path.join(outdir, '%s.%s.bin' % (case, slug))
    if not os.path.exists(path):
        print('MISSING %s' % path)
        bad += 1
        continue
    data = open(path, 'rb').read()
    got = hashlib.sha256(data).hexdigest()
    ok = (got == sha) and (len(data) == int(nbytes))
    print('%s %s %s %d' % ('OK ' if ok else 'BAD', case, slug, len(data)))
    if not ok:
        print('    got  %s %d' % (got, len(data)))
        print('    want %s %s' % (sha, nbytes))
        bad += 1
print('TOTAL_ROWS %d BAD %d' % (len(rows), bad))
sys.exit(1 if bad else 0)
PYEOF

# Record the identity table actually used, next to the bytes.
cp -f "$IDENT_TSV" "$OUTDIR/IDENTITIES.tsv"
echo "materialized into $OUTDIR"
