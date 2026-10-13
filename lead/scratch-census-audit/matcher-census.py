import ast, importlib.util, pathlib, re, sys, hashlib
EV=pathlib.Path("/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline")
R=V3=EV/"verify/tests/scripts/run_p2_suite_v3.py"
ARCH=EV/"verify/work/p2b-acceptance/logs-suite-p2b"
spec=importlib.util.spec_from_file_location("v3",R); m=importlib.util.module_from_spec(spec)
sys.modules["v3"]=m
try:
    spec.loader.exec_module(m)
    print("imported runner module OK (no main side effect)")
except SystemExit as e:
    print("import raised SystemExit",e)
print("has primary_diagnostic:",hasattr(m,"primary_diagnostic"),"classify_lines:",hasattr(m,"classify_lines"))
print("runner sha",hashlib.sha256(R.read_bytes()).hexdigest()[:16],"\n")

# extract keylists via ast (no transcription)
tree=ast.parse(R.read_text()); keylists=[]
def lit(a): return isinstance(a,(ast.List,ast.Tuple)) and a.elts and all(isinstance(e,ast.Constant) and isinstance(e.value,str) for e in a.elts)
for n in ast.walk(tree):
    if isinstance(n,ast.Call):
        fn=n.func.id if isinstance(n.func,ast.Name) else (n.func.attr if isinstance(n.func,ast.Attribute) else None)
        if fn in ("_expect_reject","reason_check"):
            for a in list(n.args)+[k.value for k in n.keywords]:
                if lit(a): keylists.append((n.lineno,fn,[e.value for e in a.elts]))
    if isinstance(n,ast.Assign) and isinstance(n.value,(ast.List,ast.Tuple)):
        names=[t.id for t in n.targets if isinstance(t,ast.Name)]
        if any(x.endswith("KEYS") for x in names) and n.value.elts and all(isinstance(e,ast.Constant) and isinstance(e.value,str) for e in n.value.elts):
            keylists.append((n.lineno,names[0],[e.value for e in n.value.elts]))

allkeys=sorted({k for _,_,ks in keylists for k in ks})
streams=sorted(ARCH.glob("*.stderr"))
print(f"{len(streams)} archived streams; {len(allkeys)} distinct keys\n")

# MATCHER-ACCURATE: a key is 'free' only if it appears in a DIAG-kind line
diaglines={}
for p in streams:
    txt=p.read_text(errors="replace")
    dl=[]
    for where,n,line,kind in m.classify_lines(txt,""):
        if kind!="diag": continue
        s=line.strip(); mo=m.DIAG_LINE_RE.match(s); dl.append(mo.group(1) if mo else s)
    if dl: diaglines[p.name]=dl
print(f"streams with >=1 diag line: {len(diaglines)}")

print("\n=== keys free on ANOTHER stream's diag surface (matcher-accurate false-green risk) ===")
flagged={}
for pname,dl in diaglines.items():
    owner=pname.split(".")[0]
    for k in allkeys:
        if any(k.lower() in l.lower() for l in dl):
            flagged.setdefault(k,set()).add(owner)
for k in allkeys:
    if k in flagged:
        print(f"  key {k!r:24} appears in diag surface of: {sorted(flagged[k])}")
print("\n-- keys NEVER on any diag surface (max discrimination) --")
print("  ", [k for k in allkeys if k not in flagged])
