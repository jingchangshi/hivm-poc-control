# P0 independent verification and test plan

> **Snapshot notice.** This is the simplified-technical-English rewrite of `docs/p0-verification.md`,
> taken BEFORE the user adjudicated the P0 gates on 2026-10-13 (D021). Its gate-status statements
> ("the P0 gate is NOT closed", "No P1 authorization is granted") were true when written and are now
> superseded: the gate is closed by user adjudication and P1 is authorized. For current state read
> `docs/p0-verification.md`. Kept as a wording reference, not as the record.
>
> **Further supersession (2026-10-13, after this snapshot).** Its "D012 and D013 stay open as hard
> gates" line is also out of date. The D013 discriminating experiment has since been executed and
> reported: the drop is confirmed, and D013's gate is closed. D012 remains a hard gate. Read
> `docs/status.md` for current state.

Date: 2026-10-13. **Current status: P0 IN PROGRESS (the user re-authorized P0 on 2026-10-13). The
P0 gate is NOT closed.** The independent Challenger delivered its verdict (contract REVISE / P1
BLOCKED; cut BLOCK-for-flag and REVISE-for-architecture, D020). D012 and D013 stay open as hard
gates. Sections 1–5 are the 2026-10-12 historical record, retained verbatim. They describe a P0
that model-route safety refusals blocked, not a compiler finding. Section 6 holds the evidence of
the current 2026-10-13 session. The requested roles are `verify` (verifier-haiku) and `review`
(challenger-opus). Actual served models require Magpie request-level evidence and remain
unverified. The Lead owns the source audit, `decisions.md` and `status.md`. This document
authorizes no production implementation.

Evidence root: `/home/s00653124/workspace/hivm-poc-evidence/p0-20261012-late-contract/`. The
Verifier owns the scripts, the frozen expectations and the execution evidence under `verify/`. The
v2 runner used the existing `s00653124_tilelang` container through `scripts/in-container.sh`,
conda env `s00653124_tilelang`. P0 ran no rebuild, no install, no Golden generation and no physical
NPU run.

## 1. Repository and baseline ledger

| Item | Observed | Outcome |
|---|---|---|
| TileLang | Existing `feat/hivm-backend-poc` @ `cc56643de04a49db6f50875e26d904b0c69e71a6`; clean index and worktree | PASS, host git checks and verifier GV-01 |
| NPU-IR | Existing `feat/tilelang-hivm-late-pipeline` @ `e878f34927fb5b77fd8c7257fda3a95a3560f2ec`; clean index and worktree | PASS, host git checks and verifier environment log |
| Reference | `main` @ `417dae9908ad2ea5ffb74294a7075b6cb64b4f2b`; clean, read only | PASS, host git checks |
| Golden migration | The generator and the five canonical fixtures are already committed in `cc56643d` | Accepted prior completion; no migration or regeneration this session |
| E0 Docker, SIMD, compiler, Triton | The user accepted all as PASS | Prior PASS; the current rerun is NOT_RUN |
| Actual served-model routing | No sanitized Magpie request-level evidence recovered | BLOCKED, MG-01 |

`repository-ledger-resumed.json` retains the resumed repository checks. The original ledger remains
intact. Both branches already existed. The control directory is not a git repository. This phase
made no new commit and no push. The user's latest baseline and this ledger supersede the historical
goal, environment and old TileLang HEAD statements.

## 2. Independent execution evidence

Frozen expectations: `verify/p0v2/expected_p0v2.txt`. Runner: `verify/p0v2/p0v2_probes.sh`.
Environment and executable identity: `verify/p0v2/logs/env.txt`. Per-probe commands, outputs and
FileCheck logs: `verify/p0v2/logs/`. Verdict table: `summary.tsv`. The runner log ends with `DONE`,
but p0v2 retains no `*.exit` artifact, so the runner process exit code is not independently
persisted. This does **not** mean all checks passed: the runner records failures and continues by
design. The original `verify/variants`, `verify/logs`, `verify/run_p1_discriminators.sh` and the v2
`logs_run1/` are preserved.

