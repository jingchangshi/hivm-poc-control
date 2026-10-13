# P0 independent verification and test plan

Date: 2026-10-13. **Current status: P0 gate CLOSED by user adjudication (D021, 2026-10-13); P1
AUTHORIZED. The user decided every open gate — D012 → option (a) narrow store-side cache carrier
extension; D013 → option 2, a discriminating experiment BEFORE P1 production work; cut → option (b)
dedicated Late entry; D004 and `AppendDeviceSpec` → per the Lead recommendation.** The gate closed by
**user decision, not by a Challenger APPROVE**: the independent Challenger verdict (D020) stands as
the review record and remains REVISE / BLOCK, and **D012/D013 remain hard gates** until their closure
conditions are met. Sections 1–5 below are the 2026-10-12 historical record and are retained
verbatim; they described a P0 BLOCKED by model-route safety refusals, not by a compiler finding.
Section 6 is the current 2026-10-13 session's evidence.
Requested roles are `verify` (verifier-haiku) and `review` (challenger-opus); actual served models
require Magpie request-level evidence and remain unverified. Lead owns the source audit and
decisions/status. No production implementation is authorized by this document.

Evidence root: `/home/s00653124/workspace/hivm-poc-evidence/p0-20261012-late-contract/`. Verifier owns scripts, frozen expectations and execution evidence under `verify/`. The v2 runner used the existing `s00653124_tilelang` container through `scripts/in-container.sh`, conda env `s00653124_tilelang`. No rebuild, install, Golden generation or physical NPU run occurred in P0.

## 1. Repository and baseline ledger

| Item | Observed | Outcome |
|---|---|---|
| TileLang | Existing feat/hivm-backend-poc @ cc56643de04a49db6f50875e26d904b0c69e71a6; clean index/worktree | PASS, host git checks and verifier GV-01 |
| NPU-IR | Existing feat/tilelang-hivm-late-pipeline @ e878f34927fb5b77fd8c7257fda3a95a3560f2ec; clean index/worktree | PASS, host git checks and verifier environment log |
| Reference | main @417dae9908ad2ea5ffb74294a7075b6cb64b4f2b; clean, read only | PASS, host git checks |
| Golden migration | Generator and five canonical fixtures already committed in cc56643d | Accepted prior completion; no migration/regeneration this session |
| E0 Docker/SIMD/compiler/Triton | User accepted all PASS | Prior PASS; current rerun NOT_RUN |
| Actual served-model routing | No sanitized Magpie request-level evidence recovered | BLOCKED, MG-01 |

`repository-ledger-resumed.json` retains the resumed repository checks; the original ledger remains intact. Both branches already existed. Control directory is not a git repository. No new commit or push. Historical goal/environment migration and old TileLang HEAD statements are superseded by the user's latest baseline and this ledger.

## 2. Independent execution evidence

Frozen expectations: `verify/p0v2/expected_p0v2.txt`. Runner: `verify/p0v2/p0v2_probes.sh`. Environment/executable identity: `verify/p0v2/logs/env.txt`. Per-probe commands, outputs and FileCheck logs: `verify/p0v2/logs/`; verdict table: `summary.tsv`. The runner log ends with `DONE`, but p0v2 has no retained `*.exit` artifact, so the runner process exit code is not independently persisted. This does **not** mean all checks passed: the runner intentionally records failures and continues. Original `verify/variants`, `verify/logs`, `verify/run_p1_discriminators.sh` and v2 `logs_run1/` are preserved.

Invocation recorded by the verifier:

```sh
/home/s00653124/workspace/hivm-poc-control/scripts/in-container.sh bash /home/s00653124/workspace/hivm-poc-evidence/p0-20261012-late-contract/verify/p0v2/p0v2_probes.sh
```

