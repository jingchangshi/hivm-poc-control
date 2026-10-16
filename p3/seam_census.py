#!/usr/bin/env python3
"""Seam census for the P3 harness.

The claim to audit is: "the host-only fault-injection self-test exercises the
SAME decision code the device path uses, not a copy of it."

That claim has two halves and each is checked from bytes, not asserted:

  (A) SOURCE: each shared decision function has exactly ONE definition, and it
      has at least one call site inside the device path AND at least one call
      site inside the self-test. A predicate called from only one of the two is
      not a shared seam, and a predicate with two definitions is a copy.

  (B) BINARY: the staged binary contains exactly one non-clone definition of
      each predicate symbol. One definition in the source plus one in the
      binary is what makes "same code" a measurement instead of a claim.

Usage: seam_census.py <p3_harness.cc> <staged_binary> <out.tsv>
Refuses to write the output unless every row validates.
"""
import re
import subprocess
import sys
from pathlib import Path

PREDICATES = [
    "JudgeTransfers",
    "JudgeCleanup",
    "JudgeStageGate",
    "BuildGateJson",
    "BuildRunJson",
    "CompareAll",
]
SELF_TEST = "RunFaultInjectionSelfTest"


def strip_comments_and_strings(src):
    """Blank out comments and string literals so a name inside either cannot be
    mistaken for a definition or a call. Replaces with spaces to keep line
    numbering and column offsets intact."""
    out = []
    i, n = 0, len(src)
    state = None  # None | 'line' | 'block' | 'str' | 'chr'
    while i < n:
        c = src[i]
        two = src[i:i + 2]
        if state is None:
            if two == "//":
                state = "line"
                out.append("  ")
                i += 2
                continue
            if two == "/*":
                state = "block"
                out.append("  ")
                i += 2
                continue
            if c == '"':
                state = "str"
                out.append(" ")
                i += 1
                continue
            if c == "'":
                state = "chr"
                out.append(" ")
                i += 1
                continue
            out.append(c)
            i += 1
            continue
        if state == "line":
            if c == "\n":
                state = None
                out.append("\n")
            else:
                out.append(" ")
            i += 1
            continue
        if state == "block":
            if two == "*/":
                state = None
                out.append("  ")
                i += 2
                continue
            out.append("\n" if c == "\n" else " ")
            i += 1
            continue
        # str / chr
        if c == "\\":
            out.append("  ")
            i += 2
            continue
        if (state == "str" and c == '"') or (state == "chr" and c == "'"):
            state = None
            out.append(" ")
            i += 1
            continue
        out.append("\n" if c == "\n" else " ")
        i += 1
    return "".join(out)


def function_extents(clean):
    """Returns {name: (start_line, end_line)} for every top-level-ish function
    definition found by brace matching from the first '{' after the name."""
    extents = {}
    for name in PREDICATES + [SELF_TEST]:
        # A definition: the name at the start of a declaration (not preceded by
        # '.' or '->'), followed by '(' ... ')' then '{' before any ';'.
        for m in re.finditer(r"(?<![\w.>])" + re.escape(name) + r"\s*\(", clean):
            start = clean.count("\n", 0, m.start()) + 1
            j = clean.find("(", m.start())
            depth = 0
            k = j
            while k < len(clean):
                if clean[k] == "(":
                    depth += 1
                elif clean[k] == ")":
                    depth -= 1
                    if depth == 0:
                        break
                k += 1
            tail = clean[k + 1:k + 200]
            if ";" in tail.split("{")[0]:
                continue  # a forward declaration, not a definition
            brace = clean.find("{", k)
            if brace == -1:
                continue
            depth = 0
            e = brace
            while e < len(clean):
                if clean[e] == "{":
                    depth += 1
                elif clean[e] == "}":
                    depth -= 1
                    if depth == 0:
                        break
                e += 1
            end = clean.count("\n", 0, e) + 1
            extents.setdefault(name, []).append((start, end))
    return extents


def calls_in_range(clean, name, lo, hi):
    """Line numbers within [lo,hi] where `name` is CALLED.

    A forward declaration is not a call. The first version of this tool counted
    one, which is a false positive: `bool CompareAll(...);` at file scope and
    `std::string BuildRunJson(...);` both matched, so the device-path column
    listed declarations as if they were call sites. Distinguish them by what
    precedes the name in its statement: a declaration has a type specifier
    before the name and nothing else (`bool `, `std::string `), whereas a call
    either starts the statement or is reached through an operator
    (`ReportLine(BuildRunJson(r))`)."""
    lines = []
    pat = re.compile(r"(?<![\w.>])" + re.escape(name) + r"\s*\(")
    for m in pat.finditer(clean):
        ln = clean.count("\n", 0, m.start()) + 1
        if not (lo <= ln <= hi):
            continue
        stmt_start = max(
            clean.rfind(";", 0, m.start()),
            clean.rfind("{", 0, m.start()),
            clean.rfind("}", 0, m.start()),
        )
        prefix = clean[stmt_start + 1:m.start()]
        if "(" in prefix or "=" in prefix or "," in prefix:
            lines.append(ln)  # an expression/call context, not a declaration
            continue
        if prefix.strip() == "":
            lines.append(ln)  # statement-initial call
            continue
        # Non-empty, no operators: a type specifier -> this is a declaration.
        # ...unless the prefix is a statement keyword, in which case the name is
        # being called: `return JudgeCleanup(ops);` is a call, not a declaration.
        if re.fullmatch(r"[A-Za-z_:<>*&\s]+", prefix) and not re.search(
            r"\b(return|co_return|throw|new|delete|case|if|while|for|switch|"
            r"else|do|sizeof|decltype|static_assert|noexcept)\b",
            prefix,
        ):
            continue
        lines.append(ln)
    return lines