Invocation recorded by the verifier:

```sh
/home/s00653124/workspace/hivm-poc-control/scripts/in-container.sh bash /home/s00653124/workspace/hivm-poc-evidence/p0-20261012-late-contract/verify/p0v2/p0v2_probes.sh
```

The runner and each `.out` log hold the full expanded argv. A displayed CMD label can use only the
input basename. The tools are the existing `AscendNPU-IR-1/build/bin/bishengir-opt`,
`bishengir-translate`, `FileCheck`, and `ccec-toolkits/default/ccec_compiler/bin/bisheng`. The
environment log records executable SHA prefixes and mtimes. The log is not full compiler freshness
proof for a rebuilt P2 route.

| ID | Frozen expectation / actual | Outcome and evidence |
|---|---|---|
| GV-01 | Exact JSON SHA `576fa7…0fe5` and TIR SHA `419473…2a18`; tracked generator and fixtures equal the committed, index and worktree content | PASS, `GV-suite.out` |
| GV-02 | Existing manifest fields: N `524288`, cores `64`, elements/core `8192`, lanes `64`, iterations `128`, UB `196608`, f32 offsets `0`/`16384`/`32768`, sync directions and id `0`, hashes | PASS, `GV-suite.out` |
| GV-03 | The oracle requires the manifest keys `ub_offsets_bytes`, `dma_bytes_per_buffer`, `buffer_spacing_bytes` and `grid`; all four are absent | **FAIL**, four preserved failed assertions. The TIR and the derived geometry supply these values independently. Do not add keys to the Golden and do not change the expectation. |
| GV-04 | Two reads of `32768` bytes with cache0, one write of `32768` bytes with cache4, burst `1`, strides `32768` | PASS, `GV-suite.out` |
| GV-05 | MTE2→V set and wait before VF; V→MTE3 set and wait before the output DMA | PASS, static TIR ordering |
| GV-06 | The JSON-loaded UB buffer views are float32; element offsets map to byte offsets by factor `4` | PASS, `GV-suite.out`, `GV06-diagnostic.out` |
| GV-07 | Fixed `__file__`-based output ROOT; the checker does not prepare or write the Golden and does not run the generator | PASS, static inspection and runner audit |
| Roundtrip | load, save and load with `assert_structural_equal`; the resaved JSON text equals `49638` bytes; UB, grid, calling_conv, launch tags and readonly indices | PASS for the actual assertions in `RT-roundtrip.out` and `p0v2_roundtrip_check.py`; two guarded or literal-True checks are not independent proof. |
| RT-01..04 | Current `ascend`→`ascend`, `pto`→`pto`, `hivm`→`ascend`, `pto+hivm`→`pto` | PASS for the baseline observation, `RT-suite.out`; future HIVM selection and rejection NOT_RUN |
| PU-01 | A single UB i64 address becomes a raw inttoptr to `ptr<6>`, with no mul and no shl | PASS, command 0; `PU-01.out` |
| PG-01 | Explicit pb32, mask256 → `pge.b32` | PASS, command 0 / FileCheck 0 |
| PG-02 | No functionType, mask256 → `pge.b8` | PASS, command 0 / FileCheck 0 |
| PG-03 | The old general fallback claim predicts b8 for no functionType and mask32 | **FAIL**, command 134; `AVENormalizeOps` aborts with an invalid predicate element bit width before the fallback. `PG-03.out` |
| PG-04 | Explicit pb32, mask32 normalizes to predicate256 and `pge.b32` | PASS, command 0 / FileCheck 0 |
| PG-05 | A separate absent-type mask256 / VL32 experiment → b8 | PASS, command 0 / FileCheck 0; this does not replace PG-03 |
| BP-01 | A static three-GM-memref entry with `use-bare-ptr-memref-call-conv=true` → exactly 3 `ptr<1>` args | PASS, command 0, `BP-01.out` |
| BP-02 | Default standalone conversion predicted struct descriptor parameters | **FAIL** against the frozen prediction: command 0, actual 15 flattened args (6 pointers and 9 i64), no struct parameters. `BP-02.out` |
| SD-01 | The existing store lowers to src, dst, atomic NONE i32 = 0 | PASS, command 0 / FileCheck 0; the third argument is not a cache mode |
| SD-03 | An arbitrary `l2_cache_ctl=4` attribute parses but the conversion drops it | Probe PASS, command 0 / FileCheck 0; **the cache4 preservation claim FAILS**. `SD-03.out` and `.fc` |
| MC-01 | The Bisheng `dav-c310` preprocessor defines `__DAV_C310__` | PASS, command 0; macro observation only |
| MC-02 | The C310 source helper hardcodes `l2_cache_ctl` 0 | PASS, source check. The runner reads the template include path; the canonical source is `lib/RegBase/Vector/include/RegBase/DMAUtils.h`. The selected linked instruction is NOT_RUN. |
| VF-01/02 | The existing translation fixtures preserve the internal SIMD VF attrs and the pointer noalias | PASS, command 0 / FileCheck 0; the complete new kernel is NOT_RUN |
| SY-01/02 | The existing conversion fixture and the explicit MTE2→V / V→MTE3 event 0 lower with the correct order and directions | PASS, command 0 / FileCheck 0; terminal completion is not tested |
| EX-01b | The existing RegBase conversion fixture | PASS, command 0 / FileCheck 0 |
| MG-01 | Actual served-model proof from the Magpie request logs | BLOCKED; aliases, transcript model names and error names are insufficient |