Full expanded argv is recoverable from the runner and each `.out` log; some displayed CMD labels use only input basenames. Tools are existing `AscendNPU-IR-1/build/bin/bishengir-opt`, `bishengir-translate`, `FileCheck`, and `ccec-toolkits/default/ccec_compiler/bin/bisheng`. Environment log records executable SHA prefixes and mtimes, not full compiler freshness proof for a rebuilt P2 route.

| ID | Frozen expectation / actual | Outcome and evidence |
|---|---|---|
| GV-01 | Exact JSON SHA576fa7…0fe5 and TIR SHA419473…2a18; tracked generator/fixtures equal committed/index/worktree content | PASS, `GV-suite.out` |
| GV-02 | Existing manifest fields: N524288, cores64, elements/core8192, lanes64, iterations128, UB196608, f32 offsets0/16384/32768, sync directions/id0, hashes | PASS, `GV-suite.out` |
| GV-03 | Oracle requires manifest keys ub_offsets_bytes, dma_bytes_per_buffer, buffer_spacing_bytes, grid; all four are absent | **FAIL**, four preserved failed assertions. Values are independently available from TIR/derived geometry; do not add keys to Golden or change expectation. |
| GV-04 | Two reads32768 bytes/cache0 and one write32768 bytes/cache4, burst1 and strides32768 | PASS, `GV-suite.out` |
| GV-05 | MTE2→V set/wait before VF; V→MTE3 set/wait before output DMA | PASS, static TIR ordering |
| GV-06 | JSON-loaded UB buffer views are float32; element offsets map to byte offsets by factor4 | PASS, `GV-suite.out`, `GV06-diagnostic.out` |
| GV-07 | Fixed __file__-based output ROOT; checker does not prepare/write Golden or run generator | PASS, static inspection and runner audit |
| Roundtrip | load/save/load assert_structural_equal; resaved JSON text equals49638 bytes; UB/grid/calling_conv/launch tags/readonly indices | PASS for actual assertions in `RT-roundtrip.out` and `p0v2_roundtrip_check.py`; two guarded/literal-True checks are not independent proof. |
| RT-01..04 | Current ascend→ascend, pto→pto, hivm→ascend, pto+hivm→pto | PASS for baseline observation, `RT-suite.out`; future HIVM selection/rejection NOT_RUN |
| PU-01 | Single UB i64 address becomes raw inttoptr to ptr<6>, no mul/shl | PASS, command0; `PU-01.out` |
| PG-01 | Explicit pb32, mask256 → pge.b32 | PASS, command0/FileCheck0 |
| PG-02 | No functionType, mask256 → pge.b8 | PASS, command0/FileCheck0 |
| PG-03 | Old general fallback claim predicts b8 for no functionType, mask32 | **FAIL**, command134; AVENormalizeOps aborts with invalid predicate element bit width before fallback. `PG-03.out` |
| PG-04 | Explicit pb32, mask32 normalizes to predicate256 and pge.b32 | PASS, command0/FileCheck0 |
| PG-05 | Separate absent-type mask256/VL32 experiment → b8 | PASS, command0/FileCheck0; does not replace PG-03 |
| BP-01 | Static three-GM-memref entry with use-bare-ptr-memref-call-conv=true → exactly3 ptr<1> args | PASS, command0, `BP-01.out` |
| BP-02 | Default standalone conversion predicted struct descriptor parameters | **FAIL** against frozen prediction: command0, actual15 flattened args (6 pointers +9 i64), no struct parameters. `BP-02.out` |
| SD-01 | Existing store lowers to src,dst,atomic NONE i32=0 | PASS, command0/FileCheck0; third argument is not cache mode |
| SD-03 | Arbitrary l2_cache_ctl=4 attribute parses but is dropped by conversion | Probe PASS, command0/FileCheck0; **cache4 preservation claim FAIL**. `SD-03.out` and `.fc` |
| MC-01 | Bisheng dav-c310 preprocessor defines __DAV_C310__ | PASS, command0; macro observation only |
| MC-02 | C310 source helper hardcodes l2_cache_ctl0 | PASS, source check. Runner reads template include path; canonical source is lib/RegBase/Vector/include/RegBase/DMAUtils.h. Selected linked instruction NOT_RUN. |
| VF-01/02 | Existing translation fixtures preserve internal SIMD VF attrs and pointer noalias | PASS, command0/FileCheck0; complete new kernel NOT_RUN |
| SY-01/02 | Existing conversion fixture and explicit MTE2→V/V→MTE3 event0 lower with correct order/directions | PASS, command0/FileCheck0; terminal completion not tested |
| EX-01b | Existing RegBase conversion fixture | PASS, command0/FileCheck0 |
| MG-01 | Actual served-model proof from Magpie request logs | BLOCKED; aliases/transcript model/error names are insufficient |