def main():
    src_path, bin_path, out_path = sys.argv[1], sys.argv[2], sys.argv[3]
    raw = Path(src_path).read_text()
    clean = strip_comments_and_strings(raw)
    extents = function_extents(clean)

    st = extents.get(SELF_TEST)
    if not st:
        print("FAIL: self-test function not found", file=sys.stderr)
        return 1
    st_lo, st_hi = st[0]

    rows = []
    problems = []
    for name in PREDICATES:
        ex = extents.get(name, [])
        # The self-test's forward declaration is not a definition (no body), so
        # it is already excluded; a genuine duplicate body would show here.
        if len(ex) != 1:
            problems.append(f"{name}: {len(ex)} definitions in source (want 1)")
            continue
        lo, hi = ex[0]
        all_calls = calls_in_range(clean, name, 1, clean.count("\n") + 1)
        in_self = [c for c in all_calls if st_lo <= c <= st_hi]
        in_dev = [c for c in all_calls if not (st_lo <= c <= st_hi)]
        # The definition line itself matches the call pattern; drop it.
        in_dev = [c for c in in_dev if not (lo <= c <= hi)]
        if not in_dev:
            problems.append(f"{name}: no call site outside the self-test")
        if not in_self:
            problems.append(f"{name}: no call site inside the self-test")
        rows.append((name, lo, hi, in_dev, in_self))

    # (B) binary: one definition ORIGIN per predicate symbol.
    #
    # The literal to test here had to be measured, not assumed. Requiring one
    # non-clone definition is WRONG: gcc emits `[clone .isra.0]` / `[clone .cold]`
    # variants for some of these functions and no unsuffixed symbol at all
    # (CompareAll is the example in this build), so that test reports 0 and
    # refuses a correct binary. Every clone is emitted from ONE source
    # definition, so the sound question is "how many distinct BASE signatures
    # are there" after stripping the clone suffixes.
    #
    # LIMITATION, stated because it bounds what this half proves: a base-symbol
    # count cannot distinguish one definition from two identical ones that the
    # linker kept separately. It is the SOURCE half (exactly one definition) that
    # rules out a copy; this half only confirms the symbol the device path and
    # the self-test both call exists once in the staged binary.
    nm = subprocess.run(["nm", "-C", bin_path], capture_output=True, text=True)
    if nm.returncode != 0:
        print("FAIL: nm could not read the staged binary", file=sys.stderr)
        return 1

    def is_definition_of(sym, name):
        """True iff `sym` is the symbol FOR `name` itself, i.e. `name`'s own
        parameter list is the last thing in the symbol. A nested lambda defined
        inside `name` also contains `name(` but continues with
        `::{lambda...}::operator()` afterwards, so it is not a second
        definition of `name`."""
        m = re.search(r"(?<![\w])" + re.escape(name) + r"\s*\(", sym)
        if m is None:
            return False
        j = sym.index("(", m.start())
        depth = 0
        k = j
        while k < len(sym):
            if sym[k] == "(":
                depth += 1
            elif sym[k] == ")":
                depth -= 1
                if depth == 0:
                    break
            k += 1
        return sym[k + 1:].strip() == ""

    bin_bases = {}
    for line in nm.stdout.splitlines():
        if not re.match(r"^[0-9a-f]+\s+[tTwW]\s", line):
            continue
        parts = line.split(None, 2)
        sym = parts[2] if len(parts) > 2 else ""
        base = re.sub(r"(\s*\[clone[^\]]*\])+$", "", sym).strip()
        for name in PREDICATES:
            if is_definition_of(base, name):
                bin_bases.setdefault(name, set()).add(base)
    bin_counts = {k: len(v) for k, v in bin_bases.items()}
    for name, *_ in rows:
        c = bin_counts.get(name, 0)
        if c != 1:
            problems.append(f"{name}: {c} non-clone definitions in the binary (want 1)")

    if problems:
        print("SEAM_CENSUS FAIL -- refusing to write:", file=sys.stderr)
        for p in problems:
            print("  " + p, file=sys.stderr)
        return 1

    lines = ["# predicate\tdef_line\tdevice_call_lines\tselftest_call_lines\tbinary_defs"]
    for name, lo, hi, in_dev, in_self in rows:
        lines.append(
            f"{name}\t{lo}-{hi}\t{','.join(map(str, in_dev))}\t"
            f"{','.join(map(str, in_self))}\t{bin_counts.get(name, 0)}"
        )
    Path(out_path).write_text("\n".join(lines) + "\n")
    print(f"SEAM_CENSUS PASS: {len(rows)} predicates, each 1 source definition, "
          f"1 binary definition, and >=1 call site on BOTH sides")
    for line in lines[1:]:
        print("  " + line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