The GV suite holds 34 checks: 30 PASS and four FAIL, command exit 1. The first v2 run failed
additionally because the checker used `tvm.tir`. The verifier corrected the API reference to the
current `tvm.tirx`, without weakening assertions, and retained `logs_run1/`. The old 66/66 evidence
is not complete manifest acceptance: its missing-key handling skipped required fields. The original
result remains historical evidence, and the explicit GV-03 failures supersede it for coverage.

Important execution limits: the SD probes use `Ascend950PR_9589`, not the required `9579`. No one
executed a full Late kernel, a minimal-pass deletion test, a complete `9579` A5 driver compile, the
selected linked DMA instruction, a terminal DMA completion test, an ACL load, or a numerical test.
The frozen expectations include observations from earlier probes, labeled OBS. These reruns
establish reproducibility, not a wholly new source prediction. The verifier's suggested Store
source relocation to `1020–1031` conflicts with the live source: at the recorded HEAD the atomic
operand code is `bishengir/lib/Conversion/HIVMToStandard/regbase/HIVMToStandard.cpp:1262-1275`. Use
the live source and leave the frozen text untouched.

## 3. Future tests defined before production

Every test below is NOT_RUN. In the phase named in the row, the verifier must freeze the runnable
fixtures, the oracles and the full commands before implementation starts.