GV suite: 34 checks, 30 PASS and four FAIL, command exit1. First v2 run failed additionally because the checker used `tvm.tir`; verifier corrected the API reference to current `tvm.tirx`, without weakening assertions, and retained `logs_run1/`. Old 66/66 evidence is not complete manifest acceptance: its missing-key handling skipped required fields. The original result remains historical evidence and is superseded for coverage by explicit GV-03 failures.

Important execution limits: SD probes use `Ascend950PR_9589`, not required9579. No full Late kernel, minimal-pass deletion test, complete9579 A5 driver compile, selected linked DMA instruction, terminal DMA completion test, ACL load, or numerical test was executed. Frozen expectations include observations from earlier probes as labeled OBS; these reruns establish reproducibility, not a wholly new source prediction. Verifier's suggested Store source relocation to1020–1031 conflicts with live source: at recorded HEAD the atomic operand code is `bishengir/lib/Conversion/HIVMToStandard/regbase/HIVMToStandard.cpp:1262-1275`; use the live source and leave the frozen text untouched.

## 3. Future tests defined before production

All tests below are NOT_RUN. Verifier must freeze runnable fixtures/oracles/full commands before implementation in the relevant phase.

| Phase | Required independent proof | Failure conditions |
|---|---|---|
| P1 | Complete manual Golden-compatible Late kernel parses/verifies; source trace for every call/attr; validator checks3 static GM args, module/entry AIV, VF function+call, fixed UB bytes0/65536/131072, arena196608, block*8192, 128*64, explicit pb32, event directions/id0, cache4 representation and completion policy | Unsupported syntax/metadata; wrong units/spacing/coverage; generic parser accepts semantically invalid input; cache4 dropped; completion left implicit without evidence |
| P2 | Newly rebuilt/installed compiler identity; approved P1 kernel through Late route; pass traces count exactly one ConvertHIVMToLLVM, zero early PlanMemory/InjectSync/AutoBlockify/vector scheduling; void entry with exactly3 GM pointers; symbol/core/grid/UB metadata; linked cache4 and completion policy | Stale installed binary/cache; duplicate conversion; flattened descriptors/hidden args; changed semantics; unverified selected bitcode |
| P3 | Standalone ACL harness independent of TileLang backend, grid64/UB196608; deterministic nonconstant per-core/per-lane inputs including b63, zeros/signs/ramps; compare all524288 outputs, guards and repeated launches | Compile/file existence/tensor print only; sampled/first-core assertion; mismatch/OOB/race symptoms |
| P4 | Normal TileLang host compile/import of validated prebuilt P3 artifact through existing tvm_ffi; A/B/C device args and separate grid/UB extras; all-element oracle; AscendC/PTO regressions | Runtime E2E mislabeled CodeGen; wrong ABI/stream/grid/UB; test injection enters production |
| P5 | Direct immutable JSON replay into new DeviceCodeGen without preparation; actual generated MLIR passes independent syntax/semantic assertions and compiles through P2; unsupported inputs reject | Golden modification; implementation-derived oracle; manual Late text/fallback/prebuilt substitution |
| P6 | Normal HIVM tilelang.compile; recorded new codegen and compiler invocation/artifact identity excluding P4 cache; NPU all-element compare; AscendC/PTO/Triton regression evidence | Stale binary/cache/manual MLIR/fallback; absent numerical assertion or actual new compilation |
| P7 | Optional multi-tile N2**21 after P6 with independently frozen contract | Premature scope expansion |

