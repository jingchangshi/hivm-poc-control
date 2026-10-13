import ast, pathlib, re, hashlib
EV=pathlib.Path("/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline")
V3=EV/"verify/tests/scripts/run_p2_suite_v3.py"
ARCH=EV/"verify/work/p2b-acceptance/logs-suite-p2b"
src=V3.read_text(); tree=ast.parse(src)
regexes=[];keylists=[]
for n in ast.walk(tree):
    if isinstance(n,ast.Assign) and isinstance(n.value,ast.Call):
        f=n.value.func
        if isinstance(f,ast.Attribute) and f.attr=="compile" and n.value.args and isinstance(n.value.args[0],ast.Constant):
            regexes.append((n.lineno,n.value.args[0].value))
    if isinstance(n,ast.Call):
        fn=n.func.id if isinstance(n.func,ast.Name) else (n.func.attr if isinstance(n.func,ast.Attribute) else None)
        if fn in ("_expect_reject","reason_check"):
            for a in list(n.args)+[k.value for k in n.keywords]:
                if isinstance(a,(ast.List,ast.Tuple)) and a.elts and all(isinstance(e,ast.Constant) and isinstance(e.value,str) for e in a.elts):
                    keylists.append((n.lineno,fn,[e.value for e in a.elts]))
    if isinstance(n,ast.Assign) and isinstance(n.value,(ast.List,ast.Tuple)):
        names=[t.id for t in n.targets if isinstance(t,ast.Name)]
        if any(x.endswith("KEYS") for x in names) and n.value.elts and all(isinstance(e,ast.Constant) and isinstance(e.value,str) for e in n.value.elts):
            keylists.append((n.lineno,names[0],[e.value for e in n.value.elts]))
print(f"runner sha {hashlib.sha256(V3.read_bytes()).hexdigest()[:16]}")
print(f"extracted {len(regexes)} regexes, {len(keylists)} key-lists\n")

full=ARCH.joinpath("A10.stderr").read_text(errors="replace").splitlines()
head_tail=full[:50]+full[-20:]
allkeys=sorted({k for _,_,ks in keylists for k in ks})
print(f"=== (A) truncation gap: key in FULL A10.stderr but NOT in head50+tail20? ===")
print(f"    full lines={len(full)}  censused lines={len(set(head_tail))}")
gap=0
for k in allkeys:
    inf=sum(1 for l in full if k in l); int_=sum(1 for l in head_tail if k in l)
    if inf and not int_:
        gap+=1; print(f"    MISSED BY TRUNCATION: key {k!r}  full-hits={inf} truncated-hits=0")
if not gap: print("    (no key is missed by the truncation)")
print()
print("=== (B) retry-loop line: fictional vs real ===")
fict="[P2-BOUNDARY] retry-loop iteration=0"
real_runtime="[P2-BOUNDARY] retry-loop iteration=1 of=2"
for lab,ln in (("census-typed",fict),("real-runtime",real_runtime)):
    kk=[k for k in allkeys if k in ln]
    rr=[(l,p) for l,p in regexes if re.search(p,ln)]
    print(f"    {lab:14} {ln!r}\n        keys matching: {kk}\n        regexes matching: {rr}")
print()
print("=== (C) does any regex match the retry-loop family at all? ===")
hits=[(l,p) for l,p in regexes if re.search(p,real_runtime) or re.search(p,fict)]
print("   ",hits if hits else "(none -- the retry-loop line is not read by any extracted regex)")
print()
print("=== (D) keys whose ONLY corpus hit is a marker line (i.e. routing-relevant) ===")
MARK=["[P2-BOUNDARY] a5-compile entry=runHIVMCCompileA5","[P2-BOUNDARY] inference entry=inferLayoutOptimization",
 "[P2-BOUNDARY] inference entry=inferMixedCV","[P2-BOUNDARY] late-route stage=late entry=runLateHIVMPipeline",
 "[P2-BOUNDARY] ordinary-regbase-pipeline entry=runRegBasePipeline",fict]
for k in allkeys:
    hm=[m for m in MARK if k in m]
    if hm: print(f"    key {k!r} hits marker(s): {[m.split('] ')[1] for m in hm]}")