| Phase | Required independent proof | Failure conditions |
|---|---|---|
| P1 | The complete manual Golden-compatible Late kernel parses and verifies; a source trace covers every call and attribute; the validator checks 3 static GM args, the module and entry AIV, the VF function and call, the fixed UB bytes `0`/`65536`/`131072`, the arena `196608`, block × `8192`, `128`×`64`, explicit pb32, the event directions and id `0`, the cache4 representation and the completion policy | Unsupported syntax or metadata; wrong units, spacing or coverage; the generic parser accepts semantically invalid input; the team drops cache4; the team leaves completion implicit without evidence |
| P2 | Newly rebuilt and installed compiler identity; the approved P1 kernel through the Late route; pass traces count exactly one `ConvertHIVMToLLVM` and zero early PlanMemory, InjectSync, AutoBlockify or vector scheduling; a void entry with exactly 3 GM pointers; symbol, core, grid and UB metadata; the linked cache4 and completion policy | A stale installed binary or cache; duplicate conversion; flattened descriptors or hidden args; changed semantics; unverified selected bitcode |
| P3 | A standalone ACL harness independent of the TileLang backend, grid `64` and UB `196608`; deterministic nonconstant per-core and per-lane inputs including b63, zeros, signs and ramps; a compare of all `524288` outputs, the guards and repeated launches | Compile, file existence or tensor print only; a sampled or first-core assertion; mismatch, OOB or race symptoms |
| P4 | A normal TileLang host compile and import of the validated prebuilt P3 artifact through the existing `tvm_ffi`; A, B and C device args and separate grid and UB extras; an all-element oracle; AscendC and PTO regressions | A runtime E2E mislabeled CodeGen; a wrong ABI, stream, grid or UB; test injection entering production |
| P5 | A direct immutable JSON replay into the new DeviceCodeGen, without preparation; the actual generated MLIR passes independent syntax and semantic assertions and compiles through P2; unsupported inputs reject | Golden modification; an implementation-derived oracle; manual Late text, a fallback or a prebuilt substitution |
| P6 | A normal HIVM `tilelang.compile`; a recorded new codegen and compiler invocation and artifact identity that excludes the P4 cache; an NPU all-element compare; AscendC, PTO and Triton regression evidence | A stale binary or cache, manual MLIR or a fallback; an absent numerical assertion or no actual new compilation |
| P7 | Optional multi-tile N = 2**21 after P6, with an independently frozen contract | Premature scope expansion |

Numerical policy for P3, P4 and P6: exact elementwise F32 equality for finite, exactly representable
deterministic inputs and independent A + B. This does not claim signed-zero bit equality. The team
must fix and justify a broad random tolerance and exceptional-value policy before execution, and
must never adjust it after a mismatch. Guards and repetition provide evidence, not proof of all
possible races or OOBs.

Frozen semantic negative requirements: reject UB `0`/`16384`/`32768` interpreted as bytes; DMA
`4096` / slot `32768` / arena `163840`; a GM offset of j-only or block × `16384` / grid `32`;
extra or descriptor device args, or a wrong symbol or order; missing AIV or VF markers; missing
pb32, an altered full-mask or zeroing; missing waits, wrong pipes or a wrong event; dropped cache4;
tensors, alloc requests, DPX, SIMT, MixedCV or additional tiles. Specific diagnostics are required
even where generic MLIR parsing succeeds.

Before deleting a conservative legalizer pass, freeze independent semantic assertions. Compare
addresses, payload, coverage and predicate, events, ABI and completion after each removal, with
pass invocation evidence. Do not infer minimality from pass names or from isolated one-op success.
Record the full argv and cwd, the container and environment, the source HEAD, the actual executable
hashes, the command and assertion exits, the stdout and stderr arrangement, and the original logs.
Source facts, isolated conversions, full compilation, load and launch, and numerical equality are
separate evidence levels.

## 4. Recovery evidence reconciliation

The resumed verifier ran a read-only recheck at 2026-10-12 12:44 CST. It found no drift in the
repositories, the Golden, the tool hashes, the prior evidence, the p0v2 summary, the source anchors
or the container. It independently confirmed that `build/bin/bishengir-opt` matches the recorded
p0v2 hash, while `build/install/bin/bishengir-opt` has the same size but a different hash and no
installed `bishengir-translate`. Install freshness therefore remains UNPROVEN. The verifier
deliberately did not rerun E0. P1–P6 remain NOT_RUN.

The recheck also found these documentation and evidence limitations. An obsolete handoff reference
was stale and is now corrected in `docs/pass-cut.md` to the real coordination checklist.
`team-gateway-evidence.json` is absent and remains unsynthesized. `docs-working-revision.json` is
retained as an older pre-edit snapshot, while `docs-working-revision-current.json` records the
current document hashes. p0v2 has no retained `*.exit` file, so its process exit code is not
independently persisted. These limitations do not alter the frozen expectations or the prior probe
outputs.

## Current recovery starting 2026-10-12T12:41:05Z