Numerical policy for P3/P4/P6: exact elementwise F32 equality for finite, exactly representable deterministic inputs and independent A+B. This does not claim signed-zero bit equality. Broad random tolerance/exceptional-value policy must be fixed and justified before execution, never adjusted after mismatch. Guards/repetition provide evidence, not proof of all possible races/OOBs.

Frozen semantic negative requirements: reject UB0/16384/32768 interpreted as bytes; DMA4096/slot32768/arena163840; GM offset j-only or block*16384/grid32; extra/descriptor device args or wrong symbol/order; missing AIV/VF markers; missing pb32/altered full-mask or zeroing; missing waits/wrong pipes/event; dropped cache4; tensors/alloc requests/DPX/SIMT/MixedCV/additional tiles. Specific diagnostics are required even where generic MLIR parsing succeeds.

Before deleting a conservative legalizer pass, freeze independent semantic assertions and compare addresses, payload, coverage/predicate, events, ABI and completion after each removal with pass invocation evidence. Do not infer minimality from pass names or isolated one-op success. Record full argv/cwd, container/env, source HEAD, actual executable hashes, command/assertion exits, stdout/stderr arrangement and original logs. Source facts, isolated conversions, full compilation, load/launch and numerical equality are separate evidence levels.

## 4. Recovery evidence reconciliation

The resumed verifier performed a read-only recheck at 2026-10-12 12:44 CST. It found no repository, Golden, tool-hash, prior-evidence, p0v2-summary, source-anchor or container drift. It independently confirmed that `build/bin/bishengir-opt` matches the recorded p0v2 hash while `build/install/bin/bishengir-opt` has the same size but a different hash and no installed `bishengir-translate`; install freshness remains UNPROVEN. E0 was deliberately not rerun, and P1–P6 remain NOT_RUN.

The recheck also found these documentation/evidence limitations: an obsolete handoff reference was stale and is now corrected in `docs/pass-cut.md` to the real coordination checklist; `team-gateway-evidence.json` is absent and remains unsynthesized; `docs-working-revision.json` is retained as an older pre-edit snapshot, while `docs-working-revision-current.json` records the current document hashes; and p0v2 has no retained `*.exit` file, so its process exit code is not independently persisted. These limitations do not alter the frozen expectations or prior probe outputs.

## Current recovery starting 2026-10-12T12:41:05Z

The existing accepted Golden invariants remain PASS by immutable input evidence. GV-03 remains **FAIL against its frozen expected-key oracle**: ub_offsets_bytes, dma_bytes_per_buffer, buffer_spacing_bytes and grid are absent in the accepted manifest. This is a preserved auxiliary schema-coverage mismatch, not authorization to alter the frozen manifest or revoke the user's accepted geometry. Those values must be checked from original TIR/JSON and independent unit arithmetic in the Late contract; no old expectation is weakened or relabeled PASS.

Lead recovery evidence: `/home/s00653124/workspace/hivm-poc-evidence/p0-20261012-resume-124105/lead/recovery-ledger.json` records actual argv, exits and outputs. Three branch/HEAD/index/worktree checks PASS; JSON/TIR SHA checks PASS; light container/CANN/Bisheng availability/Python3.11.17/two NPU health checks PASS. These are Lead checks, not new independent compiler/numerical tests. E0 rerun, p0v2 rerun and P1–P6 remain NOT_RUN. Current source inputs have no drift requiring broad baseline reruns. The accepted E0 Triton log and current `PlayGround/test_add_triton/test_add.py:38-42` contain a launch and tensor print, but no all-element numerical assertion. Retain the user's E0 PASS as the accepted baseline; this evidence cannot stand in for P3/P6 numerical correctness. TileLang's E0 SIMD log records one passed pytest. No script was changed or rerun to extend E0.

