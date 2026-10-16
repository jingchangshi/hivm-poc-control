#!/usr/bin/env python3
"""Generate the r9 mutant set: each mutant is the r9 source with EXACTLY ONE
substitution, asserted to occur exactly ONCE before writing.

Run with -I -B (no bytecode into the tree). Writes nothing unless every
substitution in every mutant is unique in the pristine source.
"""
import sys
import os

SRC = sys.argv[1]          # pristine r9 p3_harness.cc
OUT = sys.argv[2]          # output dir

MUTANTS = [
    ("mutant1",
     "JudgeTransfers ignores a failed transfer",
     "  if (bad != nullptr) {",
     "  if (false) {"),
    ("mutant2",
     "JudgeCleanup ignores a failed release op",
     "    if (op.second != ACL_SUCCESS) {",
     "    if (false) {"),
    ("mutant3",
     "BuildRunJson always emits PASS",
     '    s += std::string(r.passed ? "\\"PASS\\"" : "\\"FAIL\\"");',
     '    s += "\\"PASS\\"";'),
    ("mutant4",
     "stage-B record reported as attempted",
     "  const bool attempted = r.comparison_attempted;",
     "  const bool attempted = true;"),
    ("mutant5",
     "the stage gate drops the release operand",
     "  v.pass = work_ok && cv.ok;",
     "  v.pass = work_ok;"),
    ("mutant6",
     "a gate row with no release verdict says PASS",
     "  if (!gv.cleanup_judged) {",
     "  if (false) {"),
    ("mutant7",
     "CompareAll always sets passed=true",
     "  r->passed = (r->mismatches == 0) && (r->sentinel_residue == 0) &&\n"
     "              (r->nan_or_inf == 0) && r->a_unmodified && r->b_unmodified &&\n"
     "              r->guards_ok;",
     "  r->passed = true;"),
    ("mutant8",
     "preservation claimed measured despite A/B D2H failure",
     "    v.preservation_measured = (d2h_a == ACL_SUCCESS) && (d2h_b == ACL_SUCCESS);",
     "    v.preservation_measured = true;"),
    # The anchor must include the body line: the bare condition is a SUBSTRING
    # of the more-indented copies inside the run loop (a 4-space copy contains
    # the 2-space text), so the bare literal occurs four times and the generator
    # refuses. The two-line anchor with its distinctive body comment is unique.
    ("mutant9",
     "CompareAll does not refuse an inadmissible run",
     "  if (!tv.admissible) {\n"
     "    r->compared_elements = 0;  // NOT_EVALUATED: no reading was taken",
     "  if (false) {\n"
     "    r->compared_elements = 0;  // NOT_EVALUATED: no reading was taken"),
    # NEW in r9. This is the mutation the Challenger named as surviving by
    # inspection because every injected gate case passed work_ok=true. It drops
    # the WORK operand instead of the release operand, so a stage gate would say
    # PASS after a numerical failure with a clean release.
    ("mutant10",
     "the stage gate drops the WORK operand",
     "  v.pass = work_ok && cv.ok;",
     "  v.pass = cv.ok;"),
]

with open(SRC, "rb") as f:
    base = f.read().decode()

fail = []
for name, desc, old, new in MUTANTS:
    n = base.count(old)
    if n != 1:
        fail.append("%s (%s): the substitution literal occurs %d times, want 1"
                    % (name, desc, n))
    if new in base:
        fail.append("%s (%s): the mutant text is ALREADY in the source"
                    % (name, desc))

if fail:
    for m in fail:
        sys.stderr.write("REFUSING TO WRITE: " + m + "\n")
    sys.exit(1)

os.makedirs(OUT, exist_ok=True)
man = []
for name, desc, old, new in MUTANTS:
    text = base.replace(old, new, 1)
    path = os.path.join(OUT, "p3_harness_%s.cc" % name)
    with open(path, "w") as f:
        f.write(text)
    man.append("%s\t%s\n  - %s\n  + %s\n"
               % (name, desc, old.replace("\n", "\\n"), new.replace("\n", "\\n")))

with open(os.path.join(OUT, "MUTANTS.txt"), "w") as f:
    f.write("r9 mutant set. Base source: %s\n" % SRC)
    f.write("Each mutant differs from the base by EXACTLY ONE substitution,\n")
    f.write("asserted to occur exactly once in the base before writing.\n\n")
    f.writelines(man)
print("wrote %d mutants to %s" % (len(MUTANTS), OUT))
