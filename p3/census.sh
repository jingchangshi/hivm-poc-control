#!/usr/bin/env bash
# Host-side staging census for the P3-B escalation. NO DEVICE ACCESS, NO BUILD.
#
# Emits one row per file: sha256 <TAB> bytes <TAB> type <TAB> path, over the
# enumerated staging set, then prints counts. Refuses to write if it enumerates
# zero rows. The census file does NOT contain its own hash (a living artifact
# cannot carry its own hash); its sha256 is recorded externally.
#
# Usage: census.sh <out_tsv> <dir> [dir...]
set -uo pipefail

OUT="$1"; shift
if [ "$#" -lt 1 ]; then
  echo "usage: census.sh <out_tsv> <dir> [dir...]" >&2
  exit 2
fi

TMP="$(mktemp)"
: > "$TMP"

for d in "$@"; do
  if [ ! -d "$d" ]; then
    printf 'MISSING_DIR\t\t\t%s\n' "$d" >> "$TMP"
    continue
  fi
  # -print0/sort -z so a filename with a space cannot split a row.
  find "$d" -mindepth 1 -print0 | sort -z | while IFS= read -r -d '' p; do
    if [ -L "$p" ]; then
      printf 'SYMLINK\t0\t%s\n' "$p" >> "$TMP"
    elif [ -d "$p" ]; then
      printf 'DIR\t0\t%s\n' "$p" >> "$TMP"
    else
      h="$(sha256sum "$p" | cut -d' ' -f1)"
      s="$(stat -c '%s' "$p")"
      n="$(stat -c '%h' "$p")"
      printf 'FILE\t%s\t%s\tlinks=%s\t%s\n' "$s" "$h" "$n" "$p" >> "$TMP"
    fi
  done
done

ROWS="$(wc -l < "$TMP")"
FILES="$(awk -F'\t' '$1=="FILE"' "$TMP" | wc -l)"
if [ "$ROWS" = "0" ] || [ "$FILES" = "0" ]; then
  echo "REFUSING TO WRITE: enumerated $ROWS rows, $FILES files" >&2
  rm -f "$TMP"
  exit 3
fi

{
  echo "# P3 staging census. sha256 is over file CONTENT (not git SHA-1)."
  echo "# columns: kind <TAB> bytes <TAB> sha256 <TAB> nlink <TAB> path"
  cat "$TMP"
} > "$OUT"

rm -f "$TMP"

echo "rows=$(wc -l < "$OUT") files=$FILES"
echo "distinct_content_hashes=$(awk -F'\t' '$1=="FILE"{print $3}' "$OUT" | sort -u | wc -l)"
echo "distinct_paths=$(awk -F'\t' '$1=="FILE"{print $5}' "$OUT" | sort -u | wc -l)"
echo "total_bytes=$(awk -F'\t' '$1=="FILE"{s+=$2} END{print s+0}' "$OUT")"
echo "census_written=$OUT"
sha256sum "$OUT"
