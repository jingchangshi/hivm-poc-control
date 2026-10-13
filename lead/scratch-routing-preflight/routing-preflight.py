#!/usr/bin/env python3
"""Lead pre-seal preflight for v3's routing pair (D074 window, 2026-10-14).

Exercises the routing matcher literals against the REAL r1 binary's own marker
strings and against the REAL archived routing-pair streams.  Nothing is
retyped: the patterns come from the runner's AST, the marker lines from
strings(1) on the staged binary, the streams from the archived run directory.
"""
import ast, hashlib, importlib.util, pathlib, re, subprocess, sys

EV = pathlib.Path("/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline")
V3 = EV / "verify/tests/scripts/run_p2_suite_v3.py"
R1 = EV / "impl/stage-p2b-r1/bin/bishengir-compile"
R1OPT = EV / "impl/stage-p2b-r1/bin/bishengir-opt"
PAIR = EV / "lead/scratch/routing-pair/run-20261013T202128Z"
STUB = EV / "verify/work/p2b-acceptance/logs-suite-p2b/A13.env.stderr"

src = V3.read_text(encoding="utf-8")
tree = ast.parse(src)
v3sha = hashlib.sha256(V3.read_bytes()).hexdigest()

# --- AST-extract the routing literals (no transcription) ---
late_pat = None
ord_pats = {}
for node in ast.walk(tree):
    if isinstance(node, ast.Assign) and len(node.targets) == 1 and isinstance(node.targets[0], ast.Name):
        name = node.targets[0].id
        if name == "LATE_MARKER_RE" and isinstance(node.value, ast.Call):
            late_pat = node.value.args[0].value
        if name == "ORD_MARKER_RES" and isinstance(node.value, ast.Dict):
            for k, v in zip(node.value.keys, node.value.values):
                ord_pats[k.value] = v.args[0].value
assert late_pat and ord_pats, (late_pat, ord_pats)

# --- real marker strings from the staged binaries ---
def marker_lines(binpath):
    s = subprocess.run(["strings", "-a", str(binpath)], capture_output=True, text=True, check=True).stdout
    return sorted({ln for ln in s.splitlines() if "[P2-BOUNDARY]" in ln})

r1c, r1o = marker_lines(R1), marker_lines(R1OPT)

out = []
def p(s=""): out.append(s)

p("Lead routing-pair PREFLIGHT (before the v3 seal) -- 2026-10-14")
p(f"runner: {V3}")
p(f"runner sha256: {v3sha}")
p("extraction: ast (LATE_MARKER_RE / ORD_MARKER_RES string constants); strings(1) for the")
p("  binary's own marker lines; archived streams read as bytes. Nothing retyped.")
p()
p(f"stage-p2b-r1/bin/bishengir-compile  {hashlib.sha256(R1.read_bytes()).hexdigest()}")
p(f"stage-p2b-r1/bin/bishengir-opt      {hashlib.sha256(R1OPT.read_bytes()).hexdigest()}")
p()
p("=== literals under test (AST-extracted) ===")
p(f"  LATE_MARKER_RE  = {late_pat!r}")
for k, v in ord_pats.items():
    p(f"  ORD[{k}] = {v!r}")
p()
p("=== matrix: does each pattern match each REAL marker line? ===")
allpat = [("LATE", re.compile(late_pat))] + [(k, re.compile(v)) for k, v in ord_pats.items()]
for label, lines in (("stage-p2b-r1/compile", r1c), ("stage-p2b-r1/opt", r1o)):
    p(f"  -- {label}: {len(lines)} distinct [P2-BOUNDARY] string(s)")
    for ln in lines:
        hits = [n for n, rx in allpat if rx.search(ln)]
        p(f"     {ln}")
        p(f"        <- matched by: {hits if hits else 'NONE'}")
    p()
unmatched = [ln for ln in r1c if not any(rx.search(ln) for _, rx in allpat)]
p(f"  marker strings in the COMPILER matched by NO routing pattern: {len(unmatched)}")
for ln in unmatched:
    p(f"     (unmatched, by design? ) {ln}")
p()

# --- run the runner's OWN functions on the REAL archived streams ---
sys.path.insert(0, str(V3.parent))
spec = importlib.util.spec_from_file_location("v3mod", V3)
v3 = importlib.util.module_from_spec(spec); spec.loader.exec_module(v3)
R1SHA = hashlib.sha256(R1.read_bytes()).hexdigest()

def rd(tag, d, expect):
    err = (d / "stderr").read_text(encoding="utf-8", errors="replace")
    outp = (d / "stdout").read_text(encoding="utf-8", errors="replace")
    inst, late, ords = v3.routing_scan(err, outp)
    st, det = v3.routing_detail(err, outp, R1SHA, expect)
    p(f"  [{tag}] expect={expect!r} instrumented={inst} late_hits={len(late)} ord_names={[n for n,_ in ords]}")
    p(f"      -> status={st!r}  detail={det}")
    return st, det

p("=== the runner's OWN routing_scan/routing_detail on the archived REAL streams ===")
p(f"  streams: {PAIR}")
if PAIR.is_dir():
    rd("ordinary run", PAIR / "ordinary", "ord")
    rd("late run", PAIR / "late", "late")
else:
    p("  PAIR DIR MISSING -- measurement not taken")
p()
p("=== negative control: the 223 B markerless P2-B stub stream ===")
if STUB.is_file():
    err = STUB.read_text(encoding="utf-8", errors="replace")
    p(f"  {STUB.name}: {len(err.encode())} B, {len(err.splitlines())} line(s)")
    rd("stub (must NOT be instrumented)", STUB.parent, "late") if False else None
    inst, late, ords = v3.routing_scan(err, "")
    st, det = v3.routing_detail(err, "", R1SHA, "ord")
    p(f"  instrumented={inst} late={len(late)} ord={[n for n,_ in ords]}")
    p(f"      -> status={st!r}  detail={det}")
p()
p("END preflight")
text = "\n".join(out) + "\n"
(Dpath := pathlib.Path(__file__).with_name("routing-preflight.txt")).write_text(text, encoding="utf-8")
print(text)