The existing accepted Golden invariants remain PASS by immutable input evidence. GV-03 remains
**FAIL against its frozen expected-key oracle**: `ub_offsets_bytes`, `dma_bytes_per_buffer`,
`buffer_spacing_bytes` and `grid` are absent in the accepted manifest. This is a preserved
auxiliary schema-coverage mismatch. It does not authorize anyone to alter the frozen manifest or to
revoke the user's accepted geometry. The Late contract must check those values from the original
TIR and JSON and from independent unit arithmetic. No old expectation is weakened or relabeled
PASS.

Lead recovery evidence:
`/home/s00653124/workspace/hivm-poc-evidence/p0-20261012-resume-124105/lead/recovery-ledger.json`
records the actual argv, exits and outputs. Three branch, HEAD, index and worktree checks PASS. The
JSON and TIR SHA checks PASS. The light container, CANN, Bisheng availability, Python 3.11.17 and
two NPU health checks PASS. These are Lead checks, not new independent compiler or numerical tests.
The E0 rerun, the p0v2 rerun and P1–P6 remain NOT_RUN. The current source inputs have no drift that
requires broad baseline reruns. The accepted E0 Triton log and the current
`PlayGround/test_add_triton/test_add.py:38-42` contain a launch and a tensor print, but no
all-element numerical assertion. Retain the user's E0 PASS as the accepted baseline; this evidence
cannot stand in for P3 or P6 numerical correctness. The TileLang E0 SIMD log records one passed
pytest. No script was changed or rerun to extend E0.

The current independent verifier read the Golden, git, the prior summaries and the compiler and
runtime source anchors, then failed at 12:45:51.187Z with an explicit safety-refusal error. Its
pre-refusal tool-call ledger is `lead/verifier-before-refusal.json`. The Lead also recovered the
actual returned tool outputs in `lead/verifier-result-excerpts.json`, with a compact tool-result
summary in `lead/verifier-recovery-check-summary.json`. Twelve read-only Bash calls returned
non-error tool results, including Golden hash and git checks and source-anchor spot checks. This
does not prove every assertion or the whole command chain exit: a navigation miss or a pipeline
breaks the chain. The Lead recovered no Write, no Edit and no final report from this verifier,
so current-revision independent acceptance is **PARTIAL / BLOCKED**. The existing independent p0v2
outputs above remain valid at their original evidence level. The Lead updates this recovery section
after stopping the verifier. The refused verifier did not author or approve these updates.

The current Challenger failed at 12:44:37.553Z with an explicit safety-refusal error, producing
**NO VERDICT / BLOCKED**, not APPROVE, REVISE or BLOCK on architecture. Both current failures have
matching native StopFailure audit inputs and exact session and agent `blocked` database rows. The
Lead sent a SendMessage to `verify`, which the system denied due to lack of verified name binding;
the Lead attempted no repeated send. Native TaskStop succeeded for both affected members. The
sanitized report and the preserved team identities are
`/home/s00653124/refusal-capture/hivm-p0-20261012T124105Z.E3UJQ9/p0-refusal-report.json` and
`p0-team-identities.json`. For these two observed events, detection succeeded. Universal refusal
detection and all internal wake coverage remain unproved. The requested-route dispatch
corroboration is saved separately, while the actual served-model and provider request-level
evidence remains NOT_VERIFIED / NOT_AVAILABLE.

The Lead's current Runtime source recheck is `lead/runtime-source-recheck.md`. The CANN 9.2 header
confirms the dynamic-UB ID 2 and the HostArgs parameter order. The compiled SDK and local structure
layout and the new HIVM ELF interoperability remain NOT_RUN. This source-only observation supports
existing-runtime reuse, not P3 or P4 runtime acceptance.

## 5. Independent review and closure

