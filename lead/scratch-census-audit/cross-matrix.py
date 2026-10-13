import ast, importlib.util, pathlib, sys, hashlib
EV=pathlib.Path("/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline")
R=EV/"verify/tests/scripts/run_p2_suite_v3.py"; ARCH=EV/"verify/work/p2b-acceptance/logs-suite-p2b"
spec=importlib.util.spec_from_file_location("v3",R); m=importlib.util.module_from_spec(spec); sys.modules["v3"]=m
spec.loader.exec_module(m)
src=R.read_text(); tree=ast.parse(src)
# map lineno -> enclosing def name
funcs=[]
for n in ast.walk(tree):
    if isinstance(n,(ast.FunctionDef,)): funcs.append((n.lineno,n.end_lineno,n.name))
def owner(ln):
    best=None
    for a,b,nm in funcs:
        if a<=ln<=b and (best is None or a>best[0]): best=(a,nm)
    return best[1] if best else "?"
def lit(a): return isinstance(a,(ast.List,ast.Tuple)) and a.elts and all(isinstance(e,ast.Constant) and isinstance(e.value,str) for e in a.elts)
kl=[]
for n in ast.walk(tree):
    if isinstance(n,ast.Call):
        fn=n.func.id if isinstance(n.func,ast.Name) else (n.func.attr if isinstance(n.func,ast.Attribute) else None)
        if fn in ("_expect_reject","reason_check"):
            for a in list(n.args)+[k.value for k in n.keywords]:
                if lit(a): kl.append((n.lineno,owner(n.lineno),[e.value for e in a.elts]))
streams=sorted(ARCH.glob("*.stderr"))
print(f"runner {hashlib.sha256(R.read_bytes()).hexdigest()[:16]}  {len(kl)} key-lists  {len(streams)} streams\n")
print("=== would row X's key-list be SATISFIED by stream Y? (rc!=0 assumed) ===")
for ln,own,keys in sorted(kl):
    acc=[]
    for p in streams:
        txt=p.read_text(errors="replace")
        if m.stub_terminal(txt,""): continue           # matcher rejects the stub first
        h=m.primary_diagnostic(txt,"",keys)
        if h: acc.append(p.name.split(".")[0])
    foreign=[a for a in acc if a!=own]
    tag="SELF-ONLY" if acc and not foreign else ("NO-STREAM" if not acc else "CROSS")
    print(f"  {own:16} (: {ln})  keys={keys}")
    print(f"       accepts {len(acc):2d} streams; foreign={foreign}   [{tag}]")