The current independent verifier read Golden/git, prior summaries and compiler/runtime source anchors, then failed at12:45:51.187Z with an explicit safety-refusal error. Its pre-refusal tool-call ledger is `lead/verifier-before-refusal.json`; actual returned tool outputs were also recovered in `lead/verifier-result-excerpts.json`, with a compact tool-result summary in `lead/verifier-recovery-check-summary.json`. Twelve read-only Bash calls returned non-error tool results, including Golden hash/git checks and source-anchor spot checks. This does not prove every assertion or whole command chain exit: some calls contain navigation misses and pipelines. No Write/Edit or final report was recovered from this verifier, and current-revision independent acceptance is **PARTIAL / BLOCKED**. Existing independent p0v2 outputs above remain valid at their original evidence level. Lead updates this recovery section after the verifier was stopped; these updates are not authored/approved by the refused verifier.

The current Challenger failed at12:44:37.553Z with an explicit safety-refusal error, producing **NO VERDICT / BLOCKED**, not APPROVE/REVISE/BLOCK on architecture. Both current failures have matching native StopFailure audit inputs and exact session/agent `blocked` database rows. SendMessage to verify was denied due to lack of verified name binding; no repeated send was attempted. Native TaskStop succeeded for both affected members. Sanitized report and preserved team identities: `/home/s00653124/refusal-capture/hivm-p0-20261012T124105Z.E3UJQ9/p0-refusal-report.json` and `p0-team-identities.json`. For these two observed events detection succeeded; universal refusal detection and all internal wake coverage remain unproved. Requested-route dispatch corroboration is saved separately, while actual served-model/provider request-level evidence remains NOT_VERIFIED / NOT_AVAILABLE.

Lead current Runtime source recheck is `lead/runtime-source-recheck.md`. CANN9.2 header confirms dynamic-UB ID2 and HostArgs parameter order; compiled SDK/local structure layout and new HIVM ELF interoperability remain NOT_RUN. This source-only observation supports existing-runtime reuse, not P3/P4 runtime acceptance.

## 5. Independent review and closure

Earlier draft1 recorded REVISE and corrected `hivm.hir.block_idx` to `hivm.hir.get_block_idx`, module AIV and explicit pb32 requirements. That historical review does not approve this revision. Current reviewer encountered gateway errors containing `API Error: 400 claude-opus-5-5 refused this request (safety filter: refusal) via Claude Code`; verifier later encountered the analogous haiku/OpenCode Go error. These are gateway responses, not architecture verdicts or automatic tool-approval rejections.

Correction to Lead's earlier status: verifier did complete independent tests before later refusals. A report SendMessage to `team-lead` at01:23:29Z was recovered from its transcript; no successful delivery to current Lead is assumed. Evidence files were audited directly. No final current Challenger APPROVE/REVISE/BLOCK was recovered. The previously cited `team-gateway-evidence.json` is absent from the evidence directory; it is not recreated or treated as evidence. Raw transcripts/configuration are not distributed.

P0 remains BLOCKED by final independent review, actual served-model evidence, cache4 representation and terminal DMA completion design. Recovery additionally found that build/bin and build/install/bin compiler hashes differ, so installed compiler freshness is unproven. The recovery `challenge-resume` role then failed twice while reading the P0 materials, at `2026-10-12T04:36:06.987Z` and `2026-10-12T04:41:17.297Z`, with `API Error: 400 claude-opus-5-5 refused this request (safety filter: refusal) via Claude Code`; no Challenger verdict was produced. These gateway failures are not architecture verdicts. Documents may be handed off with accurate blockers; this is not P0 PASS and grants no P1 authorization. Minimal pass set remains a candidate until full-kernel evidence. Existing high-level store cannot currently support a faithful cache4 claim; numerical equality alone cannot waive that parameter. Stop at P0 for user review.