Earlier draft 1 recorded REVISE and corrected `hivm.hir.block_idx` to `hivm.hir.get_block_idx`, and
it added the module AIV and explicit pb32 requirements. That historical review does not approve
this revision. The current reviewer encountered gateway errors containing
`API Error: 400 claude-opus-5-5 refused this request (safety filter: refusal) via Claude Code`; the
verifier later encountered the analogous haiku / OpenCode Go error. These are gateway responses,
not architecture verdicts and not automatic tool-approval rejections.

Correction to the Lead's earlier status: the verifier did complete independent tests before the
later refusals. The Lead recovered a report SendMessage to `team-lead` at 01:23:29Z from the
verifier's transcript, but assumes no successful delivery to the current Lead. The Lead audited the
evidence files directly. The Lead recovered no final current Challenger APPROVE, REVISE or BLOCK.
The previously cited `team-gateway-evidence.json` is absent from the evidence directory, and the
Lead does not recreate it and does not treat it as evidence. Raw transcripts and configuration are
not distributed.

P0 remains BLOCKED by final independent review, actual served-model evidence, cache4 representation
and terminal DMA completion design. Recovery additionally found that the `build/bin` and
`build/install/bin` compiler hashes differ, so installed compiler freshness is unproven. The
recovery `challenge-resume` role then failed twice while reading the P0 materials, at
`2026-10-12T04:36:06.987Z` and `2026-10-12T04:41:17.297Z`, with
`API Error: 400 claude-opus-5-5 refused this request (safety filter: refusal) via Claude Code`; it
produced no Challenger verdict.
These gateway failures are not architecture verdicts. The team may hand off the documents with
accurate blockers; this is not P0 PASS and grants no P1 authorization. The minimal pass set remains
a candidate until full-kernel evidence. The existing high-level store cannot currently support a
faithful cache4 claim; numerical equality alone cannot waive that parameter. Stop at P0 for user
review.

## 6. Current session 2026-10-13 (re-authorized P0) — independent evidence

Evidence root: `/home/s00653124/workspace/hivm-poc-evidence/p0-20261013-late-pipeline/`. Roles:
`verify` (verifier-haiku) and `review` (challenger-opus). The team runtime records the
requested-route model binding. Actual served-model proof from the Magpie request logs remains
NOT_VERIFIED.

### 6.1 Verifier Task 1 — Golden integrity: PASS 55/55

Script `verify/verify_golden.py` (run with `python -I`, in-container through
`scripts/in-container.sh`, conda env `s00653124_tilelang`), log `verify/verify_golden.log`:
`total=55 pass=55 fail=0`, exit 0. Per-item ledger `verify/summary.tsv`. The script covers both
contract hashes asserted against the constants in `docs/goal.md` (not a repo-side expected file),
manifest field-by-field equality plus key-set equality, 21 text and 4 arithmetic TIR
corroborations, a JSON round-trip (`tvm.ir.load_json` + `assert_structural_equal` +
reserialization), the absence of the legacy `tests/tests` path, and the static `__file__`-based
ROOT in the freeze script.

Honest NOT_RUN: **T1.10 provenance replay is NOT_RUN**. The verifier did not run an independent
in-memory re-lowering comparison, because the freeze script does not run in place. The ledger
records this as NOT_RUN, not PASS. The end-of-session re-hash
(`verify/golden-rehash-end-of-session.log`, 16:50:07Z) confirms all five fixtures unchanged, so the
verification mutated nothing.

### 6.2 Verifier Task 2 — Compiler binary identity: RESOLVED

`verify/build-artifact-identity.json`, `build-vs-install-byte-diff.log`,
`single-byte-diff-context.log`, `build-install-runpath-evidence.log`, `path-tool-resolution.log`,
`binary-pin-note.md`. No rebuild, no install, nothing modified.

- The build↔install pairs differ by **exactly 1 byte** (a trailing `:` in RUNPATH); `readelf -d`
  confirms `$ORIGIN/../lib:` against `$ORIGIN/../lib`. The two files link the same code, so both
  SHAs denote one build.
