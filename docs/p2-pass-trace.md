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
- The instrumentation run uses `--mlir-print-ir-before-all --mlir-print-ir-after-all
  --mlir-print-ir-module-scope`, with `--mlir-print-ir-after-change` **off** and
  `--mlir-print-ir-tree-dir=<dir>` archiving the complete trace.
- Every row is reported **together with the resolved Mix/Aiv mode** (`p2-architecture.md` §3.2):
  a zero read is meaningless without it.
- A row whose coverage is not demonstrated per pass manager is **NOT_RUN**, not PASS.

## A. Routing

| # | claim | evidence needed | status |
|---|---|---|---|
| A1 | Late routing line present; ordinary-pipeline line absent | archived driver log; both lines are boundary logs (`p2-architecture.md` §3.2) | NOT_RUN |
| A2 | inference boundaries: `inferLayoutOptimization` / `inferMixedCV` did not run (or ran and set no consumed flag) | boundary log lines + the consumers of the flag | NOT_RUN |
| A3 | retry loop executed **zero** iterations in Late mode | per-iteration boundary log count = 0 | NOT_RUN |
| A4 | `runHIVMCCompileA5` invoked **exactly once**, outside any retry loop | A5 boundary log count = 1 | NOT_RUN |
| A5 | ordinary request (§4.1(9)) byte-for-byte unaffected | ordinary-input compile with and without the new binary; pass sequence compared | NOT_RUN |

## B. Forbidden — must be zero scheduled invocations

Each of these must be **absent** from a trace whose coverage is demonstrated per pass manager, and the
resolved mode recorded.

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
| C1 | the 20 tail passes ran, in the §3.1 order | trace order + per-pass **effect** in the IR (e.g. `cf` blocks after `--convert-scf-to-cf`; no `hivm.hir.*` after the conversions) | NOT_RUN |
| C2 | `ConvertHIVMToLLVM` = **exactly 1**, owned by HIVMC A5 | trace count = 1, and the pass is inside the A5 pipeline, not the tail | NOT_RUN |
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
| E1 | every pass manager in the run installs the instrumentation options (regbase tail, RetriablePassManager wrappers, A5 in-process) | NOT_RUN |
| E2 | `--mlir-print-ir-after-change` off | NOT_RUN |
| E3 | complete logs archived via `--mlir-print-ir-tree-dir`; nothing truncated | NOT_RUN |
| E4 | sentinels present: `--annotation-lowering` (first) and `--convert-hivm-to-llvm` (last) | NOT_RUN |
| E5 | resolved Mix/Aiv mode recorded for the run | NOT_RUN |

If E1–E5 are not all satisfied, every zero in section B is **NOT_RUN**, regardless of what the trace
shows.
