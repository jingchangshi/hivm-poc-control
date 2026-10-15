# P2 pass-trace acceptance record

> **Row IDs in this file are this file's own, and they COLLIDE with the frozen suite's.**
> `verify/tests/scripts/run_p2_suite.py` (the freeze of record, `0677be1a…`) numbers its rows
> `A1–A15 / B1–B12d / C1–C6 / D1–D4` with **different meanings for the same letters**
> (`docs/p2-verification.md:48` states that those IDs are the suite's, re-derived from the runner).
> Concrete collisions: suite **A1** = positive Late compile to an ELF, machine `0x1029`; here **A1** =
> the routing line is present and the ordinary-pipeline line absent. Suite **A2** = ordinary-path
> byte-identity regression; here **A2** = the two inference functions were not called. Suite **B1/B2** =
> the cache-0 and carrier-deleted fixtures; here **B1/B2** = forbidden-pipeline zeros. Suite **C1** =
> "absent ≡ explicit-0" on those two fixtures; here **C1** = the nineteen tail passes registered in order.
> Suite **D1–D4** = identity manifest / determinism / recompile / masquerade; here **D1–D11** = the
> contract invariants. **So a bare row ID in any document, including `docs/decisions.md` and
> `docs/status.md`, is ambiguous and must name which document it belongs to.** Two live instances are
> already in the record: D038(d) cites "`_expect_catch` satisfies B1/B2" (suite), while D039 rules on
> "B1–B7 are NOT_RUN" (this file).
>
> **Second, related hazard: the frozen suite carries no routing row at all.** Its runner contains
> **zero** occurrences of `preflight`, `late-tail`, `retry`, `only-run-hivm` or any boundary-log key
> (measured 2026-10-14 by grep over the sealed file), and the `[P2-BOUNDARY]` instrumentation the
> P2-B work adds is therefore read by **nothing** in the suite. Rows A1–A4 below are **Lead-read rows
> with no automated oracle** — a real but weaker class of evidence than the suite's, and it must be
> reported as such (`docs/p2-verification.md` coverage-delta 2 already records the gap and points at
> A1/A2b).

**Status: NOT_RUN — a P2 compiler now exists (`impl/stage-p2b-r1/`, `ea217b8d…`, D074), but no
archived run in this file is against it: the archived P2-B run bound the pre-instrumentation
`stage-p2b` `47e02604…` as its `p2_compiler` (measured in §E.3 below).** This file is the *record* for §3.2's
"measured at zero" claims. Every row below is **NOT_RUN** until the row carries a real command, a real
log path under `/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline/`, exit codes,
and input/output SHAs. Opening this file early is deliberate: it is far cheaper than reconstructing
evidence from memory afterwards, and it prevents a source-reading from being recorded as an execution.

Rules that apply to every row:

- Evidence comes from **real MLIR pass instrumentation** on the actual run, never from source grep.
  Source citations belong in `docs/p2-architecture.md`; they are not acceptance evidence here.
- **Absence** of a pass's headers, under the five coverage preconditions of `p2-architecture.md` §3.2,
  establishes "not scheduled". **Presence** establishes only "scheduled", not "executed" — so any row
  claiming a pass *worked* must also show its **effect in the emitted IR**, or a counter at the
  `execute`/`runOnOperation` boundary.
- The instrumentation run uses **unfiltered** `--mlir-print-ir-before-all --mlir-print-ir-after-all
  --mlir-print-ir-module-scope`, with `--mlir-print-ir-tree-dir=<dir>` archiving the complete trace.
  **Four filters are off, all four measured present on the pinned binary** (`--help`): `after-change`
  (*"only print if the IR changed"*), **`after-failure`** (*"only print if the pass failed"* — a
  failure-only filter, the one most easily left on by accident and the one that makes a clean trace
  indistinguishable from a run where nothing happened), and the per-pass selectors
  `--mlir-print-ir-before=<pass-arg>` / `--mlir-print-ir-after=<pass-arg>`. A trace produced with any
  of them is unusable for a zero in section B.
- **Counting is per operation scope; the assertion is compile-wide.** A construction entry is not an
  execution: nested `PassManager` construction inside a pass body (`PassPipeline.cpp:564` is exactly
  that) and retry wrappers can produce several entries for one execution, and nested function passes
  can execute separately on the entry and on the VF helper. So each count is **measured** at the
  `runOnOperation`/`execute` boundary of the scope that actually runs it. It is then **aggregated over
  every manager the run enters**, because the claims are about the whole invocation:
  `ConvertHIVMToLLVM` must be **1 in total** (one per scope does not satisfy it — a module pass that ran
  in two entered managers ran twice), the forbidden passes **0 in total** (absent from one manager but
  present in another is a failure, not a local exemption), and `runHIVMCCompileA5` **1 in total**. Where
  the claim is about a **function** rather than a pass (section A), the count is the boundary log's,
  not the trace's — and that log is likewise summed over the whole invocation, which is why it sits at
  the function entry and not at a call site.
- Every row is reported **together with the resolved Mix/Aiv mode** (`p2-architecture.md` §3.2):
  a zero read is meaningless without it — and the mode is the one detected at **the first action of the
  manager that produced that part of the trace**, since the policy freezes it for the rest of that run
  and only a *later* manager can see a transition (E5). The mode is not recomputed per pass action.
- A row whose coverage is not demonstrated per pass manager is **NOT_RUN**, not PASS.

## A. Routing

| # | claim | evidence needed | status |
|---|---|---|---|
| A1 | Late routing line present; ordinary-pipeline line absent | archived driver log; both lines are boundary logs (`p2-architecture.md` §3.2) | NOT_RUN |
| A2 | `inferLayoutOptimization` / `inferMixedCV` were **not called at all** | **absence** of the boundary log line at each function's top (`regbase/Utility.cpp:100`, `:61`). The claim is zero *calls*, not "ran and set no consumed flag": the Late branch returns as the first statement of `runRegBaseCompile`, before `Driver.cpp:68`/`:73`. The pass trace cannot substitute — both are plain `module.walk`s that add no passes | NOT_RUN |
| A2b | no ordinary regbase pipeline line | **absence** of the `runRegBasePipeline` boundary log, placed at the **function entry** (`regbase/BiShengIRCompileMain.cpp:345`) so that both callers (`Driver.cpp:81`, `PassPipeline.cpp:564`) are covered | NOT_RUN |
| A3 | retry loop executed **zero** iterations in Late mode | per-iteration boundary log count = 0 | NOT_RUN |
| A4 | `runHIVMCCompileA5` invoked **exactly once**, outside any retry loop | A5 boundary log count = 1, taken at the **function entry** (`bishengir/lib/Tools/hivmc/A5/HIVMCMainA5.cpp:944`) so both callers are covered (`BiShengIRCompileMain.cpp:448`, `hivmc/HIVMCMain.cpp:45`). **The oracle literal must carry its prefix: `[P2-BOUNDARY] a5-compile entry=runHIVMCCompileA5`.** The bare substring `a5-compile entry=runHIVMCCompileA5` is **NOT** a valid oracle: P2-C's new gate emits a *second*, differently-prefixed line carrying the same substring (`[P2-C-GATE] a5-compile entry=runHIVMCCompileA5 route=late`, `regbase/BiShengIRCompileMain.cpp:624`), so a prefix-agnostic count over the measured Late stream returns **2** (measured 2026-10-14 on `impl/scratch/p2c-smoke/late-20261014T005329Z/A/stderr`, lines 10 and 11) and would false-FAIL a correct run. Two tokens, two sites, two counts — never summed (D075; status 追加 61 §三, 追加 62 §一, 追加 65 §一) | **NOT_RUN — and UNATTAINABLE on the Late path until P2-C** (§E.4) |
| A5 | ordinary request (§4.1(9)) byte-for-byte unaffected | ordinary-input compile with and without the new binary; pass sequence compared | NOT_RUN |

### A.0 Anchor corrections, measured at HEAD `cfaa59677` (2026-10-14)

**Anchor by function name, not by line number.** The line numbers in the table above were read at
`80e29e6f2` (the commit this document cites). P2-A's `ddc212e29` inserted the Late branch into
`regbase/Driver.cpp`, which shifted the anchors the A-rows name. Measured now:

| row | anchor named above | at `80e29e6f2` (as written) | at HEAD `cfaa59677` (measured) |
|---|---|---|---|
| A2 | `regbase/Utility.cpp:100` and `:61` | correct | **unchanged** — `inferLayoutOptimization` `:100`, `inferMixedCV` `:61` |
| A2 | `Driver.cpp:68`/`:73` (where the Late branch returns before) | correct | **`Driver.cpp:79`/`:84`** |
| A2b | `regbase/BiShengIRCompileMain.cpp:345` (`runRegBasePipeline` signature) | correct | **unchanged** — `:344-346` |
| A2b | caller `Driver.cpp:81` | correct | **`Driver.cpp:92`** |
| A2b | caller `PassPipeline.cpp:564` | correct | **unchanged** |
| A4 | `hivmc/A5/HIVMCMainA5.cpp:944` | **the path is wrong** | **`bishengir/lib/Tools/hivmc/A5/HIVMCMainA5.cpp:944`** — there is no `Tools/bishengir-compile/` segment in this path; `:944` is the `runHIVMCCompileA5` signature, as intended |

**Measured absence, not assumed — and now measured PRESENT, with an explicit HEAD/working-tree split.**
At **HEAD `cfaa59677`**, `regbase/Utility.cpp` contains **zero** `llvm::errs()` calls, both functions
begin directly with their `module.walk`, and `runRegBasePipeline` has no log before
`checkInOutOptionsValidity(config)`. So against **HEAD alone** A1, A2, A2b, A3, A4 have no
instrumentation. **That changed in the uncommitted P2-B cut** (four files, `+82/−7`, D074): measured
2026-10-14 by `grep -c 'P2-BOUNDARY'` in the working tree — `regbase/LatePipeline.cpp` 1,
`regbase/BiShengIRCompileMain.cpp` 2, `regbase/Utility.cpp` 2, `hivmc/A5/HIVMCMainA5.cpp` 1 = six
sites, and the binary built from that cut (`ea217b8d…`) carries all six as strings. So the earlier
phrasing "un-runnable as the tree stands" is **superseded**: the instrumentation exists in the
working tree and in the P2-B artifact of record, and is **absent from HEAD** until the cut is
committed. These rows nonetheless stay **NOT_RUN**, and the reason is now specific rather than
premise-level: the only archived run whose streams are in the record was produced by
`stage-p2b` `47e02604…`, a binary that **cannot emit the channel at all** (§E.3, D048), so its
zeros are the unfirable-guard class (D040/D042) and not evidence. No row may be flipped by inference
from another, and none is flipped here.

**Why this note exists:** the anchors above were recorded as line numbers, and line numbers in a file
the phase itself edits are not stable identifiers. A log placed at the stale `Driver.cpp:81` would be
placed on nothing (that line is no longer the call), and the row would then measure a log that is
absent for the wrong reason. Same failure shape as the delta-4 literal corrected in
`p2-verification.md` §A.

## B. Forbidden — must be zero scheduled invocations

Each of these must be **absent — zero in total, summed over every manager the run enters** — from a
trace whose coverage is demonstrated per pass manager, with the resolved mode recorded. Absence from
one manager while the pass runs in another is a FAIL for that row, not a local exemption.

| # | forbidden | status |
|---|---|---|
| B1 | `buildLowerAVEPipelines` contents (VectorToHIVMAVE, ArithToHIVMAVE, reduction/loop/layout transforms, AVENormalizeOps) | NOT_RUN |
| B2 | HFusion scheduling / vectorization | NOT_RUN |
| B3 | bufferization / PlanMemory | NOT_RUN |
| B4 | GraphSyncSolver / InjectSync | NOT_RUN |
| B5 | automatic multibuffer | NOT_RUN |
| B6 | automatic layout / schedule replanning (the consumers of the flag `inferLayoutOptimization` can set) | NOT_RUN |
| B7 | the four SIMT/DPX tail passes gated by `getPureSimt() && getUseDPX()` (`AdaptGPUKernel`, `HoistCallScalarToCaller`, `DPXDivOptimization`, `SIMTInsertInitAndFinishForDebug`) | NOT_RUN |

## C. Required — order and count

| # | claim | evidence needed | status |
|---|---|---|---|
| C1 | the 19 registered tail passes are **constructed** in the §3.1 order (a *construction* claim, not an execution one — D043/Q6b) | trace order + per-pass **effect** in the IR (e.g. `cf` blocks after `--convert-scf-to-cf`; no `hivm.hir.*` after the conversions). The §3.1 table is the **base pipeline construction list, not nineteen runtime executions**: nested function passes execute separately on the entry and the VF helper, so the count is per pass **and per operation scope**, and a failed run still names later entries (`Pass.cpp:592-594`). (Corrected 2026-10-14 from 20 → 19: `BindSyncBlockLockArg` is a `Prepare`-phase pass and is not in this tail; see `p2-architecture.md` §3.1's correction. The measured P2-B stream shows the remaining nineteen plus four `OpToOpPassAdaptor` entries and the validator — 33 `[PassID]` actions, no gap.) | NOT_RUN |
| C2 | `ConvertHIVMToLLVM` = **exactly 1 in total**, owned by HIVMC A5 | per-scope counts summed over every entered manager = 1, and the single execution is inside the A5 pipeline, not the tail. "Once per scope" or "once in the tail and once in A5" is a **FAIL** | NOT_RUN |
| C3 | `AppendDeviceSpec` count = 0 in state 2 (complete matching spec) and = 1 in state 1 | trace count per state | NOT_RUN |
| C4 | the validator pass ran **before item 1** of the tail | trace order: validator precedes `--annotation-lowering` | NOT_RUN |

## D. Contract preservation on the IR

Asserted on the IR at each stage (not only at the end), with the failing stage named on failure.

| # | invariant | status |
|---|---|---|
| D1 | AIV grid = 64 | NOT_RUN |
| D2 | per-block GM indexing | NOT_RUN |
| D3 | 8192 f32 per block | NOT_RUN |
| D4 | UB byte offsets 0 / 65536 / 131072 | NOT_RUN |
| D5 | DMA payload 32768 bytes | NOT_RUN |
| D6 | dynamic UB = 196608 bytes | NOT_RUN |
| D7 | `l2_cache_mode = 4` on the entry's store (not replaced by cache0) | NOT_RUN — asserted on the **parsed** op, never on the artifact's comments. The frozen artifact's header (`:25`,`:27`) and inline note (`:184-188`) claim the opposite of line 189 and are already recorded as stale (`challenger-d025-zqumt69b/entry-evidence/p1v2/scoped-vf/INTEGRITY.json`, field `stale_header_comment`); the artifact is immutable. Cross-ref `p2-architecture.md` §2.4 |
| D8 | 128 iterations × 64 elements | NOT_RUN |
| D9 | pb32 carrier present and remains b32 | NOT_RUN |
| D10 | the four original set/wait sync ops | NOT_RUN |
| D11 | the entry's **terminal** `PIPE_ALL` — entry-scoped, not module-wide | NOT_RUN |

**D11 must be P2's own assertion, not a reuse of the P1 oracle's.** The D025 record states that the
frozen P1 checker's `B8.TERMINAL` branch (`p1_check.py:1561-1576`) has **no FAIL path** — it always
records PASS and only varies its message — so removing the artifact's terminal `hivm.hir.pipe_barrier[<PIPE_ALL>]`
changed **zero** verdicts under that checker
(`challenger-d025-zqumt69b/entry-evidence/p1v2/scoped-vf/README.md`, finding 4). A row that inherits
that behaviour cannot fail and is therefore NOT_RUN by construction.

**D11 is the row most likely to be reported falsely green.** The artifact declares a second function
(`@main_kernel_simd_vf_0`); a `PIPE_ALL` inside that helper must **not** satisfy this check. The
negative for this row is an input with the terminal barrier moved into the VF helper
(`verify/tests/negatives/b07_pipe_all_moved_into_vf.mlir`), and it must fail.

**Measured note (2026-10-14): the D11 negative does have an independent instrument at P2, unlike the
P1 oracle's.** The frozen suite carries three fail-capable rows for this construct — `t_b6`
(`b06_terminal_pipe_all_deleted`, "entry-terminal PIPE_ALL missing"), `t_b7`
(`b07_pipe_all_moved_into_vf`, "PIPE_ALL present only in helper(s)") and `t_b8`
(`b08_pipe_all_before_store`, "barrier precedes the store in the entry (wrong position)") — each of
which returns a real FAIL string from an output-side `out_checks` callable, so the P1 defect recorded
above (a checker whose branch had no FAIL path) does **not** carry over. Two caveats: those checks are
reached only on the `rc == 0` branch of `_expect_catch` (`run_p2_suite.py:955-981`); on the `rc != 0`
branch the row reduces to the shared wrong-reason test (the rejection must name one of its
`diag_keys`), which is also where the known vacuous-PASS mechanism lives (a key matched inside an
IR-dump body that merely echoes the input). And D11 **here** remains a Lead-read row on the archived
IR; the suite rows are a separate instrument with separate IDs — see the namespace note at the top.

## E. Coverage preconditions for the trace that fills this file

| # | precondition | status |
|---|---|---|
| E1 | every pass manager the run **actually enters** installs the instrumentation options — enumerated from the trace, not from the source. A manager the Late route correctly bypasses (e.g. a retry wrapper) is **not** required to appear; requiring every constructible manager would make a correct run unfalsifiable | NOT_RUN |
| E2 | all **four** filters off: `--mlir-print-ir-after-change`, `--mlir-print-ir-after-failure`, `--mlir-print-ir-before=<pass-arg>`, `--mlir-print-ir-after=<pass-arg>` | NOT_RUN |
| E3 | complete logs archived via `--mlir-print-ir-tree-dir`; nothing truncated | NOT_RUN |
| E4 | sentinels present: `--annotation-lowering` (first) and `--convert-hivm-to-llvm` (last) | NOT_RUN |
| E5 | mode recorded at **each entered manager boundary** (its first action), not once for the run: `PassExecutionPolicy` detects the mode lazily on a run's first action and **freezes it for that run** (`PassExecutionPolicy.h:76-99`), so a rewrite of `mix_mode`/`FilterPassesAttr` can only land on the **next** manager — the window V1's input check cannot see. Asserted as **no forbidden transition**: every boundary must read **Mix**, and any later **Aiv** is a FAIL (the denylist can then drop pass bodies the contract needs) | NOT_RUN |

If E1–E5 are not all satisfied, every zero in section B is **NOT_RUN**, regardless of what the trace
shows.

### E.1 Measured at P2-B (2026-10-14): E4 is only half-satisfied, so section B stays NOT_RUN

The step-3-only run (`impl/scratch/p2b-trace/`) satisfies **E1** (the instrumentation options come from
the shared `bishengir::runPipeline` entry, and the two managers the run enters are enumerated — see
E5/E.2), **E2** (the command line passes none of the four biasing filters), **E3** (`58` files
archived under `--mlir-print-ir-tree-dir`, nothing truncated) and **E5** (0 of the 58 archived dumps
introduce `mix_mode`, `filter_passes` or `FilterPassesAttr`, which **supports an inferred `Mix`** for
this run but does not prove it — **D043/Q4**: `PassExecutionPolicy` re-resolves the mode **per
manager** while `FilterPassesAttr` is checked **per action**, so a document that places both only at
later-manager boundaries is wrong, and the **first-action boundary mode records are still owed**).

**E4 is half-satisfied and cannot be otherwise at this step.** The first sentinel is present
(`inline.err` header #2 is `LateInputVerifierPass` / `hivm-verify-late-input`, immediately followed by
`AnnotationLowering`). The closing sentinel `--convert-hivm-to-llvm` is **structurally** absent: the
Late driver makes exactly two `runPipeline` calls — `late-preflight` and `late-tail`
(`regbase/LatePipeline.cpp:528-570`) — and that pass belongs to the **A5** lowering, goal §3.1
**step 5**, which D037 assigned to P2-C. `ConvertHIVMToLLVM`'s ownership is disjoint — the only two
`addPass(createConvertHIVMToLLVMPass…)` sites in the tree are `hivmc/A3/PassPipelineA3.cpp:64` and
`hivmc/A5/PassPipelineA5.cpp:52`, and only the A5 one is reachable here.

> **Corrected 2026-10-14 (D043/Q6).** This sentence previously read "the tail's pass set is disjoint
> from A5's", which is **literally false**. Measured: `regbase/PassPipeline.cpp:293` **and**
> `hivmc/A5/PassPipelineA5.cpp:53` both add `createLowerAffinePass()`, so `LowerAffine` is in both
> sets. The narrow claim above is the one the cited evidence supports, and it is also the
> load-bearing one: `ConvertHIVMToLLVM` ownership is what actually prevents duplicate lowering.

**Consequence, applied to this file's own author:** by the rule above, **B1–B7 are NOT_RUN at P2-B**.
The zeros measured over the whole raw stream are real measurements of a real stream — all 18
`FORBIDDEN_PASSES` spellings plus `VectorToHIVMAVE`/`ArithToHIVMAVE`/`AVENormalizeOps` are 0 across
`inline.out + inline.err`, re-swept by the Lead — but a run with no closing sentinel cannot show the
sweep reached the end of the chain, so they are **not yet a coverage proof**. They convert at P2-C.
**Prohibited:** satisfying E4 by injecting an artificial sentinel or empty-running A5 in a step-3-only
build; that manufactures evidence and would bypass §6.2's bitcode identity gate.

### E.2 Second instrumentation channel, measured but NOT used to flip any row

`--mlir-pass-statistics` (a real pass-manager channel, not a source grep) emits one report block per
`runPipeline` call. At P2-B: block 1 = `LateInputVerifierPass`; block 2 = exactly **19** tail passes
by class name in entry order, item-for-item with `p2-architecture.md` §3.1's corrected table and
containing **none** of the four SIMT/DPX passes. It also shows the four sync-lock passes as
`Infer → LowerCreate → Mark → Free`, with no `BindSyncBlockLockArg` (`Prepare`-phase only).

**Its limits — corrected 2026-10-14 (D043/Q6b), and the correction is stronger than the original
caveat.** This channel measures **construction, not execution**. Measured in source:
`PassStatistics.cpp:107-137` recursively prints every `pm.getPasses()` entry **regardless of whether
it executed** (including passes whose statistics are empty); `Pass.cpp:883-892` dumps the statistics
tree after `runPasses` **even when the run failed**; `Pass.cpp:592-594` stops execution at the first
failure while **later entries remain printable**; and `Pass.cpp:516-530` runs the `Before`
instrumentation **outside** the action callback, so a filter can suppress a pass's body while its
name and header still appear. Therefore:
- **Presence** establishes *scheduled-in-this-pipeline* only. It is **not** execution evidence, and
  because a failed run still names later passes, the 19-name table must **not** be read as an
  execution order.
- **Absence** establishes *not constructed in the retained pipeline-mode tree*. That is the same
  strength class as the header-based zero-claim, and it is still conditional: a pipeline-mode full
  tree cannot by itself cover dynamically invoked pipelines, removed entries, other managers, or
  complete-P2 E1–E5.
- It therefore may **not** carry section B's zeros and does **not** dissolve A11's `missing_lock`,
  and the phrase "at the execution level" is withdrawn from D039's account (see D043/Q6b).

It is **not** the channel the frozen suite reads (`run_p2_suite.py:733-735` records pass-statistics
and explicitly never asserts on it), so it cannot repair D-b. It is recorded in `docs/decisions.md`
D039 as a candidate **additive** instrument only — and D043/Q6d **BLOCKS** a statistics-only
*replacement* of the frozen acceptance while **APPROVING** it as supplementary corroboration; no row
here was flipped by it.

### E.3 Third instrumentation channel (boundary markers) — measured; the artifact the acceptance ran predates it

Six sites emit `[P2-BOUNDARY] …` on **stderr** (deliberately stderr, so the channel can never contaminate stdout, which every PassID row reads):

| marker emitted | site | fires when |
|---|---|---|
| `late-route stage=late entry=runLateHIVMPipeline` | `regbase/LatePipeline.cpp:494` | the Late branch was entered — at the **top** of the function |
| `ordinary-regbase-pipeline entry=runRegBasePipeline` | `regbase/BiShengIRCompileMain.cpp:353` | function entry; covers **both** callers (`Driver.cpp:92`, `PassPipeline.cpp:564`) |
| `retry-loop iteration=N of=M` | `regbase/BiShengIRCompileMain.cpp:408` | once **per retry iteration**; the claim is a count, so a boolean would not carry it |
| `inference entry=inferMixedCV` | `regbase/Utility.cpp:68` | function entry; a plain `module.walk` that schedules no pass, so the pass trace is blind to it |
| `inference entry=inferLayoutOptimization` | `regbase/Utility.cpp:112` | function entry; same reason |
| `[P2-BOUNDARY] a5-compile entry=runHIVMCCompileA5` | `bishengir/lib/Tools/hivmc/A5/HIVMCMainA5.cpp:952` | function entry, covering both callers. **The prefix is part of the literal**: a search for the bare substring also matches the P2-C gate line below, so an unanchored count is 2 on a correct Late run (measured 2026-10-14; status 追加 65 §一) |
| `[P2-C-GATE] a5-compile entry=runHIVMCCompileA5 route=late` | `regbase/BiShengIRCompileMain.cpp:624` | once per Late invocation, immediately before the route's single call site; a **different** site from the row above, and a separate count (P2-C; D075) |

**Measured presence of the literal `P2-BOUNDARY` in each staged binary** (`grep -c -a -F`, 2026-10-14): `stage-p2a` (`2bf961bf…`) **0**; `stage-p2a-r1` (`1a3f20f8…`) **0**; **`stage-p2b` (`47e02604…`) 0 — and that is the artifact the archived P2-B acceptance bound as `p2_compiler`**; `stage-p2b-r1` = `build/bin` (`ea217b8d…`) **6**, matching the six sites. Sweep of `verify/work/p2b-acceptance/logs-suite-p2b/`: neither `P2-BOUNDARY` nor `late-route` appears in **any** file; `A2.stderr` (372 B) and `A2.stdout` (0 B) are both 0. **Consequence for this file's own rows: A1–A3 have no executable evidence in the archived run** — the channel did not exist in the binary that produced it — so an A-row reported PASS from that run would be a PASS over a binary that could not emit the evidence. This is the D048 measurement; it does not by itself flip any row (they are already NOT_RUN), it fixes what "NOT_RUN" is *for*. The frozen suite is independent of this: `run_p2_suite.py` contains **zero** references to the marker strings, so no frozen row claims them, and the routing claim remains uncovered by the oracle.

### E.4 A4 is not an oracle gap: the Late path cannot reach A5 at P2-B (Challenger ruling, 2026-10-14)

Row A4 asserts `runHIVMCCompileA5` is invoked **exactly once**. On the only Late execution that exists at P2-B that
count is **zero**, and the reason is in the source, not in a missing reader: `regbase/LatePipeline.cpp:565-584` runs the
tail and then fails closed, and its own comment says steps 4-5 ("`addBitcodeAttrsToModule` with §3.3's hard check,
then exactly one `runHIVMCCompileA5`") "are separate authorized steps and are **NOT** part of this build". So the marker
this row would count is emitted only on the **ordinary/HIVMC** path — where our routing-pair run measured it once
(`lead/scratch/routing-pair/run-20261013T202128Z/ordinary/stderr`) — and never on the Late path. Two distinct facts
follow, and they must not be merged: (i) the marker has **no reader anywhere** in the project (D075), and (ii) even with
a reader, no P2-B Late run could satisfy the row. A4 is therefore **P2-C planned**: P2-C must execute a valid Late
artifact, count the A5 marker exactly once **on the Late path**, and retain the ordinary-path and marker-absence controls
so the count means something. Reading A4's NOT_RUN as "the oracle is missing" would understate it — the claim is
false of P2-B, not merely unmeasured.

**Limits of this channel — stated so the rows above are read at their true strength.**
- **Presence proves entry, never completion.** `late-route` fires before `verifyPinnedOptions` (`LatePipeline.cpp:497`) and before the fail-closed stub, so a run rejected immediately afterwards still prints it. A1 is evidence that the Late branch was **taken**, not that the Late pipeline ran.
- **The ordinary-route count is ≥1, not exactly 1.** `ordinary-regbase-pipeline` sits at the function entry precisely because `runRegBasePipeline` has two callers, so an ordinary run may print it more than once. Any ordinary-route assertion must be a presence test.
- **An absence clause is sound only when conjoined with the presence clause on the same run.** A binary carrying no instrumentation yields *everything* absent, so a standalone "absence of X on a Late run" row is a guard that cannot fire — the class catalogued in `docs/decisions.md` D040/D042/D049. A1 is correctly written as one row carrying both clauses; A2/A2b/A3 are read as **conditional on A1's presence clause**, and at P2-C the row set must include a positive control proving the marker channel is live on the binary under test before any of these zeros is quoted.
- **It is not a substitute for section B.** The channel names six functions; it says nothing about which passes ran, in what order, or how many times. It answers §4.1's routing questions and nothing else.