## 6. Current session 2026-10-13 (re-authorized P0) — independent evidence

Evidence root: `/home/s00653124/workspace/hivm-poc-evidence/p0-20261013-late-pipeline/`.
Roles: `verify` (verifier-haiku) and `review` (challenger-opus). Requested-route model binding is
recorded by the team runtime; actual served-model proof from Magpie request logs remains NOT_VERIFIED.

### 6.1 Verifier Task 1 — Golden integrity: PASS 55/55

Script `verify/verify_golden.py` (run `python -I`, in-container via `scripts/in-container.sh`,
conda env `s00653124_tilelang`), log `verify/verify_golden.log`: `total=55 pass=55 fail=0`, exit 0.
Per-item ledger `verify/summary.tsv`. Covers both contract hashes asserted against the constants in
`docs/goal.md` (not a repo-side expected file), manifest field-by-field equality plus key-set
equality, 21 text + 4 arithmetic TIR corroborations, JSON round-trip
(`tvm.ir.load_json` + `assert_structural_equal` + reserialization), absence of the legacy
`tests/tests` path, and static `__file__`-based ROOT in the freeze script.

Honest NOT_RUN: **T1.10 provenance replay is NOT_RUN** — an independent in-memory re-lowering
comparison was not performed, because the freeze script is not executed in place. This is recorded
as NOT_RUN, not PASS. End-of-session re-hash (`verify/golden-rehash-end-of-session.log`, 16:50:07Z)
confirms all five fixtures unchanged, so the verification mutated nothing.

### 6.2 Verifier Task 2 — Compiler binary identity: RESOLVED

`verify/build-artifact-identity.json`, `build-vs-install-byte-diff.log`,
`single-byte-diff-context.log`, `build-install-runpath-evidence.log`, `path-tool-resolution.log`,
`binary-pin-note.md`. No rebuild, no install, nothing modified.

- build↔install pairs differ by **exactly 1 byte** (RUNPATH trailing `:`); `readelf -d` confirms
  `$ORIGIN/../lib:` vs `$ORIGIN/../lib`. Same linked code; both SHAs denote one build.
- Container `PATH` resolves the tools to the **CANN 9.2 toolkit** (`bishengir-opt` sha
  `248b6c09…`), NOT the local build (`71ac4012…`). Any bare-name probe is a foreign-binary probe.
- `bishengir-toolkits/dev/bin/bishengir-compile` → `build/install/bin/bishengir-compile`.
- No `install_manifest.txt`; CMake records no install provenance.

This resolves the earlier "same size, different SHA ⇒ install freshness UNPROVEN" note to a
byte-level explanation, and establishes the P1/P2 pinning rule (D019).

### 6.3 Verifier Task 3 — P0 acceptance matrix (draft)

`verify/p0-acceptance-criteria.md` — an independent, non-weakening elaboration of the P0 row of
`docs/acceptance.md`. Sections A (Golden immutability), B (Late-IR contract validity), C (pass-cut
correctness), D (compiler binary identity). Explicit NOW / P1 / P2 / REVIEW windows, explicit
"what must NOT count" lists, and NOT_RUN discipline.

