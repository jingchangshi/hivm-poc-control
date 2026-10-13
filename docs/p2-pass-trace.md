# P2 pass-trace acceptance record

**Status: NOT_RUN — no P2 compiler exists yet.** This file is the *record* for §3.2's
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
| A4 | `runHIVMCCompileA5` invoked **exactly once**, outside any retry loop | A5 boundary log count = 1, taken at the **function entry** (`hivmc/A5/HIVMCMainA5.cpp:944`) so both callers are covered (`BiShengIRCompileMain.cpp:448`, `hivmc/HIVMCMain.cpp:45`) | NOT_RUN |
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

**Measured absence, not assumed:** at HEAD, `regbase/Utility.cpp` contains **zero** `llvm::errs()`
calls and both functions begin directly with their `module.walk`, and `runRegBasePipeline` has no log
before `checkInOutOptionsValidity(config)`. So **A1, A2, A2b, A3, A4 are un-runnable as the tree
stands** — the instrumentation they measure is not present. That instrumentation is in P2-B's scope
(recorded in `docs/status.md` §23 addendum); these rows stay **NOT_RUN** until it lands and the run is
archived, and no row may be flipped by inference from another.

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
| C1 | the 20 tail passes ran, in the §3.1 order | trace order + per-pass **effect** in the IR (e.g. `cf` blocks after `--convert-scf-to-cf`; no `hivm.hir.*` after the conversions). The §3.1 table is the **base pipeline construction list, not twenty runtime executions**: nested function passes execute separately on the entry and the VF helper, so the count is per pass **and per operation scope** | NOT_RUN |
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
