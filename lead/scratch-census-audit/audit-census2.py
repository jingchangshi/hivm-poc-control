import ast, pathlib, re, hashlib
EV=pathlib.Path("/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline")
V3=EV/"verify/tests/scripts/run_p2_suite_v3.py"; ARCH=EV/"verify/work/p2b-acceptance/logs-suite-p2b"
tree=ast.parse(V3.read_text()); keylists=[]
def litlist(a): return isinstance(a,(ast.List,ast.Tuple)) and a.elts and all(isinstance(e,ast.Constant) and isinstance(e.value,str) for e in a.elts)
for n in ast.walk(tree):
    if isinstance(n,ast.Call):
        fn=n.func.id if isinstance(n.func,ast.Name) else (n.func.attr if isinstance(n.func,ast.Attribute) else None)
        if fn in ("_expect_reject","reason_check"):
            for a in list(n.args)+[k.value for k in n.keywords]:
                if litlist(a): keylists.append((n.lineno,fn,[e.value for e in a.elts]))
    if isinstance(n,ast.Assign) and isinstance(n.value,(ast.List,ast.Tuple)):
        names=[t.id for t in n.targets if isinstance(t,ast.Name)]
        if any(x.endswith("KEYS") for x in names) and n.value.elts and all(isinstance(e,ast.Constant) and isinstance(e.value,str) for e in n.value.elts):
            keylists.append((n.lineno,names[0],[e.value for e in n.value.elts]))

full=ARCH.joinpath("A10.stderr").read_text(errors="replace").splitlines()
bare=ARCH.joinpath("A13.env.stderr").read_text(errors="replace").splitlines()
STUB=["[INFO] Using merged native A5 regbase pipeline",
 "[ERROR] Late mode: the bitcode identity gate and the HIVMC A5 lowering are not implemented in this build (P2-B tail only). Refusing to emit an artifact. No output was written."]
BOUND=["[ERROR] Late mode: input validation failed. The module was not transformed and no artifact was produced."]
MARK=["[P2-BOUNDARY] a5-compile entry=runHIVMCCompileA5","[P2-BOUNDARY] inference entry=inferLayoutOptimization",
 "[P2-BOUNDARY] inference entry=inferMixedCV","[P2-BOUNDARY] late-route stage=late entry=runLateHIVMPipeline",
 "[P2-BOUNDARY] ordinary-regbase-pipeline entry=runRegBasePipeline","[P2-BOUNDARY] retry-loop iteration=0"]
CORP_FULL={"marker":MARK,"stub":STUB,"boundary":BOUND,"bare-stream":bare,"embedded-stream-FULL":full}

print("=== FULL-corpus classification (census's own rule, untruncated dump) ===")
rows=[]
for ln,lab,ks in sorted(keylists):
    for k in ks:
        risky=sorted({c for c,cl in CORP_FULL.items() if any(k in l for l in cl)
                      and c in ("stub","boundary","marker","bare-stream","embedded-stream-FULL")})
        rows.append((ln,lab,k,risky))
for ln,lab,k,risky in rows:
    if risky: print(f"  COLLISION-RISK {k!r:34} <- {lab} (:{ln})  free on {risky}")
print()
print("=== the three truncation-hidden keys: which rows carry them? ===")
for key in ("llvm","regbaseintrins","version"):
    users=[(ln,lab) for ln,lab,k,_ in rows if k==key]
    hits=sum(1 for l in full if key in l)
    print(f"  {key!r}: {hits} hits in FULL A10.stderr; carried by {users if users else 'NO key-list (so it is a regex-only or other token)'}")
print()
print("=== all distinct keys and their full-stream hits ===")
for k in sorted({k for _,_,k,_ in rows}):
    print(f"  {k!r:34} full-hits={sum(1 for l in full if k in l):5d}  bare-hits={sum(1 for l in bare if k in l)}")