**Lead review complete:** `lead/lead-review-of-acceptance-matrix.md` — **ACCEPT with one required
correction and two tightenings.** The correction: C1 cites the *generic* driver's
`hivm::buildConvertToHIVMPipeline` + `hivm::buildOptimizeHIVMPipeline`
(`Tools/bishengir-compile/PassPipeline.cpp:98-107`), but the A5 route is RegBase, which calls
`hivm::regbase::buildConvertToHIVMPipeline` + `hivm::regbase::buildHIVMTensorOptimizations`
(`regbase/PassPipeline.cpp:501-502`); `buildOptimizeHIVMPipeline` is never called on that path
(grep: 0 hits). The tightenings: B4 must record that the D009 gap is the sharpest of the three
(silent b8 at the Golden's mask length), and D3 must be promoted to a §F rule that a run without an
absolute-path+sha256 compiler identity is BLOCKED, not PASS. Verifier asked to apply all three. **Verifier applied them (matrix Rev 2, 19745 B, sha256
`3658d8ba…`; Rev 1 was 16421 B / `2d327a8a…`), re-verifying every citation against live source
rather than copying.** Two verifier nits were accepted and fixed by the Lead: the RegBase entry
`buildBiShengHIRPipeline` is at `regbase/PassPipeline.cpp:461` (the Lead review had said 459; `grep -n`
gives exactly one hit at 461), and `docs/status.md` misspelled the generator as
`freeze_golden_hvm.py` (actual `freeze_golden_hivm.py`). The verifier also independently re-derived
the D009 mechanism (`getBitWidthFromAttr` at `HIVMAVE/Utils/Utils.cpp:864-880` initializes `-1` and
only fills 8/16/32 from `functionType`; `VL_BITS = 256*8 = 2048`) and confirmed the b32 target form
exists in-tree (`regbase-convert-llvm.mlir` CHECK lines for `intr.hivm.pge.b32 {mask_bit_width = 32}`).

### 6.4 Lead source findings this session

`lead/lead-source-trace.md` §15-§29. New or corrected:
- §16/§16.4 **D018**: `--enable-direct-hivm-lowering` does NOT reach the AVE optimizer guard in the
  `bishengir-compile` path; the "existing flag expresses the bypass" hypothesis is FALSIFIED.
- §17 **D012**: exhaustive carrier search — no MTE3 store-side cache field exists in the HIVM
  dialect (`EvictionPolicy` is LoadOp-only; raw `l2_cache_mode` is ND2NZOp-only, both MTE2).
- §18 **D013**: the terminal `PIPE_ALL` barrier is a GraphSyncSolver artifact, gated on
  `isIntraCoreMode()` and the `hacc.entry` attribute, and suppressed by `hivm.vector_function`
  (`SyncSolverCodeGen.cpp:923-933,975-982`); `InjectSync` adds it in `BARRIERALL` mode and in
  `NORMAL` mode too (`SyncAnalysis.cpp:103-119`). The Late
  cut skips both passes, so it drops the epilogue. **Correction:** the frozen `ascend_reference.asc`
  is AscendC **source**, not compiler output, so its lack of a terminal barrier is not evidence about
  emitted code; the earlier "the question is shared" inference is weaker than first recorded.
  **Result (2026-10-13):** the frozen pass-level discriminating experiment
  (`lead/d013-experiment-frozen-20261013.md`, 7 arms) ran and reported — §5 row `HAS / NO / NO`, **the
  drop is CONFIRMED** (`lead/d013-result-20261013.md`). P1 must supply terminal completion explicitly
  or justify the drop from the device layer; the device-layer question is a P3 obligation.
- §19/§19a **D004**: `target_is_plain_ascend` is defined by the absence of `pto`, so an `hivm` key
  makes it match too; resolution (not registration) is what fails.
- §20-§27: verified Late-IR syntax table, VF marker, authoritative stage cut, HIVMC A5 chain, ABI
  discriminator (bare-ptr: 3 args vs 15), D018 structural isolation, goal.md doc discrepancies.
- §28 **D009 sharpened**: `AVEPgePattern` derives the predicate width as `VL_BITS/numElems` =
  2048/numElems when `functionType` is absent, accepting only 8/16/32. The Golden mask is
  `boolx256` → derived 8, a *legal* width, so the missing pb32 carrier is **silent** wrong-code
  (b8 instead of b32), not a diagnostic. A 32-element mask would derive 64 and abort (PG-03, rc134).
- §29 **D012 sharpened**: probe SD-03 sets `{l2_cache_ctl = 4 : i64}` on `hivm.hir.store`; it
  **parses cleanly** but `-convert-hivm-to-std` output is byte-identical to the cache0 case SD-01
  apart from the function name — the libcall still gets `%c0_i32`. The attribute is **silently
  discarded**, the worst case: a clean compile can hide a cache0 binary.

### 6.5 Challenger verdict (independent)

**VERDICT DELIVERED.** The Challenger (`review`, challenger-opus) returned a
full source-anchored independent verdict by SendMessage. It does not write `.md` artifacts (its
instruction forbids report/summary/findings files), so `review/` is empty **by design, not by
failure**; the verdict is recorded in `docs/decisions.md` D020 and here. The verdict:

- **(a) Late-IR contract boundary: REVISE, with P1 advancement BLOCKED.** The boundary (primitive
  HIVM/HIVMAVE + explicit addresses/sync/VF/metadata before legalization, exactly one A5 HIVMC LLVM
  owner) is conceptually correct and source-supported, but it cannot be accepted while D012 cache4,
  D013 terminal completion, ABI, pb32/VF preservation and execution evidence are unresolved.
- **(b) Pipeline cut point: BLOCK for the existing `--enable-direct-hivm-lowering` mechanism;
  REVISE for the architecture.** D018 independently confirmed. Since `buildLowerAVEPipelines` is
  called only at `regbase/PassPipeline.cpp:223`, the correct minimal route is a dedicated, mutually
  exclusive Late entry in the existing RegBase driver (no new target kind) that skips normal
  HIR/HFusion, bufferization, planning, auto-sync, loop lowering, multibuffer and the AVE builder,
  retains validated legalization, and runs exactly one `runHIVMCCompileA5`. Option (a) getter
  plumbing is a narrow fix, not the cut; option (c) optimizer-survival is unsafe as an assumption.
- **(c) D012: BLOCK for P1** — accepted equivalence only with independent exact-target source/device
  evidence that cache4 and the emitted cache0 path are semantically equivalent for this exact store
  and target; numerical equality alone is insufficient. No such evidence exists.

Two new Challenger source claims were re-verified by the Lead: `AppendDeviceSpec` is **not
idempotent** (`AppendDeviceSpec.cpp:142-151` warns then overwrites the spec unconditionally), and the
pge width default also exists on the lowering path (`HIVMPgeOpLowering`,
`HIVMAVEToAVEIntrin.cpp:1419-1443`), independently confirming the §28 D009 mechanism. No compiler or
hardware execution was performed by the Challenger; its verdict is source-review only.

### 6.6 Status of this session's P0 gate

Independent Golden acceptance PASS; binary identity resolved; acceptance matrix drafted and
Lead-reviewed (C1 corrected, B4 sharpened, §F PATH rule added); source trace complete; two hypotheses
falsified/narrowed; two contract hazards sharpened to proven silent-failure modes; **independent
Challenger verdict delivered**.

**The user closed the P0 gate on 2026-10-13 by adjudication (D021) and authorized P1.** The user
decided every open item: D012 → option (a), a narrow store-side cache carrier extension; D013 →
option 2, a discriminating experiment with the existing compiler BEFORE P1 production work; the
pipeline cut → option (b), the dedicated mutually exclusive Late entry; D004 and `AppendDeviceSpec`
→ implement per the Lead recommendation; P1 → authorized.

**The gate closed by user decision, not by a Challenger APPROVE.** The Challenger's own verdicts
remain REVISE and BLOCK (D020) and stand as the review record; no finding was withdrawn or weakened
to reach the closure. **D012 (cache4 carrier) and D013 (terminal completion) remain hard gates** until
their closure conditions are met — the extension lands and is independently reviewed, and the D013
experiment reports. P2–P6 remain NOT_RUN and unauthorized.