- The container `PATH` resolves the tools to the **CANN 9.2 toolkit** (`bishengir-opt` sha
  `248b6c09…`), NOT the local build (`71ac4012…`). Any bare-name probe is a foreign-binary probe.
- `bishengir-toolkits/dev/bin/bishengir-compile` → `build/install/bin/bishengir-compile`.
- No `install_manifest.txt` exists; CMake records no install provenance.

This finding resolves the earlier "same size, different SHA ⇒ install freshness UNPROVEN" note to a
byte-level explanation, and it establishes the P1/P2 pinning rule (D019).

### 6.3 Verifier Task 3 — P0 acceptance matrix (draft)

`verify/p0-acceptance-criteria.md` — an independent, non-weakening elaboration of the P0 row of
`docs/acceptance.md`. Sections A (Golden immutability), B (Late-IR contract validity), C (pass-cut
correctness) and D (compiler binary identity). Explicit NOW / P1 / P2 / REVIEW windows, explicit
"what must NOT count" lists, and NOT_RUN discipline.

**Lead review complete:** `lead/lead-review-of-acceptance-matrix.md` — **ACCEPT with one required
correction and two tightenings.** The correction: C1 cites the *generic* driver's
`hivm::buildConvertToHIVMPipeline` and `hivm::buildOptimizeHIVMPipeline`
(`Tools/bishengir-compile/PassPipeline.cpp:98-107`), but the A5 route is RegBase, which calls
`hivm::regbase::buildConvertToHIVMPipeline` and `hivm::regbase::buildHIVMTensorOptimizations`
(`regbase/PassPipeline.cpp:501-502`); `buildOptimizeHIVMPipeline` is never called on that path
(grep: 0 hits). The tightenings: B4 must record that the D009 gap is the sharpest of the three (a
silent b8 at the Golden's mask length), and D3 must become a §F rule that a run without an
absolute-path and sha256 compiler identity is BLOCKED, not PASS. The Lead asked the verifier to
apply all three. **The verifier applied them (matrix Rev 2, 19745 B, sha256 `3658d8ba…`; Rev 1 was
16421 B / `2d327a8a…`), and re-verified every citation against the live source rather than copying
it.** The Lead accepted and fixed two verifier nits: the RegBase entry `buildBiShengHIRPipeline` is
at `regbase/PassPipeline.cpp:461` (the Lead review had said 459; `grep -n` gives exactly one hit at
461), and `docs/status.md` misspelled the generator as `freeze_golden_hvm.py` (actual
`freeze_golden_hivm.py`). The verifier also independently re-derived the D009 mechanism
(`getBitWidthFromAttr` at `HIVMAVE/Utils/Utils.cpp:864-880` initializes `-1` and only fills 8, 16
and 32 from `functionType`; `VL_BITS = 256*8 = 2048`) and confirmed the b32 target form exists
in-tree (`regbase-convert-llvm.mlir` CHECK lines for `intr.hivm.pge.b32 {mask_bit_width = 32}`).

### 6.4 Lead source findings this session

`lead/lead-source-trace.md` §15-§29. New or corrected:

- §16/§16.4 **D018**: `--enable-direct-hivm-lowering` does NOT reach the AVE optimizer guard in the
  `bishengir-compile` path; the "existing flag expresses the bypass" hypothesis is FALSIFIED.
- §17 **D012**: an exhaustive carrier search found no MTE3 store-side cache field in the HIVM
  dialect (`EvictionPolicy` is LoadOp-only; the raw `l2_cache_mode` is ND2NZOp-only; both are
  MTE2).
- §18 **D013**: the frozen E0-accepted AscendC reference has no terminal barrier either, so the
  AscendC path shares the question. The question is not Late-specific.
- §19/§19a **D004**: the target predicate `target_is_plain_ascend` is defined by the absence of
  `pto`, so an `hivm` key makes it match too; resolution, not registration, is what fails.
- §20-§27: the verified Late-IR syntax table, the VF marker, the authoritative stage cut, the HIVMC
  A5 chain, the ABI discriminator (bare-ptr: 3 args against 15), the D018 structural isolation, and
  the `goal.md` doc discrepancies.
- §28 **D009 sharpened**: `AVEPgePattern` derives the predicate width as `VL_BITS/numElems` =
  2048/numElems when `functionType` is absent, and it accepts only 8, 16 and 32. The Golden mask is
  `boolx256` → derived 8, a *legal* width, so the missing pb32 carrier is **silent** wrong-code (b8
  instead of b32), not a diagnostic. A 32-element mask would derive 64 and abort (PG-03, rc 134).
- §29 **D012 sharpened**: probe SD-03 sets `{l2_cache_ctl = 4 : i64}` on `hivm.hir.store`; it
  **parses cleanly**, but the `-convert-hivm-to-std` output is byte-identical to the cache0 case
  SD-01 apart from the function name — the libcall still gets `%c0_i32`. The compiler **silently
  discards** the attribute, which is the worst case: a clean compile can hide a cache0 binary.

### 6.5 Challenger verdict (independent)

**VERDICT DELIVERED.** The Challenger (`review`, challenger-opus) returned a full source-anchored
independent verdict by SendMessage. It does not write `.md` artifacts (its instruction forbids
report, summary and findings files), so `review/` is empty **by design, not by failure**; the
verdict is recorded in `docs/decisions.md` D020 and here. The verdict:

- **(a) Late-IR contract boundary: REVISE, with P1 advancement BLOCKED.** The boundary (primitive
  HIVM/HIVMAVE plus explicit addresses, sync, VF and metadata before legalization, with exactly one
  A5 HIVMC LLVM owner) is conceptually correct and source-supported, but the team cannot accept it
  while D012 cache4, D013 terminal completion, ABI, pb32 and VF preservation and execution evidence
  are unresolved.
- **(b) Pipeline cut point: BLOCK for the existing `--enable-direct-hivm-lowering` mechanism;
  REVISE for the architecture.** The Challenger independently confirmed D018. Because
  `buildLowerAVEPipelines` has only one call site, at `regbase/PassPipeline.cpp:223`, the correct
  minimal route is a dedicated, mutually exclusive Late entry in the existing RegBase driver (no
  new target kind). That entry skips normal HIR/HFusion, bufferization, planning, auto-sync, loop
  lowering, multibuffer and the AVE builder, retains the validated legalization, and runs exactly
  one `runHIVMCCompileA5`. Option (a), getter plumbing, is a narrow fix, not the cut; option (c),
  optimizer survival, is unsafe as an assumption.
- **(c) D012: BLOCK for P1** — the team may accept equivalence only with independent exact-target
  source or device evidence that cache4 and the emitted cache0 path are semantically equivalent for
  this exact store and target; numerical equality alone is insufficient. No such evidence exists.

The Lead re-verified two new Challenger source claims: `AppendDeviceSpec` is **not idempotent**
(`AppendDeviceSpec.cpp:142-151` warns and then overwrites the spec unconditionally), and the pge
width default also exists on the lowering path (`HIVMPgeOpLowering`,
`HIVMAVEToAVEIntrin.cpp:1419-1443`). The second claim independently confirms the §28 D009
mechanism. The Challenger ran no compiler and no hardware execution; its verdict is source-review
only.

### 6.6 Status of this session's P0 gate

Independent Golden acceptance PASS; binary identity resolved; acceptance matrix drafted and
Lead-reviewed (C1 corrected, B4 sharpened, §F PATH rule added); source trace complete; two
hypotheses falsified or narrowed; two contract hazards sharpened to proven silent-failure modes;
**independent Challenger verdict delivered**. **The P0 gate is still NOT closed**: the Challenger's
own verdicts are REVISE and BLOCK, not APPROVE. D012 (cache4 carrier) and D013 (terminal
completion) remain open hard gates, and the Late route needs a dedicated driver entry plus P2 pass
instrumentation before anyone can prove the cut point. No P1 authorization is granted. All P1–P6
rows remain NOT_RUN.
