# POC status

- Date: 2026-10-13
- Current stage: **P0 gate CLOSED by user adjudication (D021); P1 AUTHORIZED.** The user decided every
  open gate on 2026-10-13: D012 → option (a) narrow store-side cache carrier extension; D013 → option
  2, a discriminating experiment BEFORE P1 production work; pipeline cut → option (b) dedicated
  mutually exclusive Late entry; D004 and `AppendDeviceSpec` → implement per the Lead recommendation;
  P1 → authorized. The P0 gate closed by **user decision, not by a Challenger APPROVE**: the
  independent Challenger verdict (D020) stands as the review record and remains REVISE / BLOCK.
  D012 remains a hard gate until its closure condition is met; **D013's closure condition (the
  experiment reports) was met on 2026-10-13, so the D013 gate is closed**, with the device-layer
  question carried forward as a P3 obligation.
- Supersedes the 2026-10-12 "P0 BLOCKED" handoff. That block was caused by model-route safety
  refusals, not by a compiler finding.
- P1: **CLOSED on B5–B8, 2026-10-13. Final independent-verifier state: 31 PASS / 1 FAIL** (checker
  `fd5f0c078fcb`, run `20261013T025929Z`). B5 PASS, B6 PASS (all 8 negatives, each raising only its
  own code), B7 PASS (13 counts + D009 discriminated on the emitted intrinsic), B8 7 clauses PASS
  with **`B8.CACHE4` FAIL attributed to D012** — an open hard gate that D021 deliberately scoped out
  of P1; it is not an artifact correctness defect. Scope and design basis:
  `p1-20261013-late-kernel/lead/p1-scope-and-design-basis.md`; artifact
  `p1-20261013-late-kernel/impl/vadd_late.mlir` (sha256 `3b930509…06a99`); Lead adjudication and
  independent re-execution in `lead/p1-lead-verification.md`; checker falsification and repair in
  `lead/p1-checker-falsification.md`. P1 is hand-authored Late MLIR plus parser/verifier and negative
  tests only — **no NPU-IR production code, no rebuild, no install, no physical NPU run in this
  phase.** **P2 is NOT authorized: D012 is the gate.** P2–P6: NOT_RUN. P7: OPTIONAL / NOT_RUN.
- **P1 Lead pre-work (read-only probes, `p1-20261013-late-kernel/lead/diagnostics/`), 2026-10-13.**
  Findings that constrain the artifact and the B5–B8 checks. Each was verified with
  `bishengir-opt` (sha256 `71ac4012…5962`) through `scripts/in-container.sh`; each sets no
  acceptance verdict — the verifier must reproduce independently. **Item 4 below was subsequently
  found to be wrong and is corrected in place; the correction is itself a finding.**
  1. **The D009 mask-width defect is reproduced.** `ave.hir.pge <ALL> : vector<256xi1>` (no carrier)
     **verifies cleanly (exit 0)** and lowers to `pge.b8 {mask_bit_width = 8}`; the same shape with
     `{functionType = #ave.func_dist_type<pb32>}` lowers to `pge.b32 {mask_bit_width = 32}`. **Exit
     code cannot distinguish them** — this is the empirical basis for B7's requirement. Measured
     2×2: the carrier is **redundant** at a 64-element mask (2048/64 = 32 already) and
     **load-bearing** at 256 (2048/256 = 8), so **a B7 discriminating pair exists only at the
     256-element width**.
  2. **The Golden hoists one mask; the canonical template does not.** The Golden defines
     `pset(32,"PAT_ALL")` once before the loop (`golden_codegen_input.tir:30`) and reuses it, while
     the template emits a fresh `ave.hir.pge` per op inside the loop (~256 vs 1). The hoisted
     single-pge form verifies and lowers to b32, so B7's count-equality can close with pge count = 1.
  3. **The terminal barrier spelling is legal.** `hivm.hir.pipe_barrier[<PIPE_ALL>]` immediately
     before `return` on a device-entry function verifies (exit 0), with the module/entry attributes
     of design decision 1. Precedent: `enable-multi-buffer-while.mlir:66-67`.
  4. **B8's grid and UB arena are not entry-function fields.** Grid is the function attribute
     `hacc.block_dim = 64 : i64`. **CORRECTION (Lead, later the same day): the UB-arena claim in this
     item was WRONG.** I originally recorded the 196608 arena as the device spec's `UB_SIZE` entry,
     citing `HACCAttrs.td:309`. That line is the **doc-comment example** of
     `HACC_TargetDeviceSpecAttr`, not the target's spec, and the real pass emits a different value:
     `-hacc-append-device-spec=target=Ascend950PR_9579` produces
     `#dlti.dl_entry<"UB_SIZE", 2031616 : i32>`, from `NPUTargetSpec.td:224`
     (`Ascend950_BaseSpec::UbSize = 2031616; // bits = 248KB`). The two are **different quantities in
     different units** — 196608 **bytes** is this kernel's dynamic arena (a *launch* parameter,
     `ascend_module.cc:482-489`), while `UB_SIZE` 2031616 **bits** is hardware capacity
     (`PlanMemory.cpp:2934-2936` divides by `numBitsInByte`). Writing 196608 into the spec asserts a
     falsehood, and a preprovided spec is overwritten anyway (`warning: Overwriting the device
     spec...`, `AppendDeviceSpec.cpp:142-151`). **SECOND CORRECTION (Lead, same day, ~09:46): the
     "no device-IR carrier" conclusion below was ALSO WRONG and is retracted.** I had grepped
     `AscendNPU-IR` for `dyn_shared_memory_buf`/`dyn_shared`, found zero hits, and read zero
     *consumers* as zero *carriers*. The Golden's own prepared TIR carries the arena as a **PrimFunc
     attribute** at the line that defines the Late boundary: `golden_codegen_input.tir:8`,
     `"dyn_shared_memory_buf": T.int64(196608)`. Measured on a one-line patch of the artifact adding
     `dyn_shared_memory_buf = 196608 : i64` to the entry func attributes: bare parse rc=0/stderr 0 with
     the attribute present; `-ave-normalize-ops -convert-hivmave-to-ave-intrin` rc=0/stderr 0 emitting
     `pge.b32`/`mask_bit_width = 32`; `-hacc-append-device-spec=... -convert-hivmave-to-ave-intrin`
     rc=0 with the attribute surviving. So **B8's `dynamic UB=196608` clause IS satisfiable from the
     parsed IR, no frozen criterion needs rewording, and D022 is WITHDRAWN** (not awaiting user
     adjudication). The implementor added the attribute at 09:48 (artifact sha256
     `3b930509bcb3dcda37dc9f3c37da04ac2c5eb99ade63454d35c450df3b706a99`). Independent corroboration:
     the verifier's own baseline, written 09:42 *before* this was found and from the Golden rather than
     from my guidance, already carried `dyn_shared_memory_buf = 196608`. What **survives** from the
     correction above: the device-spec `UB_SIZE` is capacity in bits, a different quantity — 196608
     must not be written there, and a checker must not accept the spec's `ub_size` key as the arena.
     Separately, a **partial** device spec (only `UB_SIZE`) makes `-convert-hivmave-to-ave-intrin`
     **segfault (exit 139, deterministic, 5/5)**, so any preprovided spec must be complete. Detail:
     `lead/p1-lead-verification.md` §4, §6, §8; `lead/p1-lead-spotchecks.md` §4;
     `lead/diagnostics/README.md` fourth probe.
- **D012 mechanism fully explained (Lead, read-only), which makes the authorized extension
  concrete:** `StoreOp` declares **no cache field at all** (`HIVMDMAOps.td:183-186`), so
  `{l2_cache_ctl = 4}` parses only via the assembly format's discardable `attr-dict` and is
  discarded; the store template **hardcodes** `l2_cache_ctl = 0` in `store_ubuf_to_gm_intrin_core`
  (`RegBase/DMAUtils.h:277`). The fix shape is source-supported: mirror the raw-value
  `OptionalAttr<I32Attr>:$l2_cache_mode` that `ND2NZOp` already carries (`HIVMDMAOps.td:396`) onto
  `StoreOp`. **P1 does not implement this** — P1 records the absence so it is visible. Recorded in
  `decisions.md` D012 and `lead/p1-lead-spotchecks.md` §2a.
- **A third unrepresentable argument, benign (Lead, read-only):** the Golden's `vadd` carries
  `"MODE_ZEROING"` (`golden_codegen_input.tir:36`), but `ave.hir.vadd` declares only
  `lhs/rhs/mask` (`HIVMAVEOps.td:61-74`). The template fixes the mode —
  `REGISTE_BINARY_VV(vadd)` → `MODE_ZEROING`, the only `vadd` registration tree-wide
  (`VecOpI64.cpp:598`). So the Golden's value is reproduced **by construction**: no IR change is
  needed. It is recorded anyway because it is the third instance of one question — *the source-level
  argument has no Late carrier; what happens to it?* — with three different answers: D009 (default
  **wrong**, a defect), D012 (default **wrong**, a gap), this one (default **right**, benign). A
  trace silent about all three reads as if all three were preserved. `lead/p1-lead-spotchecks.md` §3.

## P1 — hand-authored Late MLIR, acceptance against frozen B5–B8 (2026-10-13)

**Stage: P1 artifact DELIVERED; the independent verifier's checker now runs end-to-end and its B5–B8
report is the evidence of record. Final state: 31 PASS / 1 FAIL — the single FAIL is `B8.CACHE4`,
attributed to D012 (an open hard gate that D021 scoped out of P1), not to an artifact defect.
B6's discrimination is restored and enforced by a gate that would catch its own regressions.
P1 is closed on B5–B8; D012 remains the gate that blocks P2.**

Artifact: `p1-20261013-late-kernel/impl/vadd_late.mlir`, sha256
`3b930509bcb3dcda37dc9f3c37da04ac2c5eb99ade63454d35c450df3b706a99` (274 lines, 2026-10-13 09:48).
Hand-authored Late MLIR only: **no NPU-IR production code, no rebuild, no install, no NPU run.**
Golden TIR/JSON re-hashed this session and unchanged (`4194733b…12a18` / `576fa7a3…c0fe5`).

### Per-check verdicts (Lead adjudication of the implementor's report + Lead's own re-execution)

| Check | Verdict | Basis |
|---|---|---|
| **B5** parse + verify | **PASS** | `bishengir-opt <artifact>` → exit 0, stderr 0 bytes, stdout round-trips `@main_kernel` with real `hivm.hir.*` ops (4797 bytes; not a usage print, not `module {}`). |
| **B6** negatives rejected | **PASS** (verifier, checker `fd5f0c078fcb`, run `20261013T025929Z`; Lead-reproduced) | All 8 negative cases rejected: `B6.N01` by `bishengir-opt` itself (exit 1, specific diagnostic); `B6.N02`–`N08` by the independent checker, **each raising only its own code** — Lead-re-ran the isolation check over the verifier's own parsed reprints and confirmed: N02→[B5.MODCORE], N03→[B8.ENTRY_ATTRS], N04→[B8.UB_UNITS,B8.ABI_ROLE], N05→[B7.D009_CARRIER], N06→[B8.SYNC], N07→[B8.ABI], N08→[B8.SYNC_ORDER]. N04's extra `B8.ABI_ROLE` is a Lead-verified **true downstream consequence** of its own byte-unit error, not contamination. `BASELINE.GATE` enforces that the valid baseline is accepted with **zero** diagnostics. |
| **B7** op-by-op + D009 | **PASS** | All 13 Late-op counts equal the Golden's, recounted independently from the parsed IR. D009 discriminating pair, run this session: artifact → `intr.hivm.pge.b32` / `mask_bit_width = 32`; one-token mutation (carrier removed) → `intr.hivm.pge.b8` / `mask_bit_width = 8`. **Both exit 0, stderr empty** — the assertion, not the exit code, distinguishes them. `B7.D009_EMITTED` passes at `stage=attempt1_as_parsed` (`pge.b32=1`, `mask_width=32`, `pge.b8=0`), so no fallback stage was used. `B7.INVENTORY` passes under the adjudicated trip-count + induction-variable predicate. |
| **B8** preservation from parsed IR | **7 clauses PASS, 1 FAIL (`B8.CACHE4`)** | PASS: A/B/C ABI (`3 × memref<524288xf32, gm>`), grid `hacc.block_dim = 64`, arena `dyn_shared_memory_buf = 196608`, spacing `{0, 65536, 131072}`, both sync pairs (2 set + 2 wait, order resolved through the VF call site), GM element addressing (`get_block_idx` + `muli 8192` + `subview [8192][1]`), module attrs (`hacc.target`, `hivm.module_core_type<AIV>`), entry attrs. **`B8.CACHE4` = FAIL**: the Golden's store `l2_cache_ctl = 4` is **not** carried — `StoreOp` has no cache field and `{l2_cache_ctl = 4}` is silently discarded. This FAIL is the **visibility D021 asked P1 to produce**; it is attributed to **D012** (open hard gate), **not** to an artifact correctness defect. Observation: terminal `pipe_barrier[<PIPE_ALL>]` (D013 addition, not one of the four Golden ops). |

### Lead findings raised during P1 (all in `lead/p1-lead-verification.md`)

1. **The verifier's frozen `stage_hazard` observation was falsified.** It claimed `hacc.target`
   makes `-convert-hivmave-to-ave-intrin` exit 1 and adding a spec segfaults. Measured 4-arm matrix:
   `hacc.target` alone → rc=1/stderr **0** (silent); `hacc.target` + **partial** spec → **139**;
   `hacc.target` + **complete** spec → **0**; complete spec alone → 0. The discriminator is a
   **complete** spec, not `hacc.target`. This mattered: the frozen fallback would have stripped
   `hacc.target` and asserted D009 against a mutated IR. Flagged before the verifier ran it.
2. **`B8.SYNC_ORDER` must resolve the VF outline through the call site, not by line number.** The
   artifact outlines the vector body into `@main_kernel_simd_vf_0` called at line 158; a literal
   textual-position check would see every `ave.hir.*` op *after* `set_flag[V,MTE3]` and fail a correct
   artifact. Execution order through the call is correct.
3. **The `B8.UB_ARENA` escalation (D022) was MY ERROR and is retracted** — see pre-work item 4 above.
   The arena carrier is the Golden's own `dyn_shared_memory_buf` PrimFunc attribute; it is satisfiable
   and now present.
4. **The implementor resolved my two open sub-questions by differential experiment, both reproduced:**
   `vector_layout` is **not** a width carrier (256-wide pge + layout, no `functionType` → still
   `pge.b8`); and `masked_store`'s `functionType` is **inert** (byte-identical output with and without
   it, in the correct trailing position). Not writing either is correct.
5. **A partial device spec segfaults the conversion pass** (exit 139, 5/5) while a complete one runs —
   so any preprovided spec must be whole. Argument for keeping the append step in P2.

### Verifier checker falsified — the independent B6 evidence does NOT yet exist

The verifier's checker `verify/p1_check.py` (revision sha256
`a8307c1ad4e9f4e04f08b32189b9436cef373f58f62a2f77db2141d1363a7ce5`) has **no discriminating power**.
The Lead ran the checker's own semantic assertion set, unmodified, over three inputs; it raised the
**same 24 diagnostics** on all of them:

| Input | Bare-parse rc | Semantic diagnostics |
|---|---|---|
| `baseline/valid_baseline.mlir` (must raise **nothing**) | 0 | **24** |
| `impl/vadd_late.mlir` (the correct artifact) | **0** | **24** |
| `negatives/n02..n06, n08` (must each raise its **own** code) | 0 | **24** |

**Consequence: B6.N02–N07 as reported are FALSE PASSES**, and a positive run over the correct
artifact would FAIL it. N08 is reported FAIL for the same reason — `B8.SYNC_ORDER` is the one code
the blind pile happens not to contain. Three independent defects, all reproduced:

- **(A)** `ParsedIR.entry_body_lines` is **always exactly 1 line**: the body scan starts after the
  `func.func` line with `depth = 0` but the opening `{` is at the end of that line, so the first
  brace-free body line breaks the loop. Measured buggy=1 vs correct=36 (baseline) / 24 (artifact).
  Hence `pointer_cast=0` (should be 3), `loads=0` (2), `stores=0` (1), `flags=[]` (4) on **every**
  input — which is why it reports "found 0" for all four Golden flags.
- **(B)** Attribute **values** are never read: `RE_ATTR_UNIT` stores the key with the placeholder
  `"<unit>"`. On the artifact's own parse `hacc.function_kind='<unit>'` (actually `DEVICE`),
  `hivm.func_core_type='<unit>'` (actually `AIV`), `hacc.target='<unit>'` (actually
  `"Ascend950PR_9579"`). Independent of (A): fixing (A) alone still false-fails B5.MODCORE,
  B8.TARGET, B8.ENTRY_ATTRS on a correct artifact.
- **(C)** Latent: the artifact **outlines** the VF, so `ave.hir.*` occurs 0 times in
  `@main_kernel`'s body and 5 times file-wide. `check_sync`'s `vec_pos` comes from
  `entry_body_lines` only → `-1` → the `waitMTE2<vector` and `vector<setV` guards never fire, so
  B8.SYNC_ORDER would report PASS **vacuously** even after (A) and (B) are fixed.

`BASELINE.GATE` did not catch this because it asserts only that the baseline *parses*, never that
the semantic set raises **zero** diagnostics on it. The D009 self-test is genuinely discriminating
and does pass (two outputs differ only in the function name and `pge.b32`/`32` vs `pge.b8`/`8`,
both rc=0) — it works on compiler output text, not on `ParsedIR`.

Full write-up: `lead/p1-checker-falsification.md`; reproduction scripts and the frozen checker
snapshot under `lead/diagnostics/checker-falsification/`. **No frozen assertion was weakened and
`expected.json` was not edited.**

**Repair progress (Lead re-verified after each revision).** The verifier fixed A–D across four
rounds and has now landed **both** Lead-required fixes. Current state (run `20261013T025223Z`,
checker `7bac9dbe0a68`):

- **`BASELINE.GATE` now runs the full semantic assertion set over the canonical reprint and requires
  zero diagnostics** (`p1_check.py:1757-1805`). Required fix (4) is **closed**. This is the gate whose
  absence let five defects survive.
- **`B7.INVENTORY` uses the adjudicated predicate** (`p1_check.py:1049-1120`): trip count == 128
  **and** the induction variable wired to the per-iteration subview offsets, either spelling accepted,
  the literal `[0,128)` kept only as a reported observation. **The artifact now PASSES it.** The same
  predicate immediately caught a **real bug in the verifier's own baseline** — its loop IV was dead
  (all three offsets constant `%c0`, all 128 iterations re-reading one tile) — which the inverted
  literal check had waved through. That is a strengthening demonstrated on a genuine defect.
- All seven semantic negatives were re-derived from the corrected baseline and **each raises only its
  own code**; `frozen_fixture_sha256.txt` is re-recorded; `expected.json` is **untouched** (mtime
  09:40, 18842 bytes).
- `B7.D009_EMITTED` passes at `stage=attempt1_as_parsed` (`pge.b32=1`, `mask_width=32`, `pge.b8=0`),
  so the falsified `stage_hazard` fallback did **not** fire. `B8.SYNC_ORDER` is **non-vacuous**
  (resolves the outlined VF through its call site at entry body line 17).

**One check must remain FAIL.** `B8.CACHE4` currently PASSes via an "arm (b): the artifact documents
the absence" escape hatch — a Lead instruction **retracted before that revision was written**
(`p1-checker-falsification.md` §7d/§7f). It is a keyword grep over the artifact's comment prose, and
a **three-line fake comment** satisfies it, which is exactly the "silent pass" the frozen rule's own
final sentence forbids. Revert ordered; the correct verdict is **FAIL attributed to D012**, not to
the artifact and not to the checker. Expected final state: **31 PASS / 1 FAIL**.

The two adjudications, recorded for the file:

- **`B7.INVENTORY` (loop literal) — adjudicated: the literal check is INVERTED.** It *accepted* the
  verifier's own baseline, whose induction variable `%arg3` was declared and then **never referenced**
  (0 mentions in the body; all three subview offsets the constant `%c0`, so all 128 iterations
  re-read the same 64 elements), and *rejected* the artifact, whose offsets **are** the induction
  variable (offsets `{0,64,…,8128}`, trip count 128 = the Golden's `i*64`). Measured, not argued:
  `lead/diagnostics/.tmp/probe_loop_adjudication.py` runs both predicates over both reprints and they
  select **opposite** fixtures. The frozen requirement says "**counts equal**" and its own inventory
  row records `golden_count: 1` for `for i in range(128)`. Adjudicated: assert `(hi−lo)/step == 128`
  **and** that the induction variable is wired to the offsets; accept either spelling. **Implemented
  by the verifier; the artifact passes; the predicate caught the real dead-IV bug above.**
- **`B8.CACHE4` — NOT a criterion defect. The FAIL is CORRECT and stands.** The frozen rule
  (`expected.json` B8 assertions[12]) requires the store cache mode to be *"recorded explicitly in
  the artifact"* **and** the checker to report what is carried. The checker satisfies the second half.
  The artifact cannot satisfy the first: `StoreOp` has no cache field and `{l2_cache_ctl = 4}` is
  silently discarded (D012). **The Lead first adjudicated this as a checker defect and RETRACTED that
  adjudication** — it had reasoned that the artifact's N1 comment block satisfied the first conjunct,
  but measured that comments **do not survive the canonical reprint** (source: 190 comment lines, 3
  `l2_cache` mentions; reprint: **0 and 0**), so the escape hatch was unverifiable by construction,
  and documenting an absence is not recording the mode. The verifier nonetheless implemented the
  withdrawn instruction as a comment-keyword grep; a **three-line fake comment satisfies it**
  (§7f), so the revert is ordered. This FAIL **is** the visibility D021 asked P1 to produce; it is
  attributed to **D012**, not to the artifact being incorrect. It must be carried in any P1 result
  and must not be cleared by relaxing the rule.

**Both round-1 requests are now closed:** `BASELINE.GATE` runs the full semantic assertion set over
the canonical reprint and requires zero diagnostics, and `logs/frozen_fixture_sha256.txt` has been
re-recorded (checker `7bac9dbe0a68`).

### P1 evidence of record

- **Run:** `p1-20261013-late-kernel/verify/logs/run-20261013T025929Z/` — `summary.tsv` (32 rows),
  `result.json` (machine-readable), `baseline.semantic.txt` (`<zero diagnostics>`), `b5.out`/`b5.rc`,
  and per-fixture `N0*.parsed.mlir` reprints.
- **Checker:** `verify/p1_check.py` sha256 `fd5f0c078fcbfeb22f4f1739826ee707eb1a314edaa5bcc730badafb7c39da3d`
  (74597 bytes, 10:57:40). Frozen-fixture hashes: `verify/logs/frozen_fixture_sha256.txt`.
- **Oracle:** `verify/expected.json` sha256 `a64a0d02e0042deba4562a3817cea7916076012152569ea4a68885da92cb0f9e`,
  **unmodified** (mtime 09:40:15, 18842 bytes) — no frozen assertion was weakened at any point.
- **Artifact:** `impl/vadd_late.mlir` sha256 `3b930509bcb3dcda37dc9f3c37da04ac2c5eb99ade63454d35c450df3b706a99`,
  unchanged throughout.
- **Lead-side falsification and adjudication:** `lead/p1-checker-falsification.md` (§7c/§7c-i loop
  adjudication, §7d/§7f the retracted CACHE4 adjudication and its revert); probes under
  `lead/diagnostics/.tmp/` (`probe5.py`, `probe_loop_adjudication.py`, `probe_b6_isolation.py`).
- **Verifier's written report: RECEIVED 2026-10-13 03:06Z (gap CLOSED).** The formal B5–B8 report
  arrived after three requests and closes the documentation gap previously recorded here. It states
  31 PASS / 1 FAIL / 0 NOT_RUN / 0 BLOCKED, checker `fd5f0c07…39da3d`, `expected.json` UNTOUCHED, and
  attributes the single FAIL to D012. It independently confirms the four items the Lead required:
  (1) no frozen assertion weakened; (2) the seven semantic negatives re-derived from the corrected
  IV-wired baseline and each isolated to its own code; (3) `BASELINE.GATE` enforcing the full
  semantic set at zero diagnostics; (4) `B8.CACHE4` FAIL attributed to D012, not to an artifact
  defect. This agrees with the Lead's independent reproduction in every particular, so the gap is
  closed as a documentation matter only — the verdicts above stand on the machine-readable evidence,
  and the report adds no new claim that was not already reproduced.

### Still open after P1

- **B6 evidence now exists and is enforced.** The checker was falsified by the Lead, repaired through
  five rounds, and both Lead-required fixes have landed: `BASELINE.GATE` runs the full semantic
  assertion set over the canonical reprint and requires **zero** diagnostics (`p1_check.py:1757-1805`),
  and the negatives were re-derived so each raises only its own code. Final run `20261013T025929Z`.
  The written verifier report arrived 03:06Z and is filed above; no item of record remains open on
  the B6 evidence.
- **D012 (cache4) remains a HARD GATE and is the P2 blocker.** P1 makes the drop visible — the
  `B8.CACHE4` FAIL is that visibility — but P1 does not implement the carrier. P2 does not start until
  the narrow store-side carrier extension lands and is reviewed.
- **D013's device-layer question** (whether the hardware supplies terminal completion) is a **P3**
  obligation; P1 carries the barrier explicitly.
- **The arena's *runtime* forwarding** (`dyn_shared_memory_buf` → `kAclLaunchKernelAttrDynUbufSize`)
  is unproven at P1 — the IR-level preservation is proven, the runtime half is a P2 launch-ABI check.
- **A4 (the VF outline is REQUIRED, not the author's choice) is an unproven assumption**, and it is
  load-bearing: `B8.SYNC_ORDER` now resolves ordering *through the call site*, which presupposes the
  outline. The Lead's inlining experiment was INCONCLUSIVE (the text-surgery splices were malformed),
  so the assumption stands unproven rather than refuted. `lead/p1-lead-verification.md` §7a.
- **No numerical correctness is claimed.** Nothing was compiled to a binary or run on a device.

## D013 experiment — EXECUTED, VERDICT: DROP CONFIRMED

The D013 mechanism was re-derived from local source this session and the experiment definition was
frozen **before execution**, per the user's decision (D021 option 2) and `CLAUDE.md`.

- **Frozen definition:** `lead/d013-experiment-frozen-20261013.md` — fixture, 7 arms (A1 baseline,
  A2 GSS, A3 inject-normal, A4 inject-barrier-all, A5 GSS-without-`hacc.entry`, A6 GSS-with-
  `vector_function`, A7 to-LLVM), the exact invocation and the predicted result per arm, plus the
  §5 discriminating table. Written by the Lead; executed by the verifier.
- **Addendum (written before any arm executed):** `lead/d013-addendum-1-counter-case.md` records an
  in-tree counter-case found after freezing — `hivm-gss.mlir`'s reg-based `@dotdot_mix_aic`
  (`Ascend910_9589`) has its terminal-barrier expectation **commented out**, while its mem-based
  sibling asserts it. The source path still predicts A2 = HAS BARRIER, so the two signals disagree.
  The addendum changes no prediction and no arm; it fixes how a refuted A2 must be reported (a
  refuted prediction is the finding, not a near-miss).
- **Mechanism (re-derived from source, corrects the earlier flat citation).** The terminal `PIPE_ALL`
  barrier is a **GraphSyncSolver artifact, not a legalizer artifact**: `insertBarrierAllBeforeReturn`
  (`GraphSyncSolver/SyncSolverCodeGen.cpp:923-933`) inserts it before the return of a device-entry
  function, gated on `isIntraCoreMode()` (`:975-982`, always true for the pass's hardcoded
  `INTRA_CORE_SYNC`, `GraphSyncSolver.cpp:76`) and on the `hacc.entry` attribute
  (`HACC/IR/HACCInterfaces.cpp:151-154`). The pass returns early for `hivm.vector_function`
  (`GraphSyncSolver.cpp:70-72`). `InjectSync` adds the same barrier in `BARRIERALL` mode
  (`InjectSync.cpp:50-67`) and in `NORMAL` mode too, via
  `SyncAnalyzer::InsertLastPipeAll` (`InjectSync/SyncAnalysis.cpp:103-119`, called from `Plan()` at
  `:53-54` with `insertBarAllAtLast` defaulting to **true** at `InjectSync/SyncAnalysis.h:56`), also
  gated on `isDeviceEntry`. So the barrier has three emitters. The A5 regbase stage picks GSS by default
  (`HIVMRegbasePipelines.cpp:115-131`; `bishengir-compile/Options.td:202-206,576-579`). Upstream
  `ReturnOpLowering` adds no barrier
  (`third-party/llvm-project/mlir/lib/Conversion/FuncToLLVM/FuncToLLVM.cpp:678-719`). **The Late cut
  skips GSS and InjectSync, so it drops the epilogue; the conversion does not restore it.**
- **Correction to a prior record.** The frozen `ascend_reference.asc` is AscendC **source** (1848
  bytes of `__simd_vf__`/`__global__` C++), **not** compiler output. Its lack of a terminal barrier
  is therefore **not** evidence about emitted code. The earlier inference that the AscendC path
  "shares the question, so the device layer likely supplies it" is **withdrawn as a basis for
  de-risking D013**. This is recorded in `decisions.md` D013, `pass-cut.md`,
  `late-ir-contract.md` §5 and `lead-source-trace.md` §18a.
- **Scope limit:** the experiment tests the MLIR pass layer only. Device/runtime completion stays a
  P3 hardware question. The four Golden flag/wait ops are untouched by this experiment.
- **Result: EXECUTED and reported. Verdict: the drop is CONFIRMED at the MLIR pass layer.** Final
  verdicts against the frozen predictions: **A1 PASS, A2 PASS, A5 PASS, A6 PASS, A7 PASS**;
  **A3 — prediction REFUTED, observed result genuine** (NORMAL-mode `InjectSync` *does* emit the
  terminal barrier, via `SyncAnalyzer::InsertLastPipeAll`; the frozen F11 was wrong);
  **A4 — NOT_RUN** (the bare `-hivm-inject-sync{sync-mode=barrier-all}` form is swallowed by the tool,
  which prints usage and exits 0; the frozen arm was not re-frozen). Two **post-freeze** arms were
  added, each with its prediction frozen before it ran: **A4-fix PASS** (the corrected
  `-pass-pipeline=` spelling emits 12 `PIPE_ALL`, last before `return` — confirms F10) and **A8 PASS**
  (GSS → `-convert-hivm-to-std` → `-convert-hivm-to-llvm` preserves exactly one
  `hivm.intr.hivm.BARRIER` with `pipe = 6` immediately before `llvm.return` — the conversion
  *preserves* a barrier a sync pass created; it simply never creates one). §5 discriminating-table row
  **`HAS / NO / NO` — Drop confirmed**. **The A3 refutation strengthens D013**: the terminal barrier
  has three emitters (GSS; `InjectSync` `BARRIERALL`; `InjectSync` `NORMAL`), all gated on
  `hacc.entry` + DEVICE and all skipped by the Late cut. **P1 obligation:** the Late IR tail must
  carry the terminal completion explicitly, or the drop must be justified by independent device-layer
  evidence; the four Golden `set_flag`/`wait_flag` ops are preserved verbatim. **Device/runtime
  completion is carried forward as an explicit P3 obligation.** D013's closure condition (the
  experiment reports) is met. Evidence: `lead/d013-result-20261013.md`; verifier evidence under
  `verify/d013/` (`summary.tsv`, `report.md`, three run snapshots, `binary-pin.log`).
- **Run-3 process incident (disclosed by the executor, re-verified by the Lead).** A per-run selector
  `D013_ONLY` was passed as an environment variable, but `scripts/in-container.sh` forwards only
  `HOME` and `HIVM_CONDA_ENV` to `docker exec`, so it was silently dropped and **all arms re-executed,
  including A3, which the Lead had forbidden re-running**. The Lead independently re-verified that no
  evidence value changed: A3's observed string and `FAIL` verdict are identical in all three runs
  (only a `note=` field was added), A1/A2/A4/A5/A6/A7 are identical between runs 2 and 3, `logs/A7.out`
  is byte-identical to `logs/A7.run2.out`, and the fixture still hashes to the frozen spec §3 value.
  A guard was added inside `verify/d013/` (an arm with a row is never re-run unless named in
  `D013_RERUN`). **General hazard for P1/P2: any per-run selector must be carried by a file or an
  explicitly forwarded variable — the container wrapper drops all other environment variables without
  warning.** Details: `lead/d013-result-20261013.md` §9.

## Repository ledger

| Repository | Branch | HEAD | Observation |
|---|---|---|---|
| TileLang | `feat/hivm-backend-poc` | `cc56643de04a49db6f50875e26d904b0c69e71a6` | Clean index/worktree. Golden commit present. |
| AscendNPU-IR | `feat/tilelang-hivm-late-pipeline` | `e878f34927fb5b77fd8c7257fda3a95a3560f2ec` | Clean index/worktree. Existing build dir retained. |
| Reference tilelang-mlir-ascend | `main`, read only | `417dae9908ad2ea5ffb74294a7075b6cb64b4f2b` | Clean; no edits. |

## Task A — Golden freeze: **CONFIRMED COMPLETE, no action taken**

The five fixtures are already at the canonical `tests/golden/hivm/vadd_f32_single_tile/`; the old
`tests/tests/...` path does not exist. No migration or regeneration was performed or needed.

| Fixture | SHA-256 |
|---|---|
| `golden_codegen_input.json` | `576fa7a37bcaf1748e387efbd26508652e2e4b18790fa21aea874a7b756c0fe5` (matches contract) |
| `golden_codegen_input.tir` | `4194733b8412484c9bb8caf74ba3285348345f92ad61684784fe8c3a79e12a18` (matches contract) |
| `ascend_reference.asc` | `c025961ada28868bf3c1d4af70d550c2742109a92a59d190aa61055047d776c8` |
| `device_mod.tir` | `0fc125b511c70be4c12e6f2dc830b6a134aa2a0bc8f8b07a2e8dd8e3922df3df` |
| `manifest.json` | `42dda67a5694530f8efa878f27cdbb4d7530dc93f0cdd72d0afef162b1c7b221` |

Independent verification (Verifier, isolated script): **55/55 PASS, exit 0**. Covers the two contract
hashes, manifest invariants, geometry cross-checks, JSON round-trip via `tvm.ir.load_json` with
`assert_structural_equal`, absence of the legacy path, and `__file__`-relative ROOT in
`freeze_golden_hivm.py`. Log: `verify/verify_golden.log`; script `verify/verify_golden.py`.
Verifier also re-hashed all five fixtures at end of session (`golden-rehash-end-of-session.log`,
16:50:07Z) — unchanged, so the verification itself mutated nothing. `summary.tsv` records per-item
status including one deliberate **NOT_RUN** (T1.10 provenance replay; the freeze script is not
executed in place).

## Task B — Branches: **CONFIRMED COMPLETE**

Both branches already existed and are clean; no commits, resets, cleans, pushes or remote changes.

## Task B2 — Compiler binary identity: **RESOLVED (Verifier Task 2)**

Evidence: `verify/build-artifact-identity.json`, `build-vs-install-byte-diff.log`,
`single-byte-diff-context.log`, `path-tool-resolution.log`, `binary-pin-note.md`.

- `build/bin/*` and `build/install/bin/*` differ by **exactly one byte** — the trailing `:` of
  `DT_RUNPATH` (`$ORIGIN/../lib:` vs `$ORIGIN/../lib`). They are the **same code**; the earlier
  "same size, different SHA ⇒ UNPROVEN" concern is resolved (D019).
- **Bare `PATH` inside the container does NOT resolve to the local build.** It resolves to the CANN
  9.2 toolkit (`/data/pri/cann-9.2.0/.../tools/bishengir/bin/`, opt sha `248b6c09…`). Any compile
  probe that used a bare tool name tested a foreign binary — including the Lead's first
  `--enable-direct-hivm-lowering` probe, which was therefore re-run against the explicit local path.
- `bishengir-toolkits/dev/bin/bishengir-compile` symlinks to `build/install/bin/bishengir-compile`.
- **Path resolution differs between host and container, and both are foreign.** On the host (outside
  the container) `command -v bishengir-opt` resolves to `/data/pri/cann-9.1.0/bin/bishengir-opt`; inside
  the container it resolves to the CANN 9.2 toolkit under `/data/pri/cann-9.2.0/...`. Neither is the
  local build. A probe run on the host without the container helper, or with a bare tool name, tests a
  different foreign binary again. This is a second independent reason for the D019 rule below.
- P1/P2 evidence must pin the **absolute path + sha256** of the local binary, never `command -v`.

## Task C/D — Source investigation: Lead trace complete; Challenger verdict DELIVERED

Lead evidence: `lead/lead-source-trace.md`. Independent Challenger verdict: **DELIVERED** (recorded
in `docs/decisions.md` D020; the Challenger writes no `.md` file by its own instruction, so `review/`
is empty by design). Summary: **contract boundary REVISE with P1 advancement BLOCKED; pipeline cut
BLOCK for the existing flag, REVISE toward a dedicated Late entry; D018 → option (b); D012 → BLOCK
for P1.** Two new Challenger source claims re-verified by the Lead **and independently concurred by
the verifier** (`verify/challenger-claims-check.log`): `AppendDeviceSpec` is **not idempotent**
(`AppendDeviceSpec.cpp:142-151` warns then overwrites unconditionally; non-idempotent whenever a
pre-existing spec differs), and the pge width default also exists on the lowering path
(`HIVMPgeOpLowering`, `HIVMAVEToAVEIntrin.cpp:1435-1440`, which *stamps* `mask_bit_width` and is a
second silent-accept site for the Golden's derived-8 case), independently confirming the §28 D009
mechanism. See `lead/lead-source-trace.md` §30.

Key established facts:

**Late-IR contract (high confidence):**
- The closest existing template is `bishengir/test/Dialect/HIVM/arith-constant-op-to-hivm-regbase-broadcast.mlir`,
  which establishes the canonical idiom for a regbase AIV GM->UB / vload / vadd / masked_store / UB->GM kernel.
- `hivm.hir.pointer_cast(<i64 BYTE offset>)` matches Golden byte offsets 0 / 65536 / 131072.
- Sync maps exactly: `MTE2_V` -> `hivm.set_flag[PIPE_MTE2, PIPE_V, EVENT_ID0]`;
  `V_MTE3` -> `hivm.set_flag[PIPE_V, PIPE_MTE3, EVENT_ID0]`.
- Function AIV metadata `hivm.func_core_type = #hivm.func_core_type<AIV>` is required (D009).
- ABI confirmed: three bare GM pointers; grid=64 and dyn UB=196608 arrive as launch attributes
  (`kAclLaunchKernelAttrDynUbufSize`), sourced from `tirx.kernel_launch_params` tail.

**D009 (predicate granularity) — MANDATORY carrier; omission is SILENT at the Golden's geometry:**
Golden `pset(32,"PAT_ALL")` is a **b32** predicate; the frozen `ascend_reference.asc` emits
`asc_create_mask_b32(PAT_ALL)`. The carrier is `functionType = #ave.func_dist_type<pb32>` on the
pge/masked_store ops. Mechanism re-derived from source this session: `AVEPgePattern`
(`AVENormalizeOps.cpp:587-631`) reads `functionType` via `getBitWidthFromAttr`; **if absent it derives
the width as `VL_BITS/numElems` = 2048/numElems and accepts only 8/16/32, else `report_fatal_error`.**
The Golden mask is `boolx256`, so 2048/256 = 8 is a **legal** derived width — omitting the carrier
yields **silent `intr.hivm.pge.b8 {mask_bit_width = 8}` wrong-code, not a diagnostic**. A 32-element
mask would derive 64 and abort (PG-03, rc 134); the Golden will not fail loudly. Hence P1 must assert
the emitted `mask_bit_width = 32` directly — "the compile succeeded" is not evidence. See
`lead/lead-source-trace.md` §28.

**D012 (cache4) — carrier EXHAUSTED and SILENT-DROP PROVEN, hard P1 gate:** `hivm.hir.store` has NO
cache operand (`HIVMDMAOps.td:157`). The `HIVM_EvictionPolicyAttr` is on `LoadOp` only (MTE2), and the
raw `l2_cache_mode` I32Attr is on `ND2NZOp` only (also MTE2) — there is **no MTE3/store-side cache
carrier** in the HIVM dialect. The store libcall lowering (`HIVMToStandard.cpp:1020-1032`) appends
only `atomicKind`; the RegBase C310 template hardcodes `l2_cache_ctl=0`. Yet the frozen AscendC
reference forwards cache4 faithfully as `asc_store_l2_cache_mode(4)`. **Probe SD-03 now proves the
attribute is silently discarded rather than rejected:** `hivm.hir.store ... {l2_cache_ctl = 4 : i64}`
parses cleanly, but `-convert-hivm-to-std` emits a libcall byte-identical to the cache0 case (SD-01)
apart from the function name — still `%c0_i32`. A Late-IR author could write cache4, see a clean
compile, and get cache0. Cache4 is real and must not be silently dropped. Carrier table:
`lead/lead-source-trace.md` §17.

**D013 (terminal DMA completion) — mechanism re-derived, experiment EXECUTED, verdict DROP CONFIRMED:** the
terminal barrier is a **GraphSyncSolver artifact**, not a legalizer artifact. `insertBarrierAllBeforeReturn`
(`SyncSolverCodeGen.cpp:923-933`) inserts `PIPE_ALL` before the return of a device-entry function,
gated on `isIntraCoreMode()` (`:975-982`, always true for the pass's hardcoded `INTRA_CORE_SYNC`) and
on the `hacc.entry` attribute; the pass returns early for `hivm.vector_function`. `InjectSync` adds the
same barrier in `BARRIERALL` mode and in `NORMAL` mode too (`SyncAnalyzer::InsertLastPipeAll`,
`SyncAnalysis.cpp:103-119`). **The Late cut skips both passes, so it drops the epilogue**,
and `ReturnOpLowering` does not restore it. **Correction:** the frozen `ascend_reference.asc` is
AscendC **source**, not compiler output, so its lack of a terminal barrier is **not** evidence about
emitted code; the earlier "the question is shared, so the device layer likely supplies it" inference is
withdrawn as a basis for de-risking. **Result:** the frozen pass-level discriminating experiment
(`lead/d013-experiment-frozen-20261013.md`, 7 arms) ran and reported — §5 row `HAS / NO / NO`, **the
drop is CONFIRMED** (`lead/d013-result-20261013.md`). Device/runtime completion is carried forward as
an explicit **P3 obligation**. See `lead/lead-source-trace.md` §18 and §18a.

**Pipeline cut point — hypothesis, NOT accepted:** `runHIVMCCompileA5` does the real HIVM->LLVM
conversion (via `runSIMDToLLVMCompile` -> HIVMC A5 `buildLowerToLLVMPipeline`). A name collision
exists between the regbase and HIVMC-A5 `buildBiShengHIRAVEToLLVMPipeline`. The Late route must
skip the HIR/HFusion scheduling and final HIVM allocation/scheduling stages and run exactly one
HIVM->LLVM conversion. **The earlier hope that an existing `--enable-direct-hivm-lowering` flag
could express the needed bypass is FALSIFIED (D018):** that flag never reaches the AVE optimizer
guard in the `bishengir-compile` device path (`setupHIVMAVEPipelineOptions` does not populate it),
so `buildLowerAVEPipelines` always runs. Bypassing it needs an explicit route. See
`lead/lead-source-trace.md` §16.

## Blockers and open design questions

All six items below were adjudicated by the user on 2026-10-13 (D021). **D012 remains a hard gate**
(its closure condition is not yet met). **D013's gate is closed** — its closure condition, "the
experiment reports", was met on 2026-10-13 (see item 2).

1. **D012 cache4** — carrier search exhausted; no MTE3 store-side cache field exists in the HIVM
   dialect. **Decided: option (a), a narrow store-side cache carrier extension (ODS/attribute plus
   matching lowering), independently reviewed.** Gate closes when the extension lands and P1 asserts
   the carrier is present in the emitted IR. A clean compile is not evidence.
2. **D013 terminal DMA completion — GATE CLOSED by the experiment (2026-10-13).** The frozen
   discriminating experiment ran and reported: §5 row `HAS / NO / NO` — **the drop is confirmed** at
   the MLIR pass layer. The terminal `PIPE_ALL` barrier has three emitters (GSS; `InjectSync`
   `BARRIERALL`; `InjectSync` `NORMAL` via `InsertLastPipeAll`), all gated on `hacc.entry` + DEVICE
   and all skipped by the Late cut; the HIVM→LLVM conversion does not restore one. **P1 obligation:**
   the Late IR tail must carry terminal completion explicitly, or the drop must be justified by
   independent device-layer evidence. The four Golden `set_flag`/`wait_flag` ops are preserved
   verbatim. Device/runtime completion is carried forward as an explicit **P3 obligation**.
   Evidence: `lead/d013-result-20261013.md`; `verify/d013/summary.tsv`.
3. **D009 mask width** — representation identified (`pb32`). **No adjudication needed.** P1 obligation:
   assert the emitted `mask_bit_width = 32` directly.
4. **Pipeline cut** — `enableDirectHIVMLowering` cannot bypass the AVE optimizer in the compile
   driver (D018, falsified hypothesis). **Decided: option (b), the Challenger's recommendation — a
   dedicated, mutually exclusive Late entry in the existing RegBase driver**, no new target kind,
   skipping HIR/HFusion, bufferization, planning, auto-sync, loop lowering, multibuffer and
   `buildLowerAVEPipelines`, retaining validated legalization and exactly one `runHIVMCCompileA5`.
5. **Runtime predicate exclusivity (D004)** — confirmed: `target_is_plain_ascend` is defined by the
   *absence* of `pto`, so an `hivm` key would make it match too and `_resolve_backend_module` would
   raise. **Decided: implement per the Lead recommendation** — redefine the plain predicate to
   exclude `hivm` (see `lead-source-trace.md` §19, §19a).
6. **AppendDeviceSpec is not idempotent** (Challenger, Lead-re-verified) — it warns and then
   overwrites the device spec unconditionally (`AppendDeviceSpec.cpp:142-151`). **Decided: implement
   per the Lead recommendation** — the Late entry validates a complete matching spec and errors on
   mismatch rather than silently overwriting.

## Next action

**The ordering constraint is satisfied.** The D013 discriminating experiment was defined with its
expected result frozen before execution, it ran, and it reported: the drop is **confirmed**. Steps 1
and 2 of the previous plan are complete.

**The next action is P1**, now started, with D012 option (a) and the cut option (b) as the design
basis. P1's acceptance is the frozen check set **B5–B8** (`p0-acceptance-criteria.md` §B-2), all
defined before implementation and all NOT_RUN until executed:

1. **B5** — the hand-authored Late kernel parses and verifies with the locally built `bishengir-opt`
   (exit 0, empty verifier diagnostics).
2. **B6** — negative tests: each deliberately invalid op / wrong ABI attribute / missing
   `module_core_type` is rejected with a nonzero exit and a specific diagnostic.
3. **B7** — op-by-op semantic trace to the Golden TIR, counts equal, **including the D009 assertion
   on the emitted `intr.hivm.pge.b32` / `mask_bit_width = 32`**; a clean compile is not evidence.
4. **B8** — preservation asserted from the parsed IR: grid=64, dynamic UB=196608, spacing 65536, both
   sync events with set+wait, A/B/C ABI, and the three distinct unit systems.

P1 carries two D013/D012 consequences into the artifact: an explicit terminal completion epilogue,
documented separately from the four Golden flag/wait ops (which stay verbatim), and an explicit
record of the store cache mode that states what the current lowering actually does with it rather
than assuming cache4 survives.

**D013 is closed as a gate.** D012 remains a hard gate until the extension lands and is reviewed —
P1 does not implement it; P1 makes the drop observable. P2–P6 remain unauthorized. The Challenger
verdict (D020) is not converted to APPROVE and remains the review record.

## Lead artifacts this session (2026-10-13)

- `lead/d013-experiment-frozen-20261013.md` — the frozen D013 experiment (fixture, 7 arms, predictions
  recorded before execution, §5 discriminating table). `lead/d013-addendum-1-counter-case.md` records
  an in-tree counter-case found after freezing, without changing any prediction.
- `lead/d013-triage-20261013.md` — Lead triage of the first pass: A3 is a genuine refutation of F11
  (not a harness artifact), A4 and A7 were invocation defects. Diagnostics under `lead/diagnostics/`.
- `lead/d013-result-20261013.md` — the consolidated result: §5 row `HAS / NO / NO`, **drop confirmed**;
  the corrected three-emitter mechanism; the A3 refutation; the P1 obligation and the P3 carry-forward.
- `lead/lead-source-trace.md` — §28 (D009 silent-default mechanism, 2048/numElems) and §29 (D012
  silent-drop proof SD-01 vs SD-03) added; §18 corrected for the three-emitter terminal barrier.
- `lead/lead-review-of-acceptance-matrix.md` — Lead review of the verifier matrix: ACCEPT with the
  C1 source-citation correction (regbase `buildConvertToHIVMPipeline`/`buildHIVMTensorOptimizations`,
  NOT the generic `buildOptimizeHIVMPipeline`) and two tightenings (B4/D009, D/§F PATH rule). The
  verifier applied all three (matrix Rev 2).
- Docs updated: `late-ir-contract.md` → draft 4 (D009 mechanism in §5, D012 silent-drop in §4);
  `pass-cut.md` §1 (verified regbase tail symbols); `decisions.md` D009/D012 sharpened, **D020
  (Challenger verdict) recorded**; `p0-verification.md` §6.5 verdict, §6.6 gate status.
- Independent Challenger verdict received by SendMessage and recorded (D020); `review/` is empty by
  the Challenger's own no-`.md`-artifact instruction, so the verdict lives in `decisions.md`.

## Phase table

| Area | Outcome | Limit |
|---|---|---|
| Golden freeze + hashes | PASS (independent, 55/55; end-of-session re-hash unchanged) | No regeneration performed |
| Branches preserved | PASS | No commits/pushes |
| Compiler binary identity | PASS (Verifier Task 2) | No rebuild/install |
| TileLang backend/runtime trace | PASS (Lead source) | Routing not implemented |
| A5 compile-chain trace | PASS (Lead source; Challenger independently anchored) | No execution trace |
| Late-IR contract | DRAFT 4, evidence-backed | Challenger verdict **REVISE**; user adjudicated the boundary items (D021) |
| Pass cut point | HYPOTHESIS; flag route FALSIFIED (D018) | Challenger: BLOCK for flag, REVISE for dedicated Late entry. **User decided option (b)**; P2 must still prove it with IR dumps |
| P0 acceptance matrix | DRAFT Rev 2, Lead-reviewed and verifier-revised (`p0-acceptance-criteria.md`) | Challenger review not applied to this file |
| Independent Challenger verdict | **DELIVERED** (D020) | REVISE/BLOCK, not APPROVE |
| P0 gate | **CLOSED by user adjudication** (D021) | Closed by user decision, not by a Challenger APPROVE; D012 stays a hard gate, D013's gate closed 2026-10-13 when its experiment reported |
| D013 experiment | **EXECUTED; verdict DROP CONFIRMED** (`lead/d013-result-20261013.md`) | A4 NOT_RUN (tool swallowed the `{…}` form); A3 refuted the frozen F11, recorded as a refutation; A4-fix and A8 added post-freeze, both PASS. Run-3 re-execution incident disclosed and independently re-verified as value-neutral |
| P1 | **ARTIFACT DELIVERED and Lead-verified; B5/B7/B8 PASS, B6 PARTIAL (5/9 rejected); verifier report pending** | Hand-authored Late MLIR only; no NPU-IR production code, no rebuild, no NPU run. D012 stays a hard gate |
| P2–P6 | NOT_RUN | No authority in this phase |

## P2 gate round (2026-10-13, later) — SUSPENDED by the user mid-round

**Stage: SUSPENDED at the user's request. Reason: the Haiku model route is unavailable** — every
`verifier-haiku` spawn failed immediately with `API Error: 403 OpenCode Zen`, with **zero tool calls
executed** (the original `verify` teammate: 3 consecutive 403s; a fresh `verify2` spawn: same). This
is a model-route failure, not a task refusal. The user paused the round to reconfigure the provider.
Both teammates were stopped. **No production code was modified; both repositories are clean.**

**Evidence written this round** (all under `/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-d012/`):
`PAUSE-STATE.md` (the full pause record), `lead/task-a/{repos,hashes,env}.log`,
`lead/lead-d012-source-trace.md`, `lead/p2-late-entry-design.md`,
`lead/p2probe/{README,README-tail,README-hivmc}.md` + raw logs, `lead/probes/`.

**Task A (state restoration) — Lead-measured, matches the record; independent verification NOT_RUN.**
AscendNPU-IR-1 `e878f349…0f2ec` clean; TileLang `cc56643d…71a6` clean; all 5 Golden hashes match
(`.tir` `4194733b…12a18`, `.json` `576fa7a3…c0fe5`); P1 artifact `3b930509…06a99` (18633 B) unchanged;
P1 oracle `expected.json` `a64a0d02…b0f9e` and `p1_check.py` `fd5f0c07…da3d` unchanged;
`build/bin/bishengir-opt` `71ac4012…5962` vs `build/install/bin/bishengir-opt` differing by **exactly
1 byte** (offset 75202, `0x3a`→`0x00`, the DT_RUNPATH trailing colon — D019); container up; conda
Python 3.11.17. **This is Lead-measured, so it does not satisfy Task A's independence requirement;
that run is NOT_RUN.**

**Task B (D012 fix design) — Challenger verdict REVISE (design review only; no gate closure).** The
`review` teammate (challenger-opus, ran normally) returned a source-anchored REVISE. Its load-bearing
claims were independently re-verified by the Lead: `StoreOp::build` delegates to the generated builder
(`HIVMDMAOps.cpp:384-386`); `Copy1D.cpp` carries 13 `REGISTE_DMA_STORE` bodies, so the store bodies
live in `Copy1D/2D/3D.cpp` and not only in the header; `store-2d-compact-mode.mlir:37` asserts the
3-operand store libcall and must be updated to 4 with a default cache0; the scalar fallback
`store_ubuf_to_gm_1d_by_scalar` uses `INTRINSIC(st_dev, …, 0)` with a literal cache, so a nonzero
policy cannot be threaded through it; the valid store cache set is `{0,1,2,4}`
(`asc_store_l2_cache_mode`, CANN enum.h sha `01ed1142…19ef`). **Challenger's minimal modified-file
list (7 files, AscendNPU-IR-1):** `HIVMDMAOps.td`, `HIVMDMAOps.cpp`,
`regbase/HIVMToStandard.cpp`, `lib/RegBase/Vector/include/RegBase/DMAUtils.h`,
`lib/RegBase/Vector/{Copy1D,Copy2D,Copy3D}.cpp`. **Required design revisions:** the public ciface
takes a 32-bit cache arg (narrowed to `uint8_t` only at the intrinsic); append the operand **only for
`isAscend950`**, not all of `isRegBasedArch` (which also covers 310B), with a loud failure for a
nonzero cache on a non-950 target; add the ODS attr at the **end** of the argument list and update the
delegating build call; accept only valid store modes and reject invalid ones rather than truncating to
uint8; bound nonzero policy support to the aligned DMA path; add frozen tests (cache4 / default /
invalid / unsupported-target). The Challenger's full written review had not landed on disk at pause;
only its inline summary is available. **No architecture or user escalation is required within D021's
narrow extension.**

**Task C (D012 independent acceptance) — NOT STARTED.** Blocked by the Verifier route failure. No
production code was modified, so there is nothing to accept.

**Task D (P2 design) — Lead draft complete**, `lead/p2-late-entry-design.md`, covering all 10 required
items with measured evidence: the branch point (`bishengir-compile.cpp:114-119`) and the three-route
mutual-exclusion table; the **measured** proof that the normal RegBase entry crashes on the P1 Late
artifact (`PlanMemory Traverse IR Failed!`, rc=134, dies at `hivm-plan-memory-regbase/module/2`); the
**measured** clean conservative legalization tail (rc=0, preserving the 4 Golden flag/wait ops, the
D013 barrier, `pge.b32`, grid 64, arena 196608, module/func AIV metadata, `dav-c310`); the skip list
with reasons; the unique `runHIVMCCompileA5` call site (`BiShengIRCompileMain.cpp:448`); the
pass-instrumentation mechanism (`--print-pass-id` = 335 lines measured on the normal path;
`hivmc --mlir-print-ir-after-all` = exactly 6 passes with `convert-hivm-to-llvm` ×1); the **measured**
LLVM ABI table (`@main_kernel(!llvm.ptr<1> ×3)`); 8 negative cases; the VF-outline assumption with a
frozen minimal experiment; and the real artifact paths. Also measured: invoking `hivmc` directly fails
with `cannot find any bitcode files for coreType:1`, because the template-bitcode path attributes are
attached only by the `bishengir-compile` driver's `addBitcodeAttrsToModule` — so the Late entry must
run in that driver and must hard-fail on a missing bitcode file.

**Gate answers at pause:** D012 closed — **NO** (design reviewed REVISE; nothing implemented or
verified). P1 fully passed — **NO** (31 PASS / 1 FAIL stands; `B8.CACHE4` = FAIL attributed to D012).
P2 startable — **NO** (D012 is the gate; P2 production code is unauthorized and not begun). Golden and
oracle unedited.

---

## D012 preflight REPAIR — landed, built, and independently verified (2026-10-13 ~14:42-14:54)

**Stage: the two ACTIVE preflight defects (plus the Lead's third) are repaired; the repair is built
and the Verifier's independent suites pass. Awaiting the Challenger's final-diff verdict.**

**What changed (Implementor, `git diff` confined to 3 of the 11 already-modified files).** Repair-only
diff = 193 lines (`impl/repair-build/repair-only.diff`); the other 8 files are byte-identical to the
pre-repair state (verified by reconstructing the pre-repair tree from `HEAD` + the F1–F8 diff).
1. `regbase/HIVMToStandard.cpp` — `verifyStoreCacheMode` tightened to a **6-clause fail-closed
   predicate**: mode valid + ∈{0,1,2,4}; Ascend950; src rank-1 unit stride; **dst rank-1 unit stride
   (new — the predicate previously never read the destination)**; **srcOffset == 0 (new — fixes the
   bytes-vs-elements unit mix)**; **exactly one constant `pointer_cast` address with byteAddr%32==0**.
   `getConstantUBByteAddr` no longer sums variadic addresses. The diagnostic and the function's doc
   comment now name all four requirements.
2. `RegBase/Vector/include/RegBase/DMAUtils.h` — the rank-1 cache wrapper and the
   `DECLARE_DMA_STORE`/`REGISTE_DMA_STORE` macro pair are moved **inside** the existing
   `#if defined(__DAV_C310__)` guard, with **byte-identical pre-D012** `#else` arms (Lead-verified
   textually against `git show HEAD:` and by real `g++ -E -P` preprocessing of both configs:
   `lead/m300-confinement-check/README.md`). This removes the F1–F8 m300 signature regression.
3. `HIVMDMAOps.td` — the **falsified** doc claim ("a nonzero policy cannot reach the scalar
   fallback") replaced with the actual narrow predicate and the explicit statement that other shapes
   *are* rejected because they can reach a scalar fallback that stores cache 0.

**Build (14:46:23, single serialized incremental build, `impl/repair-build/build.log`, NINJA_RC=0).**
`build/bin/bishengir-opt` `b98897af…` → **`0761c6e662ee015e30425ee29e0c2314753aa356b21eb4119ff99747327a1977`**.
The three repaired files re-hash byte-identical to the built state (`impl/repair-build/repair-files-sha256.txt`).

**Independent verification (Verifier, own scripts).**
- **Probe suite re-run** (`verify/d012-probes-20261013T064807Z-postfix/`, absolute paths, compiler
  sha recorded before/after): **all 6 previously-RED probes (L1,L2,L3,L4,P1,P4b) now REJECT** rc=1
  with the tightened diagnostic; **all 5 controls hold** — L5 (Golden shape) and P3 ACCEPT and carry
  `%c4_i32`, L6 (mode 0) ACCEPTs with `%c0_i32`, P2/P4 stay REJECT. `grep "cannot open input file"`
  over the run → 0. This is a clean 6-RED→GREEN flip with no control regression.
- **Frozen oracle C1–C8 ALL PASS** (`verify/d012-oracle/logs-accept-20261013T065150Z/summary.txt`,
  absolute paths, `cannot open input file` → 0): C5 now reads `6/6 numeric arms rejected via value
  diagnostic` with `badtype` `[category=type]` — the non-vacuous signature. Lead independently
  reproduced the same C1–C8 all-PASS + exit 0 in `lead/oracle-lead-check/`.

**HARNESS DEFECT found and reported (Lead).** The Verifier's *first* post-fix oracle attempt
(`logs-accept-20261013T065017Z/`, 14:50) is **INVALID**: it passed a **relative** `--work-dir`, and
`scripts/in-container.sh` fixes the container workdir at `/home/s00653124`, so **all 27 invocations
failed with `cannot open input file`**. Every rc=1 in it is file-not-found, not a compiler result,
and its **C5 "PASS" was a false pass** (arms "rejected" only because the input was absent — the
"0/6 numeric arms rejected via value diagnostic" line is the tell). Recorded in
`lead/FINDING-oracle-accept-run-relative-paths.md`; the corrected re-run supersedes it and the bad
run is kept as the defect record. (Second relative-path/selector incident in this evidence set,
after the D013 run-3 `D013_ONLY` env-var drop.)

**Gate answers (this repair does NOT by itself close D012 — the Challenger verdict is still pending).**
D012 closed — **PENDING Challenger final-diff verdict** (implementation-side evidence now strong:
predicate sound+complete on the accepted set, C1–C8 all PASS, 6-RED→GREEN with controls held).
P1 fully passed — **NO** (see the Task C compiler-pin conflict below: `B8.CACHE4` reporting is
BLOCKED, so `B8.CACHE4` = NOT_RUN; the historical 31 PASS / 1 FAIL stands). P2 startable — **NO**
(no P2 production code; unauthorized and not begun).

---

## P2 gate round, resumed (2026-10-13) — suspension lifted; oracle amendment is the current gate

**Stage: RESUMPTION IN PROGRESS. The Haiku route works again.** The verifier teammate (`verify3`,
verifier-haiku) now executes tool calls; its model route is `opencode-go/mimo-v2.6-flash`, recorded
before measurement (D026's route failure is cleared). **No production code has been modified; both
repositories are clean; the Golden and the P1 oracle are byte-unchanged.**

### Task A — **PASS, independently verified** (D026's NOT_RUN closed)

`verify/task-a/state-restore.md` reproduces every Lead number with the Verifier's own commands, and
states its independence explicitly: AscendNPU-IR-1 `e878f349…` clean on
`feat/tilelang-hivm-late-pipeline`; TileLang `cc56643d…` clean on `feat/hivm-backend-poc`; **5/5**
Golden sha256 match; P1 artifact `3b930509…06a99` (18633 B); `expected.json` `a64a0d02…b0f9e` and
`p1_check.py` `fd5f0c07…da3d`; `build/bin/bishengir-opt` `71ac4012…5962` vs `build/install/bin/…`
differing by **exactly one byte** at offset 75202 (RUNPATH trailing colon, D019); container
`s00653124_tilelang` up; Python 3.11.17. Raw logs A1–A8. **Task A's independence requirement is
satisfied.**

### D012 oracle — **FROZEN, validated, and now being amended**

Frozen `verify/d012-oracle/d012_oracle.py` (conditions C1–C6) + `README.md` (13366 B: per-check frozen
rule, exact command line, raw-log paths, pre-registered baseline) + `selftest/` + `probes/`. The
Lead's independent before-run on the unmodified tree gives **exit 1: C1/C2/C6 PASS, C3/C4/C5 FAIL** —
a correct, non-vacuous before-signature. The cleanest single-line statement of D012 is the probe
`probes/std-compare.log`: **six different inputs (base, cache0, cache4, cache7, cacheneg, cachestr)
all lower to one identical output**, md5 `6c750d0f65b48375d6564d81904c28c7`.

### Challenger's full written review — **ON DISK** (D025's open item closed)

`review/challenger-d012-design-review.md`, persisted **verbatim** by the Lead (the Challenger's
reviewer instructions forbid it writing report files). Verdict **REVISE**; it confirms all five D025
revisions, answers the three open questions, and lists **F1–F7** falsification experiments. Notable:
**no Ascend310B template exists in-tree** (the 950 gate protects a future build *and* the existing c220
arm); the gate belongs at **both** the ODS verifier and the conversion; and value **3 is not a valid
load mode** either (only 5 is load-only), correcting the Lead.

### New measured finding — carrier name/type vs the frozen oracle (**requires an oracle amendment, not a design change**)

`lead/FINDING-carrier-name-type-vs-oracle.md`, raw probe `lead/type-probe/`. `OptionalAttr<I32Attr>`
**rejects a bare integer literal** — measured against the already-registered `ND2NZOp` `I32Attr`
(`HIVMDMAOps.td:396`, so **no production change was needed**): `{l2_cache_mode = 4}` → *"failed to
satisfy constraint: 32-bit signless integer attribute"*; `{l2_cache_mode = 4 : i32}` parses rc=0. The
oracle fixtures are all **bare** (`{l2_cache_ctl = …}`) and parse only because `l2_cache_ctl` is
registered **nowhere** — i.e. they are built from the syntax D012 itself produces.

**Rulings (both delivered):** the design is **UNCHANGED** — `l2_cache_mode` + `OptionalAttr<I32Attr>`
appended last (R3); the Challenger **withdrew** its own I64/`l2_cache_ctl` alternative after reading
the README's predeclared compatibility provision. **All integer fixtures must be typed** (positives
*and* the four numeric negatives), with the **rejection category reported per arm**, so a
type-constraint failure cannot stand as proof of the `{0,1,2,4}` value set — otherwise C5's weak
exit-OR-named predicate (`d012_oracle.py:440-444`) would report PASS for the wrong reason. The
re-freeze is **legitimate and not a weakening**. **A bare `ARTIFACT_SHA` re-pin is NOT approved:**
`d012_oracle.py:205-230` *appends* the variant attribute to a copy of the source, so an annotated
source yields `absent` that already carries 4 plus duplicate attributes (verified by simulation); Task
C must keep the original artifact byte-frozen and run the unchanged P1 checker via `--artifact`
(`p1_check.py:1785`) on a **separately named** typed annotated deliverable, preserving the original
31-PASS/1-FAIL report as historical record. **Two of the Lead's own first-draft claims were wrong and
are corrected on the record:** "unsatisfiable by any correctly-typed carrier" was too broad (a
correctly-typed **I64** carrier *would* satisfy the bare fixtures; the contradiction was with **I32**
specifically), and the annotate-and-re-pin recommendation was wrong. The I64 bare-literal behaviour was
**measured, not inferred** (`VGatherOp.gather_axis`, `HIVMVectorOps.td:1912`) and independently
reproduced by the Lead (`lead/type-probe/vg_{bare,i64,i32,str}.mlir`).

### Current gate

**The Verifier's amended oracle (typed fixtures + re-established failing baseline on the unmodified
tree) is the gate before the Implementor.** Order per the user's constraint: Verifier freezes →
Challenger reviews the design (DONE) → Implementor edits production code → build → Verifier re-runs
the oracle → Challenger reviews the final diff.

**Gate answers now:** D012 closed — **NO**. P1 fully passed — **NO** (31 PASS / 1 FAIL stands;
`B8.CACHE4` = FAIL). P2 startable — **NO** (no P2 production code; unauthorized and not begun).

## Lead read-only de-risking for the Implementor (2026-10-13, ~12:50-13:00)

While the Verifier amends the oracle, the Lead measured five facts the fix design depends on. **No
production code touched; both trees clean; Golden and P1 oracle byte-unchanged.**

**1. The P1 checker needs no edit — proven, not assumed.** `StoreOp`'s `assemblyFormat`
(`HIVMDMAOps.td`) is `ins(...) outs(...) attr-dict (atomic = $atomic_kind^)? (-> type($result_tensor)^)?`.
`attr-dict` precedes the optional `atomic` clause, so a named optional attribute with no custom
directive prints inline on the `hivm.hir.store` line. `p1_check.py:1529-1531` extracts store lines and
matches `(?i)(l2_cache_ctl|cache_modifier|eviction_policy|l2_cache\w*)`; `l2_cache_mode` matches
`l2_cache\w*`. F1's claim is confirmed.

**2. `isValidStoreCacheMode` does NOT exist** (`grep` over `lib/` + `include/` returns nothing). The
F2 snippet names a helper that must be **created**, and because it is needed in **two** TUs (the F2
verifier and the F3 preflight) it belongs in `Dialect/HIVM/Utils/Utils.h` (namespace `mlir::hivm`,
`:62`), which **both** TUs already include. One definition, not two — a split {0,1,2,4} set is how the
load/store value spaces get conflated.

**3. ND2NZ precedent confirms F2's delegating-builder fix.** `ND2NZOp::build` (`HIVMDMAOps.cpp:667-684`)
already passes `/*l2_cache_mode=*/nullptr` explicitly; `StoreOp::build` (`:384-386`) must mirror it.

**4. Two further in-tree `l2_cache_mode` users are out of scope** — `InferHIVMDataLayout.cpp:142`
(an `annotation.mark` **string**, not an op field) and the `ND2NZOp` builders (`:683,692`,
`CombineOptimizedConvertLayout.cpp:91`). No collision; no behavioural change to those paths.

**5. Why the fix cannot reuse the existing `eviction_policy` carrier (anti-false-pass).** The tree
already has a shared-memory cache attribute that the P1 regex **also** matches, so it is a plausible
shortcut to a green `B8.CACHE4` **without carrying the mode**. Measured: `HIVM_EvictionPolicyEnum`
(`HIVMAttrs.td:1206-1211`) = `{EvictNormal, EvictFirst, EvictLast}`, `EvictionPolicyMap`
(`HIVMToStandard.cpp:67-70`) = `{EvictFirst→0, EvictLast→1}` — **no value 4**; it is declared on
`LoadOp` (`HIVMDMAOps.td:101`) and `CopyOp` (`:130,134`), **never** `StoreOp`; and `addEvictionOperands`
is called **only** for `LoadOp` (`:1258-1260`), while the `StoreOp` branch (`:1260-1276`) appends only
`atomicKind`. So an `eviction_policy` annotation would flip the regex-based checker while the store
still ignores it — a **false pass**, forbidden by Task C. The oracle's C4 already forecloses it
(`check_c4`: `val != 4 → FAIL`, a **value** assertion); it must not be relaxed to a presence check.

### Oracle defect found during the Lead's review of the v2 baseline (12:58:03)

The Verifier's v2 baseline ran **before** the required `l2_cache_ctl` → `l2_cache_mode` rename, so
**that run is not a valid frozen baseline** (reported; the rename is in progress). Its signature is the
expected one — C1/C2/C6 PASS, C3/C4/C5/C7/C8 FAIL — and is unchanged by the rename (pre-fix both
spellings are unregistered/discardable and parse identically; the rename only changes the **after**
run). **Independent defect found in that run:** C2 **PASSes** ("carrier value 4 present") on the same
`c4` fixture C4 proves is **silently dropped**. `check_c2` only regex-matches the store line and never
checks that the op *owns* the attribute, and `attr-dict` accepts any named attribute — so C2 is
satisfiable by a discardable attribute the op ignores, i.e. **C2 can PASS on exactly the drop C4
detects**. Verdicts are correct; C2's **claim** overstates carriage. Required (message-only, no
verdict/assertion change): reword C2 to what it proves (parses, stable, not rejected) and defer
carriage to C4/C3; add the caveat to README. **C4's `val == 4` assertion must stay.** Evidence:
`lead/FINDING-carrier-name-type-vs-oracle.md` §9-§10; `lead/d012-fix-design-revised.md` §10;
`lead/regression-sweep-store-arity.md`.

**Gate answers unchanged:** D012 closed — **NO**. P1 fully passed — **NO**. P2 startable — **NO**.

## D012 oracle FROZEN — v3 baseline on the unmodified tree (2026-10-13 13:15)

The Verifier completed the amendment: `d012_oracle.py` fixtures now write **`l2_cache_mode`** typed
`: i32` (`:284-285`), `classify_rejection` added (C5 per-arm category), and C2 reworded to claim only
what it proves. **The Lead independently verified all 16 regenerated fixtures** (`work/`, 13:14:57):
every integer arm is `{l2_cache_mode = N : i32}` (c0/c4/bad3/bad5/bad6/bad7/bad256/badneg/tless_c4/
n950_c0/n950_c4/unaligned_c4/unproven_c4), `badtype` is `{l2_cache_mode = "not-an-int"}`, and
`absent`/`tless_absent` carry **no** attribute. Pre-run state: both trees clean (`porcelain 0`, HEAD
`e878f349…`), artifact sha `3b930509…` matches.

**Frozen before-signature (v3, `logs/baseline-run-v3.log`, `logs/summary.txt` 13:15:08):**
**C1 PASS, C2 PASS, C6 PASS; C3 FAIL, C4 FAIL, C5 FAIL, C7 FAIL, C8 FAIL** — the correct non-vacuous
pre-implementation state, **unchanged from v2** as predicted (the rename does not move the pre-fix
result: both spellings are unregistered/discardable pre-fix).

**C5 is now provably non-vacuous and category-attributed:** every numeric arm
(`bad3/bad5/bad6/bad7/bad256/badneg`) is rejected as **`silent`** (parse rc=0, lower rc=0, no
diagnostic) — i.e. pre-fix the failures are *silent drops*, not type-constraint artifacts, so C5
exercises the **value** space and its `bad3`/`bad5` load-vs-store distinction is genuinely untested by
a broken implementation. `badtype` is the type arm. C2's text now states explicitly: *"C2 does NOT
establish carriage — attr-dict accepts discardable attributes the op does not own; carriage is
adjudicated by C3/C4 only"* — the overclaim the Lead found in v2 is fixed.

**This closes the user's ordering step 1 (Verifier freezes the oracle).** The Implementor is now
ungated on the oracle side; the fix design is at `lead/d012-fix-design-revised.md` §1-§10 (with §10.7's
four-link template threading restated per the Challenger's D029 corrections).

**Gate answers unchanged:** D012 closed — **NO**. P1 fully passed — **NO** (31 PASS / 1 FAIL stands).
P2 startable — **NO** (no P2 production code; unauthorized and not begun).

## Task D — P2 Pass + ABI acceptance plan is DESIGN-COMPLETE (2026-10-13)

`lead/p2-late-entry-design.md` (17140 B, §1–§12) covers **all ten** Task D required items, one per
section, with measured evidence rather than assertion. Mapping:

| Task D item | § | Key measured content |
|---|---|---|
| Where the dedicated Late entry branches; mutual exclusion vs normal RegBase + legacy | §1 | branch at `bishengir-compile.cpp:114-119` inside the existing `hasRegBaseTarget` block; three routes, exactly one taken; Late+legacy together = hard error (not silent precedence) |
| Why the normal RegBase entry cannot be reused | §2 | measured **rc=134 SIGABRT** — `PlanMemory Traverse IR Failed!` at `hivm-plan-memory-regbase/module/2`; never reaches `convert-hivm-to-std`/`convert-hivm-to-llvm`/`runHIVMCCompileA5` |
| Conservative Legalization Tail order (first version) | §3 | measured rc=0 tail; spelling correction found by measurement: `hivm-memref-alloc-to-alloca` (not `alloc-to-alloca`); all Golden markers preserved |
| Which stages must be SKIPPED | §4 | HIR/HFusion, Bufferization/PlanMemory, GraphSyncSolver+InjectSync, Multibuffer, AVE optimizer, loop lowering, the retry loop — each with a reason |
| Unique `runHIVMCCompileA5` call site | §5 | `BiShengIRCompileMain.cpp:448`, reached only via `runRegBasePipeline:345` ← `Driver.cpp:81`; shape (i) separate `runLateRegBasePipeline` preferred (keeps normal path byte-identical) |
| Pass instrumentation proving LLVM lowering runs exactly once | §6 | `--print-pass-id` (335 lines measured) + `hivmc --mlir-print-ir-after-all`; HIVMC A5 tail = **exactly 6 passes, `convert-hivm-to-llvm` ×1**; P2 asserts count=1 and forbidden-set=0 from the stream, not source occurrences |
| LLVM ABI checks | §7 | 3 `!llvm.ptr<1>` GM params, `@main_kernel`, AIV module/core metadata, `hacc.block_dim = 64 : i64`, `dyn_shared_memory_buf = 196608 : i64`, `dav-c310` target stamp, D013 barrier ×6 |
| Negative cases (frozen before impl) | §8 | N1–N8 (selector+legacy; partial spec→segfault hazard D022; wrong target; wrong stage; unsupported op; missing module core type; missing bitcode→hard error; mismatched spec) |
| Unresolved VF-outline assumption + minimal validation experiment | §9 | frozen prediction: outlined form required; experiment = inline the VF body, run both through the tail, compare `convert-hivmave-to-ave-intrin` output; either outcome is a finding, recorded before execution |
| Real build/bitcode/compiler artifact paths P2 will use | §10 | absolute paths: `build/bin/bishengir-compile`, `build/bin/bishengir-opt` (sha `71ac4012…5962`), `build/bin/hivmc`, `build/lib/meta_op.aiv.c310.{O0,O2}.bc`, the P1 Late input, the device-compiler flags |

**Three open decisions are surfaced for the user, not silently chosen (§12):** the Late selector
spelling; whether P2 shares the D012 rebuild; the "installed binary identity" target artifact.

**No P2 production code has been written and none will be without the user's explicit authorization.**

## D012 fix landed + independent verification (2026-10-13, resumed session)

**Stage: F1–F8 landed, built, and independently verified; D012 carrier closure established on the
implementation side. P1's `B8.CACHE4` flip is BLOCKED by a compiler-identity pin — a gate conflict
reported to the user, not worked around.** Both repos: only the 11 intended production files modified
in `AscendNPU-IR-1`; `tilelang-mlir-ascend` clean (0 changes). No build running.

**What landed (Implementor, `git diff` 31077 B / 11 files).** `StoreOp` gains
`OptionalAttr<I32Attr>:$l2_cache_mode` appended last (`HIVMDMAOps.td`); `StoreOp::verify` rejects any
mode outside the store value space `{0,1,2,4}` and rejects a nonzero mode on a non-Ascend950 module
(`HIVMDMAOps.cpp`); `isValidStoreL2CacheMode` added (`Utils.{h,cpp}`); the regbase `HIVMToStandard`
store lowering appends the 4th libcall operand on Ascend950 only, after a fail-closed preflight that
rejects a nonzero mode unless the UB base is provably 32-byte aligned with unit last-dim stride
(`regbase/HIVMToStandard.cpp` +113); the C310 template threads the cache through the full four-link
chain with `uint32_t` at the public ABI narrowed to `uint8_t` only in the final intrinsic
(`RegBase/Vector/include/RegBase/DMAUtils.h`, `Copy{1,2,3}D.cpp`); two test files updated and the
`host-compile.mlir` canary left frozen.

**Build (13:46, single incremental build, serialized).** `build/lib/meta_op.aiv.c310.O2.bc`
`44e4387b…` → `5e7ccae2…` (**regenerated**); `aiv.c220` `b1dfbce3…`, `aic.c220` `3c23b302…`,
`aic.c310` `0ddddeed…` all **byte-unchanged** — no other target architecture moved.
`build/bin/bishengir-opt` `71ac4012…` → `b98897af…`.

**Independent verification (Verifier `verify3`, own scripts under `p2-20261013-d012/verify/`).**
- **D012 oracle C1–C8 ALL PASS** (`d012-oracle/logs-after-fix/summary.txt`), against the post-fix
  compiler. C5 = 6/6 numeric arms rejected via **value** diagnostic (`badtype` = type), so the
  store-vs-load value-space distinction is genuinely tested, not a type artifact. C3's c0-vs-c4 diff
  is exactly one line — the `%c4_i32` constant and the 4th operand — where pre-fix six different
  inputs produced byte-identical output. C7 confirms the non-950/targetless gates reject cache4 and
  keep the 3-operand ABI for absent/0. C6 confirms all 5 Golden fixtures + `expected.json` +
  `p1_check.py` still match their frozen shas.
- **Arity sweep PASS** (`arity-sweep-20261013T055130Z/summary.txt`): 3 RUN pipelines green; the 4
  must-update sites assert 4 operands; the SURVIVES site intact; the canary frozen at 3 operands.
- **c220 regression PASS** (`c220-regression-20261013T055140Z/summary.txt`): c220 store macro
  byte-identical, 3-operand ABI intact, c220 bitcode population unchanged.
- **Hard gate (Challenger D029/D030) independently met.** The Verifier's own
  `bitcode-obligation-probe/meta_op.aiv.c310.O2.linked.ll` shows the ciface taking
  `i32 noundef %l2_cache_mode`, `zext i32 … to i64`, `shl i64 …, 60` into the
  `llvm.hivm.MOV.UB.TO.OUT.ALIGN.V2.DV` descriptor (bits 60–63), with **0** remaining
  `store i8 0, ptr %l2_cache_ctl`. The Lead's independent `lead/bitcode-gate/HARD-GATE-EVIDENCE.md`
  records the same from per-TU `llvm-link -S` dumps (rank-1/2/3 ciface all `i32 noundef
  %l2_cache_mode`; 13 data-dependent truncations; scalar fallback untouched).

**GATE CONFLICT — reported, not worked around.** Task C requires the frozen P1 checker's
`B8.CACHE4` to flip via a real carrier check. Run against the post-fix compiler it **BLOCKS rc=2
before any B-item executes**: `expected.json` pins the P1-era compiler sha `71ac4012…` while the
post-fix binary is `b98897af…`, and there is no override mechanism (no env var, no `--compiler`
flag; `--artifact` only). The Lead and the Verifier reproduced the **identical** BLOCK
independently (`verify/post-fix-p1-taskc.log`). Editing `expected.json`/`p1_check.py` is forbidden
("do not edit test oracle") and would additionally break oracle C6, which pins both files' shas.
The frozen P1 report's own first item is `PASS ENV.COMPILER sha256=71ac4012…`, so the pin is part of
the frozen record. **The deliverable itself is verified ready**: `l2_cache_mode = 4 : i32` matches
the frozen B8 regex (`l2_cache\w*`), the annotation is the compiler-accepted production attribute
(post-fix lowering rc=0, 4th operand = `%c4_i32`), and the source artifact is byte-frozen at
`3b930509…` / 18633 B. Only the identity gate stands in the way. This is an oracle-integrity vs.
acceptance-runnability conflict that **only the user can adjudicate**.

**Latent defect (non-breaking, disclosed).** The shared rank-1 wrapper
`store_ubuf_to_gm_1d_core_with_contiguous_last_dim` (`DMAUtils.h:546`) sits **outside** the
`#if defined(__DAV_C310__)` block (which opens at `:558`), now takes `l2_cache_mode`, and forwards
**9** arguments to the intrinsic; the m300 `store_ubuf_to_gm_intrin_core` (`:301`) still takes **8**,
and the m300 call sites (`DMAUtils.h:881` in the `#else` block opening at `:851`, and
`Copy1D.cpp:339` in the `#else` block `:299–363`) still pass **two** descriptor arguments. RegBase
compiles only for `Ascend910D` → `__DAV_C310__` (`Vector/CMakeLists.txt:1`), so this is dead code in
this build and cannot break it; the file would not compile for m300. Challenger ruling: repair it
anyway — dormancy does not justify a retained-source signature regression. (An earlier note here
said "3 args"; that described the public ciface, not this wrapper, which takes 2.)

**Challenger final-diff review — VERDICT: REVISE (persisted verbatim at
`p2-20261013-d012/review/challenger-d012-final-diff-review.md`).** Two ACTIVE defects in the
fail-closed preflight, both **independently reproduced by the Lead** in
`p2-20261013-d012/lead/d012-counterexample-probes/` (fixtures + `README.md`):
1. **Incomplete DMA-only proof.** `verifyStoreCacheMode` proves only `srcStrides.back() == 1` and
   base alignment; it ignores outer UB strides, the destination layout, rank reduction and the
   nested scalar dispatch. Executed rc=0 acceptances of cache4 whose template path is
   *deterministically* scalar (`Copy2D.cpp:381` outer 9 elems = 36 B unaligned; `Copy3D.cpp:435`
   outer 17 = 68 B unaligned; `Copy1D.cpp:272` dst stride 2^38 decomposes to unit rows) → the policy
   is discarded and the store runs with cache 0.
2. **Incorrect pointer provenance.** `getConstantUBByteAddr` (`:1206–1219`) **sums** variadic
   `pointer_cast` addresses, but `PointerCastOpLowering` (`HIVMToLLVM.cpp:232`) builds the descriptor
   from `op->getOperands()[0]` only with offset 0. Executed: `pointer_cast(131076, 28)` is accepted
   (sum 131104 aligned) while the emitted UB address is 131076, unaligned.
The Challenger's closure ruling: **"I REFUTE that the CURRENT green C1–C8 plus hard-gate note is
sufficient to close D012 implementation."** The linked-bitcode dataflow evidence stands (cache does
reach `MOV.UB.TO.OUT.ALIGN.V2.DV` bits 60–63 on DMA branches); what it does not prove is that those
branches are *selected* for every input the preflight accepts. The Lead's
`lead/bitcode-gate/HARD-GATE-EVIDENCE.md` §4/§5 claim that the scalar path is unreachable has been
**corrected in place** as falsified.

**Gate answers (CORRECTED). D012 closed — NO.** F1–F8 landed and C1–C8 pass, but the two active
preflight defects above mean accepted nonzero policies can still be silently downgraded to cache 0,
so the carrier is not established for the accepted input set. A narrow repair (reject nonzero unless
a single-tile contiguous rank-1 DMA path is actually proven on **both** layouts; require exactly one
constant `pointer_cast` address and a supported zero static offset) plus the m300 signature repair is
required, and per the ordering constraint must be preceded by Verifier-owned isolated distinguishing
probes and followed by Challenger review of the repaired diff. **The P1 `B8.CACHE4` reporting flip is
BLOCKED by the compiler pin** (`B8.CACHE4` = NOT_RUN; historical 31 PASS / 1 FAIL unchanged; no
workaround attempted — frozen files untouched, no compiler substitution, no hash spoofing). **P1
fully passed — NO.** **P2 startable — NO** (no P2 production code; unauthorized and not begun).

**Task D** remains design-only and complete (`lead/p2-late-entry-design.md`, all 10 items, three open
decisions surfaced in §12). **No P2 production code has been written and none will be without the
user's explicit authorization.**

---

## D012 preflight REPAIR — landed, built, independently verified (2026-10-13, resumed session)

**Stage: repair landed + built + independently verified by the Verifier. The Challenger's
post-repair final-diff verdict is OUTSTANDING — it went idle three times without delivering, and no
verdict is fabricated here.**

### What landed (Implementor, exactly 3 files on top of the F1–F8 change)

| file | change | sha256 |
|---|---|---|
| `lib/Conversion/HIVMToStandard/regbase/HIVMToStandard.cpp` | 6-clause `verifyStoreCacheMode` + single-operand `getConstantUBByteAddr` | `02006629…c6950` |
| `lib/Template/lib/RegBase/Vector/include/RegBase/DMAUtils.h` | rank-1 cache wrapper moved INSIDE `#if defined(__DAV_C310__)`; macro pair split | `e0a4d275…9016` |
| `include/bishengir/Dialect/HIVM/IR/HIVMDMAOps.td` | falsified doc sentence replaced with the actual narrow predicate | `2b90fe34…f3cd` |

The repair-only diff is **193 lines** (`impl/repair-build/repair-only.diff`); the full vs HEAD is 403
lines (`repair-full-vs-HEAD.diff`). Isolation artifacts + `bishengir-opt-sha256.txt` in
`impl/repair-build/`. Built with `ninja bishengir-opt` (NINJA_RC=0); compiler
`71ac4012…` → `b98897af…` → **`0761c6e662ee015e30425ee29e0c2314753aa356b21eb4119ff99747327a1977`**.

### Independent verification (Verifier `verify3`, own frozen suites, sha before/after identical)

- **Post-fix probe suite** `verify/d012-probes-20261013T064807Z-postfix/`: **GREEN=11, RED=0,
  NOT_RUN=0, FAIL=0**. 6/6 formerly-RED probes (L1,L2,L3,L4,P1,P4b) now REJECT rc=1 with the
  tightened DMA-only diagnostic; 5/5 controls hold (L5/P3 ACCEPT `%c4_i32`, L6 mode-0 ACCEPT, P2/P4
  REJECT). Group A E1–E3 PASS, PP-ARITY PASS, A4 (full m300 compile) **NOT_RUN** (no toolchain).
- **Frozen oracle** `verify/d012-oracle/logs-absolute-20261013T070005Z/`: **C1–C8 ALL PASS**, exit
  code **0**, `cannot open input file` count **0** (31 stderr files, 20 non-empty = real executions).
  C5 = 6/6 numeric arms rejected via **value** diagnostic. (This supersedes the invalid 06:50:17Z
  relative-path run, which the Verifier marked additively `INVALID.txt` and preserved as the defect
  record.)

### Lead cross-checks (non-authoritative, independent of the Verifier)

- `lead/d012-counterexample-probes/README.md` — the Challenger's a/b/c/d/g/h fixtures re-run against
  `0761c6e6…`: all now **rc=1 REJECT**; control `f` still ACCEPT `%c4_i32`; Golden `fix_c4` still
  ACCEPT; `fix_unproven_c4`/`fix_unaligned_c4` still REJECT (no over-rejection of the Golden shape).
- `lead/predicate-soundness-analysis.md` — clause-by-clause adversarial read; concludes sound +
  complete for the cache-carrying DMA set, with clause 6's narrowing documented as a limitation.
- `lead/m300-confinement-check/README.md` — real `g++ -E -P` both configs: m300 `#else` arms
  byte-identical to `git show HEAD:`; **C310 preprocessed output byte-identical pre/post repair**, so
  the device bitcode (`5e7ccae2…`) need not change and the `bishengir-opt`-only rebuild suffices.

### Challenger post-repair verdict — **DELIVERED: APPROVE** (persisted verbatim)

The Challenger independently reviewed the **post-repair** final diff and returned **APPROVE** on the
narrow nonzero-cache contract, on its own executed checks (rebuilt compiler `0761c6e6…`, the 11 frozen
fixtures, the linked `5e7ccae2…` bitcode, and its own preprocessing of the non-C310 bodies). Verbatim
text at `review/challenger-d012-post-repair-verdict.md` (it refuses report-file writes, so the Lead
persisted it). Its own words: *"prior scalar-fallback counterexamples reject, the Golden cache-4 store
accepts, and no falsifying counterexample remains within the agreed narrow contract."*

Scope the Challenger itself attached to the approval: it is a **conservative supported subset**, not a
necessary-and-sufficient characterization of every DMA-capable layout; it does **not** establish
hardware numerical correctness, the Late pipeline cut, or full CodeGen E2E. Two non-blocking notes:
the diagnostic wording at `HIVMToStandard.cpp:1235` overstates every rejected case; P4b rejection is
the agreed conservative narrowing. Its earlier pre-repair **REVISE** (14:13) prescribed exactly the
shape that landed, so the design provenance and the final approval are both on record.

### Gate answers (final, this round)

- **D012 closed — YES on the implementation side, for the agreed narrow contract.** Both independent
  reviewers are now green: Verifier 11/11 GREEN (6/6 formerly-RED flipped, 5/5 controls held) + frozen
  oracle C1–C8 ALL PASS (exit 0, 0 `cannot open input file`) + post-repair bitcode obligation
  OBLIGATION_RC=0; Challenger APPROVE of the post-repair diff with no falsifying counterexample. The
  claim is scoped: rank-1 unit-stride both sides, zero static source offset, single constant 32-byte
  aligned `pointer_cast` address. Hardware numerical correctness, the Late pipeline cut, and full
  CodeGen E2E are **not** established by this and remain separate obligations.
- **P1 fully passed — NO.** The historical **31 PASS / 1 FAIL** stands unchanged. Task C's
  `B8.CACHE4` flip is **BLOCKED rc=2** at the frozen `ENV.COMPILER` identity pin (`expected.json`
  pins the P1-era sha `71ac4012…`; the post-fix binary is `0761c6e6…`; no override exists). Editing
  `expected.json`/`p1_check.py` is forbidden ("do not edit test oracle") and would break oracle C6.
  So `B8.CACHE4` = **NOT_RUN**, not PASS. The deliverable itself is verified ready
  (`task-c/vadd_late_l2cache4.mlir`, sha `d1023ac1…f36b`, exactly one line changed to the ruled
  `{l2_cache_mode = 4 : i32}` carrier, matching the frozen B8 regex `l2_cache\w*`); only the identity
  gate stands in the way. **This oracle-integrity vs acceptance-runnability conflict needs the user's
  adjudication** (options: bless a re-pin of the compiler identity in the frozen record; or accept
  `B8.CACHE4` = NOT_RUN with the carrier proven separately by the D012 oracle C3/C4 + hard-gate
  bitcode evidence).
- **P2 startable — NO.** D012 is the gate; P2 production code is unauthorized and not begun. Task D
  remains design-only and complete.

### Repo state at round end

`AscendNPU-IR-1` @ `e878f349…`: **11 intended production files** modified, nothing else. `tilelang`
(and `tilelang-mlir-ascend`) clean (0 changes). Golden `.tir` `4194733b…` / `.json` `576fa7a3…`
byte-unchanged. `expected.json` `a64a0d02…` / `p1_check.py` `fd5f0c07…` byte-unchanged. No build
running. No push, no remote change, no `git reset`/`clean`.

**No P2 production code has been written and none will be without the user's explicit authorization.**

### Post-verdict corrections (Lead, 2026-10-13)

- **§6 factual error corrected** in `lead/d012-preflight-repair-design.md` after the Challenger's
  finding: the earlier claim that `single_fn_spec.mlir:37` stores a **block argument** was wrong.
  Re-measured — line 9 defines `%2 = hivm.hir.pointer_cast(%c131072_i64)` (a local SSA value) and
  line 37 stores that local; the real function args are `%arg0/%arg1/%arg2`, all GM. The file
  therefore **satisfies** clause 6 and does not evidence a block-argument rejection. The Challenger
  was right; the text and its §"brief" cross-reference are corrected in place, and the terminology
  fixed to "diagnostic derived MLIR", not "real Golden lowering".
- **Non-blocking, recorded not fixed**: the Challenger notes the rejection diagnostic at
  `HIVMToStandard.cpp:1235` ("Any other shape can take the scalar fallback") overstates every rejected
  case; a better phrasing would name an unsupported/unproven path. It is a production-string change,
  so it is left for the Implementor and flagged here rather than edited by the Lead. The Challenger
  called it nonblocking. The stale summation claim is confirmed gone (`:1244` "deliberately not
  summed"); the ODS doc, the preflight header comment, and the diagnostic all describe the actual
  narrow predicate.

## D025 versioned P1 re-pin — the v2 acceptance snapshot (2026-10-13)

**Stage:** post-D012-repair P1 acceptance, built as a NEW frozen snapshot per the user's D025 ruling
(register entry **D031**; the ruling text is verbatim at `p1v2-20261013-d025/lead/d025-user-instruction.md`).

### The two P1 records, kept strictly separate

| | **original P1 (v1)** | **new P1 (v2, this round)** |
|---|---|---|
| snapshot | `p1-20261013-late-kernel/` | `p1v2-20261013-d025/snapshot/` |
| status | **BYTE-FROZEN, never touched** | new, independently sealed |
| compiler pin | `71ac4012…` (**pre-fix**) | `0761c6e6…` (post-D012-repair) |
| artifact | `impl/vadd_late.mlir` `3b930509…` (store carries **no** cache mode) | `impl/vadd_late.mlir` `d1023ac1…` (carries `{l2_cache_mode = 4 : i32}`) |
| result | **31 PASS / 1 FAIL** (`B8.CACHE4` NOT-CARRIED) — **unchanged, still the historical record** | Lead run: **32 PASS / 0 FAIL**, rc=0 |
| oracle | `expected.json` `a64a0d02…`, `p1_check.py` `fd5f0c07…` | `expected.json` `0439774b…` (5 recorded lines differ), `p1_check.py` **byte-identical** to v1 |

**Correction carried forward (Challenger P1.6):** the original 31/1 run was executed with the
**PRE-fix** compiler `71ac4012…`. An earlier Lead brief said post-fix; that was wrong. This matters,
because it is precisely why the original oracle now BLOCKS: `ENV.COMPILER` pins a binary that no
longer exists — correctly, by design.

### Why v2 is a versioned re-pin and not a relaxation

- `verify/p1_check.py` is **byte-identical** to v1, so "the assertion set is unchanged" is a *byte*
  claim. No sha256 comparison anywhere was disabled; the new gates only ever **ADD** checks.
- `expected.json` v2 differs from v1 in exactly **5 recorded lines** (`schema`, `frozen_utc`,
  `author`, `compiler.sha256`, `B5.artifact_default`), each with a per-field reason in
  `SNAPSHOT-DIFF.json` and each re-verified on every run (I6). Only the last is arguably semantic,
  and it is a path, not a criterion.
- Every v1 fixture, the 31/1 report and the 5 Golden files are hash-pinned as invariants (I3).

### Outcome (Lead run) — NOT the independent Task C result

| gate | result |
|---|---|
| step 1 integrity (I1–I7) | **PASS 75 / 0 FAIL**, rc=0 |
| step 2 parsed-IR contract | **PASS**, rc=0 |
| step 3 frozen checker `--all` | **32 PASS / 0 FAIL / 0 BLOCKED / 0 NOT_RUN**, rc=0 |

**Log paths:** `p1v2-20261013-d025/lead/T6-real.out`, `lead/T7-fullentry.out`,
`lead/contract-neg/entry-attacks-v2/*.log`, `lead/lead-crosscheck-run-diff.md`,
`lead/TASK-B-negative-matrix.md`, `lead/SNAPSHOT-MANIFEST-DIGEST.txt`.

**Cross-check vs v1 (Lead, non-authoritative):** all 32 check names identical in order between the v1
`run-20261013T025929Z` and the v2 `run-20261013T083240Z` summaries; **exactly one verdict changed**
(`B8.CACHE4` FAIL→PASS); `ENV.COMPILER`, `B6.N01`, `B7.D009_EMITTED` differ only in path/sha text.
Independently confirmed by the Challenger with `csv.DictReader`. Reporting consistency, not
acceptance evidence.

### Two D025 §6 negatives the frozen oracle provably cannot catch (measured, not asserted)

1. **cache4→cache0.** `B8.CACHE4` asserts only *what is carried*, never the value, and its evidence
   string is built with a capture-group regex, so `re.findall` returns the captured KEY and the
   verdict omits the value. A **well-formed** `l2_cache_mode = 0` artifact (parses rc=0, empty
   stderr — verified, so this is a value test and not a syntax accident) scores **32 PASS including
   `[PASS] B8.CACHE4` under the frozen checker alone**.
2. **Terminal `PIPE_ALL` removed.** `B8.TERMINAL` (`p1_check.py:1561-1576`) has **no FAIL path**; it
   records PASS unconditionally. Deleting the artifact's single terminal
   `hivm.hir.pipe_barrier[<PIPE_ALL>]` changes **zero** verdicts across all 22 positive checks.

D025 §1(2)/§5(2) forbid editing the frozen oracle, so the remedy ADDS two separately named gates:
`verify/check_snapshot_integrity.py` (I1–I7) and `verify/check_contract_gate.py` (parsed-IR,
**entry-scoped**).

### Negative matrix — each attack on a copy that was then RE-SEALED and re-parsed

Re-sealing makes the manifest sha256 match the mutated tree, so the hash pin cannot be the rejecting
mechanism. Every mutated artifact was then re-parsed with the pinned compiler: **all 8 still parse**,
so no rejection below is a syntax artefact. All 8 mutations were confirmed applied (sha differs from
the pinned artifact).

| mutation | rc | parses | assertion that fired |
|---|---|---|---|
| cache4 → cache0 (well-formed) | 2 | yes | `store-carries-l2_cache_mode=4` |
| carrier deleted | 2 | yes | `store-cache-attr-count` |
| carrier moved into a comment | 2 | yes | `store-cache-attr-count` |
| `{0 : i32} // {4 : i32}` | 2 | yes | `store-carries-l2_cache_mode=4` |
| terminal barrier deleted | 2 | yes | `contract.exactly-one-terminal-barrier` |
| barrier present, before the last store | 2 | yes | `contract.barrier-is-entry-terminal` |
| barrier moved **into the outlined VF** | 2 | yes | `contract.barrier-in-entry` |
| barrier nested in a **conditional** | 2 | yes | `contract.barrier-is-entry-terminal` |

### Two defects the Challenger found in the first version of these gates (both fixed)

1. **The digest was overstated.** It emitted only the 21 files in `INTEGRITY.json → snapshot_v2` plus
   the compiler. That list cannot contain `INTEGRITY.json` (a manifest cannot pin itself), and it also
   omitted the 5 Golden files and the linked bitcode — so three load-bearing identities had no row
   while the header claimed they did. Now generated by `build/make_digest.py` with explicit rows for
   the manifest, the bitcode and the Golden set: **47 rows, 47 OK / 0 FAILED, zero warnings**.
2. **The terminal check was not entry-scoped.** It searched the whole parsed module, and the artifact
   already declares a second function `@main_kernel_simd_vf_0`, so a `PIPE_ALL` moved into that helper
   satisfied "exactly one barrier" and "after the last store" while `@main_kernel` returned without
   terminal completion — i.e. the gate passed on exactly the defect D025 §6 asks it to catch. The
   Challenger executed that counterexample against the old gate and got a full green entry. The gate
   now splits functions by brace matching, reads the entry name from `expected.json →
   abi.entry_symbol` (not hardcoded), asserts the store inside the entry and nowhere else, requires
   the module-wide barrier count to remain 1, requires the barrier to be **in** the entry, and
   requires that nothing but `return` follows it there.
3. **The snapshot's own README described the superseded gate.** It still named
   `contract.barrier-after-last-store`, an assertion that no longer exists after the scoping rewrite,
   and described the module-global semantics the Challenger had falsified — so the frozen artifact
   carried documentation contradicting the code it ships with, the same class of defect as the stale
   header comments in the Late IR (README scope note 2). Corrected, then re-sealed. No assertion, no
   pinned input and no checker byte changed; only `README.md` and therefore `INTEGRITY.json`.
   Superseded working copies from the earlier round are labelled, not deleted, in
   `lead/contract-neg/README.md`.

**Digest wording correction (Challenger, accepted — the earlier text was backwards):** a retained
digest, when rechecked, **does** detect changes made *after* its creation. It **cannot** certify
builder correctness *up to* publication. TOCTOU is the separate, narrower window between hash
verification and execution; nothing in-tree closes it.

### Defect found and closed during this round

The first `run_p1v2.sh` forwarded `"$@"` to the checker. Since `p1_check.py` accepts `--artifact`
while the gate pins only `impl/vadd_late.mlir`, `run_p1v2.sh --artifact <cache0 file>` passed I1–I7
and then scored **32 PASS including `B8.CACHE4` for a file that was never pinned** — voiding the
value-custody I7 exists to provide. Reproduced independently by the Lead and the Challenger. The entry
now takes **no arguments** and names the pinned artifact explicitly; any argument aborts rc=2.

### Two Lead errors of my own, disclosed (same class the Challenger keeps catching)

1. My first negative-runner filtered a list of **indices** as if they were lines, so it applied **no
   mutation** and printed a table of "ACCEPTED" — every row was an unmutated copy passing. Fixed, and
   the runner now asserts the artifact's sha actually changed.
2. My VF-barrier mutation deleted a line and then inserted using a **pre-deletion** index, landing the
   barrier after the VF's `return`, so it was rejected by a parse error rather than by the contract.
   Fixed by recomputing indices after the deletion.
Neither reached the claims in this section, but (1) is precisely the failure mode where a green table
proves nothing, so it is recorded rather than quietly fixed.

### Integrity anchor and residual risk

`lead/SNAPSHOT-MANIFEST-DIGEST.txt` (sha256 `4dfbc856…`) sits **outside** the snapshot and carries
explicit rows for `INTEGRITY.json` (`7f9b92bf…`), all 21 pinned inputs, the compiler, the linked
bitcode and the 5 Golden files; `sha256sum -c` verifies 47/47 with no warnings. **Residual, NOT
closed:** the digest proves integrity from publication forward, never builder correctness before it,
and the TOCTOU window between verification and execution stays open — nothing in-tree re-reads it.
Closing that needs an independent second copy. Recorded as a residual, not claimed as resolved.

### Snapshot freeze — final identity (measured on disk, 2026-10-13 ~17:0x local)

The snapshot changed **mid-round** while the Verifier was measuring it (08:41–08:54Z). That was not
drift: it was exactly the two Challenger gate defects being fixed, plus the manifest re-seal. The
Verifier detected the change and asked whether it was intentional; it was, and the bytes below are
now final. Any *further* change is a new finding and must be reported as one.

| file | sha256 |
|---|---|
| `INTEGRITY.json` | `7f9b92bf9766f008bcf23ab6d34f791a300f260b218584d2d3b2d088c32d4807` |
| `SNAPSHOT-DIFF.json` | `02c1f93654698ee5119a33648cc6dcbb1b312ba8ab4fb13a984154b7d5e5d125` |
| `README.md` | `9219f9d4fa6465991b4a1ceabd6aa546e48d26c17563da0ee11cbcd20c0b3553` |
| `run_p1v2.sh` | `cdbeb5a415bde1c41a8e0d9e3618a6ba518edde088fe5f05986fc64346aec911` |
| `verify/check_contract_gate.py` | `60cbd386837b8edf9cf3995784b3273014eb0d2ea13c45694e6c644d73e9aec2` |
| `verify/check_snapshot_integrity.py` | `063ae76f081b865befd50674dfac7deb387bcdfe8704ddf32df04ab537d72ea5` |
| `verify/p1_check.py` | `fd5f0c078fcbfeb22f4f1739826ee707eb1a314edaa5bcc730badafb7c39da3d` (= v1, byte-identical) |
| `verify/expected.json` | `0439774bc8c21be67c83089fdd37dc7d4abc0dfd11d13c144c235ba102c60c22` |
| `impl/vadd_late.mlir` | `d1023ac1fdfd22c8d7f2aed5c71241fdd83039744b383e43b19255d10a31f36b` |
| anchor `lead/SNAPSHOT-MANIFEST-DIGEST.txt` | `4dfbc856658b87034c6f2562e9aaf15863ec0c148ba1b13faa54c5eb438690a8` |

**Only the harness/entry/manifest moved.** The functional core — artifact, `expected.json`,
`p1_check.py` (byte-identical to v1) and every fixture — was byte-stable across the whole window.

### Task C negatives — structural defect found by the Lead (2026-10-13 ~17:0x local)

The Verifier's first negative set (`verify/task-c/negatives/`, 16:58–16:59 local) produced 7 cases all
rc=2, but **all seven BLOCKED at step 1**: the mutants were copied from the *current* tree and then
mutated, so `INTEGRITY.json` in each copy still pinned the **original** artifact sha and the integrity
gate fired first. Steps 2–3 never ran. The rejecting mechanism was therefore the **hash pin**, not the
semantic assertion D025 §6 asks for — and D025 §6 names exactly that trap.

Per case, as it stood: n1/n2 showed a semantic line, but from integrity gate **I7** (a regex over the
file *text*), so the parsed-IR `contract.cache-value-is-4` had still never executed on a cache0
artifact; n3/n4 showed **no semantic assertion at all** — n4 in particular does not demonstrate
"remove the trailing `PIPE_ALL` → the sync contract check trips"; n5 proved only that the oracle
cannot be edited undetected (I6), not that the runtime SHA comparison is load-bearing; n6a/n6b were
correct as-is (blocking at step 1 *is* the intended outcome for an integrity negative).

Repair required of the Verifier: make the **named semantic check** the rejecter, by invoking the
copied gate/checker directly (n1 → `contract.cache-value-is-4`; n2 →
`contract.store-carries-one-cache-attr`; n3 → `B7.D009_CARRIER`/`B7.D009_EMITTED`, `p1_check.py:1151`
and `:1267`; n5 → `ENV.COMPILER` BLOCKED via `verify_compiler_and_golden`, `p1_check.py:1313-1329`,
which hashes the path against the recorded sha before any B check) or by re-sealing the full entry
(n4 → `contract.exactly-one-terminal-barrier`). Each case must also record that the mutant still
**parses** rc=0 with empty stderr, so a parse error is not what rejects. **This is a test-harness
repair; no frozen expectation is changed.** The Challenger independently reached the same distinction
and supplied the per-case recipe, which the Lead relayed with the assertion names verified against
the source.

### Task C has LANDED — independent Verifier results (2026-10-13 ~17:0x local)

Deliverables: `verify/TASK-B-snapshot-audit.md`, `verify/TASK-C-report.md`, raw logs in
`verify/task-b/` and `verify/task-c/`. Verifier-reported model route: `mimo-v2.6-flash` — which
**matches** the already-recorded finding in
`p2-20261013-d012/lead/model-identity/model-identity-evidence.md`: CLAUDE.md's "Sonnet/Haiku/Opus"
labels are **agent-type routing labels, not served models**. The Sonnet slot is served by
`deepseek-v4.1-flash`, the Haiku slot by `mimo-v2.6-flash`, the Opus slot by `gpt-6.1-sol`; **no
agent in this project is served by a Claude model.** D025 §3 asks for exactly this check by real
routing rather than by alias, and the Verifier's report is consistent with it. Carried into the
final user report so model identity is not overstated.

**Task B — the v2 oracle is functionally EQUIVALENT to v1, not a relaxation.** Derived independently
*before* reading my `SNAPSHOT-DIFF`/gate files, then compared: full agreement on all five leaves.
`p1_check.py` byte-identical (`fd5f0c07…`, `cmp` rc=0). B4 enumerates **every** `exp` access and
shows the checker reads only `abi`, `geometry`, `compiler.path/sha256`, `golden.*` and
`B7.d009.discrimination_selftest` — it never reads `schema`, `frozen_utc`, `author` or
`B5.artifact_default`, so those leaves are identity/documentation only. Artifact differs on **line
189 only** (confirmed by the Lead's own `diff`: 274 lines both sides).

**Task C — the authoritative run.** `task-c/entry2-20261013T090206Z.log`, **rc=0**: step 1 I1–I7 all
PASS, step 2 contract gate 18/18, step 3 frozen checker **32 PASS / 0 FAIL / 0 BLOCKED / 0 NOT_RUN**,
`acceptance checker exit code: 0`. D012 oracle C1–C8 **8 PASS** (absolute paths). 11 fixtures
**11 GREEN / 0 RED / 0 NOT_RUN**, parse-stderr empty, compiler sha identical before/after. N7
reproduces all 10 TASK-A hashes and recounts v1 `summary.tsv` as 32 rows / 31 PASS / 1 FAIL. The
Verifier did **not** write into the snapshot: the Lead re-hashed the frozen tree 9/9 unchanged.

**The repaired negatives are correct** (three ways each — ENTRY / contract gate direct / frozen
checker direct): N1 cache0 → `FAIL contract.cache-value-is-4`, and the direct checker scores
**rc=0 / 32 PASS with the `B8.CACHE4` evidence string byte-identical to the control**, so F1's
value-blindness is now confirmed **by execution**; N2 → `contract.store-carries-one-cache-attr`;
N3 → `B7.D009_CARRIER` + `B7.D009_EMITTED` (rc=1, 30/2); N4 → `contract.exactly-one-terminal-barrier`
+ `contract.barrier-in-entry`, direct checker rc=0 / 32 PASS with `B8.TERMINAL` printing
`NONE PRESENT` — F3 confirmed by execution; N5 → a single `[BLOCKED] ENV.COMPILER` with zero B-items;
N6a/N6b correctly retained as integrity negatives. The `字符串注释` loophole is closed three ways.

**Two plan-prediction failures, reported not hidden** (both inherited from the frozen v1 oracle, and
both the reason the additive gates exist): the frozen checker does **not** reject cache0 (N1) and does
**not** reject a removed terminal `PIPE_ALL` (N4). D025 forbids patching the oracle, so these are
recorded as properties to be ruled on.

### Still open

- **C9 — DELIVERED and verified.** Verifier `task-c/c9/c9-chain.log`, its own run on `impl/vadd_late.mlir`
  (`d1023ac1…`, compiler `0761c6e6…`): stage 1 `--hivm-normalize-ops` rc=0, stderr 0 bytes, stdout
  `a4d5af20…`, `l2_cache_mode = 4 : i32` count=1; stage 2 `--hivm-normalize-ops --convert-hivm-to-std`
  rc=0, stderr 0 bytes, stdout `0a5d3db6…`, line 33
  `call @store_ubuf_to_gm_1d_float(%cast_5, %cast_6, %c0_i32, %c4_i32)` with line 3
  `%c4_i32 = arith.constant 4 : i32`; the attribute is gone (now an operand) and the load libcalls
  carry no mode. **Two independent runs — the Verifier's and the Lead's read-only probe — produced
  byte-identical outputs on both stages**, which is what D025 §6 asks for. Harness note, **RESOLVED**:
  the `A3` FAIL was **the Verifier's own** parser bug (its type-list splitter broke on bare commas
  inside `memref<?xf32, strided<[?], offset: ?>, …>` generics), fixed with a bracket-depth-aware
  splitter — **10/10 assertions now PASS**, and it also proved the only `i32` constant equal to 4 in
  the whole file is `%c4_i32`, consumed *only* by the store call. **The log handles its own bad
  assertion the right way and this is worth keeping:** it *retains* the spurious FAIL visibly
  (`c9-chain.log:87`), follows it with a `--- CORRECTION ---` header (`:107`), the corrected
  `A3 … : PASS (i32)` (`:140`) and `ALL C9 CONDITIONS: PASS (10/10)` (`:160`), then appends a closing
  READER NOTE (`:188`) so a reader who scans only assertion lines still sees the resolution. The FAIL
  is kept deliberately — **a visible, explained FAIL beats a quietly dropped assertion**, which is the
  same standard D025 §6 sets for the round. Its root-cause statement is also more precise than the
  Lead's first description: the `10 types` came from splitting the **call site's own** `: (…) -> ()`
  clause on bare commas (which shreds the 4 real types inside the nested `memref<…, strided<…>>`
  generics), **not** from cross-contamination with the callee's `func.func` declaration. No wrong
  version of that diagnosis survives anywhere in `docs/` or this round's tree.
- **C9's scope boundary, stated by the Verifier itself (`extension (e)`) and important:** C9
  establishes that the mode reaches the **libcall call boundary carrying 4**, and that the linked
  bitcode is byte-identical to the pinned identity (c310 `5e7ccae2…`, c220 `b1dfbce3…`, both
  re-hashed and unchanged during C9). It did **not** relink, disassemble, or execute against the
  bitcode, so *the bitcode **body**'s consumption of operand 4 is outside C9*. **That half comes only
  from the Challenger's fresh disassembly** — so the full chain is two composed, separately owned
  evidences: **artifact → call boundary** (Verifier, C9) and **call body → hardware intrinsic**
  (Challenger). Neither alone is complete, and the record does not present either as if it were.
  C9's before/after set is otherwise all-unchanged: compiler, `INTEGRITY.json`, artifact,
  `expected.json`, `p1_check.py`, digest 47 OK / 0 warnings. The Verifier's two semantic negatives
  are also honest: it reports **N1 (cache0) value-blindness** and **N4 (`PIPE_ALL` removed) as
  plan-prediction failures**, matching the Lead's F1/F3 exactly.
- ~~Admission C7 (bitcode hard gate)~~ — **CLOSED, see below.**
- ~~Challenger phase-2 verdict~~ — **APPROVE, see below.**

### D025 round CLOSED — Challenger verdict APPROVE (2026-10-13, persisted ~17:41 local)

Verdict persisted **verbatim** at `review/challenger-d025-final-verdict.md` (the Challenger's reviewer
instructions forbid it writing report Markdown, so the Lead persists it unedited — `MEMORY.md`
`challenger-refuses-md-writes`). Headline: *"FINAL VERDICT: APPROVE for D025 versioned P1 re-pin,
scoped to the frozen F32 single-tile compiler acceptance. §7(1)-(7) are confirmed from independent
diff/hash/source review and raw evidence."* It then states the Lead must persist the verdict and
update `docs/status.md` + the admission report, *"then P1 can be formally CLOSED and P2 READY TO
REQUEST AUTHORIZATION. P2 remains unauthorized; stop at its entry."*

**§7(4) — the bitcode obligation — was settled the hard way, and C7 is now MET.** Rather than reuse
the D012 round's historical dump (which is what C7 explicitly refuses), the Challenger **freshly
disassembled the current pinned bitcode** through the producer `llvm-link -S`: rc=0, **zero stderr**,
bitcode hash `5e7ccae2…` **unchanged before and after** the operation. Output
`/home/s00653124/workspace/hivm-poc-evidence/challenger-d025-zqumt69b/final-pinned-bitcode.ll`
(`5ab225a1…`, 166 690 439 B, sidecar stderr 0 B). **The Lead re-read the pinned file independently**
and reproduced both decisive claims:

```
746746: define dso_local void @_mlir_ciface_store_ubuf_to_gm_1d_float(
          ptr nocapture noundef readonly %src, ptr … %dst,
          i32 noundef %atomic_kind, i32 noundef %l2_cache_mode) …
746864:   %and9.i107 = zext i32 %l2_cache_mode to i64
746866:   %shl10.i108 = shl i64 %and9.i107, 60
746868:   %or8.i106  = or i64 %shl7.i105, %shl10.i108
746870:   tail call void @llvm.hivm.MOV.UB.TO.OUT.ALIGN.V2.DV(…, i64 %or11.i109, …)
```

The cache parameter is **dataflow-connected to the final hardware intrinsic** — `zext → shl 60 → or`
into the descriptor that `MOV.UB.TO.OUT.ALIGN.V2.DV` consumes — not merely present in the signature.
That is exactly what the gate demanded and what symbol arity alone could never show.
`MOV.UB.TO.OUT.ALIGN.V2.DV` occurs **1331×** module-wide; source HEAD `e878f349…` with the 11-file
D012 change set.

**Verdict caveat carried into the record verbatim:** *"Verifier VF/nested-barrier negatives remain
NOT_RUN; my separate clean-parse resealed counterexamples supply those executed checks."* The
Challenger's counterexample logs are in `challenger-d025-zqumt69b/` (`barrier-in-vf.log`,
`scoped-vf.log`, `scoped-if.log`, `cache0-new.log`, `no-terminal-new.log`,
`builder-controlled-resealed.log`). **Ledger note:** C7's executed artifact is **Challenger-owned**,
not Verifier-owned; recorded so the user can weigh it rather than have it read as Verifier evidence.

**Admission now stands at 8 of 9 MET.** Every condition except **C8 (explicit user authorization)**
is MET on executed evidence. `docs/decisions.md` D031, this section and `P2-ADMISSION.md` now record
**new P1 = CLOSED** and **P2 = READY TO REQUEST AUTHORIZATION, NOT authorized**.

**Residual risks the round does NOT close** (the Challenger's list, adopted in substance):
the frozen checker alone is insufficient — a well-formed `l2_cache_mode = 0` artifact and an artifact
with the terminal `PIPE_ALL` deleted each still score **32 PASS** under `p1_check.py` alone, so
approval belongs to the **full three-step entry**, never to step 3 in isolation; the retained digest
shares writable trust with the snapshot and detects drift only when independently retained and
rechecked (not builder correctness, not TOCTOU); the contract gate is a **regex gate scoped to the
frozen input, not a general CFG proof**; and **no** hardware numerical, ELF-launch, final-kernel
LLVM-link, CodeGen-E2E, A5 binary ABI/TVM-FFI, dynamic UB/runtime, or P2 stage-cut/double-lowering
validation is claimed. D012's conservative DMA subset is the whole of what is supported.

### Lead errors this round (disclosed, same class the Challenger keeps catching)

- **The B1 caveat was right and my push-back was wrong.** The Verifier's `TASK-B-snapshot-audit.md`
  (:70, :268) notes that "every file under `verify/` is byte-identical to its v1 source" does **not**
  hold as a universal — 2 of 16 files (`check_snapshot_integrity.py`, `check_contract_gate.py`) are
  **new harness files with no v1 source**; accurate counts **14 identical / 2 new / 0 differing**.
  I grepped `snapshot/README.md`, found every occurrence properly scoped, and told the Verifier its
  caveat misattributed my wording. **I checked the wrong file.** The overclaim is real and it is
  mine, at `PLAN.md:52` — my own Task-B requirement to the Verifier. Corrected: `PLAN.md` item 1 now
  says "every file … *that has a v1 counterpart* is byte-identical" with those counts and an
  explicit note that the Verifier caught it. `snapshot/README.md` was already scoped correctly and is
  left untouched. The Verifier has been sent a retraction telling it **not** to reword its caveat,
  and the Challenger has been corrected too. Lesson: when checking whether I overclaimed, grep the
  whole evidence tree for the phrase rather than the one document I assume contains it.
- The Verifier's finding 6 reports `lead/d025-user-instruction.md` as having moved from
  `p2-20261013-d012/lead/`. Checked: it is present only at `p1v2-20261013-d025/lead/` (6822 B), and
  **no manifest, digest or record in the D012 round pins it** — no frozen evidence was disturbed.

### Next action

**None on the Lead's side. The round has executed its mandate and stopped at the P2 entrance.**
Task C landed, the Challenger returned **APPROVE**, and `P2-ADMISSION.md` (C1–C9), this section and
the D031 row are updated. The user has been given the closing report: new P1 **CLOSED**, the residual
risks above, and P2 **READY to request authorization but NOT authorized**.

**The Lead is now blocked on the user, deliberately.** D025 §1(7) makes the user's authorization the
only remaining gate, and §8 ends *"停止在 P2 入口，等待明确授权。不要自动继续开发 P2。"* No P2–P6
production code is written — **not** an implementation, not a design edit, not a `git` operation on
either repo — until the user says so. Two housekeeping items remain open and are **not** gates:
(1) ~~the Challenger's promised remainder~~ — **RESOLVED.** It confirmed the closing line was not a
leftover and re-sent the full final text, which is now appended **verbatim** below a rule in
`review/challenger-d025-final-verdict.md`, with nothing above the rule edited. The remainder adds the
§7(1)–(7) confirmation table, a second table mapping each serious finding to the distinguishing
experiment that resolved it, and its explicit NOT_RUN list. **The Lead independently verified the two
source lines it cites:** `bishengir/lib/Conversion/HIVMToStandard/regbase/HIVMToStandard.cpp:1143` is
inside `verifyStoreCacheMode`, which rejects out-of-set modes with *"invalid store l2_cache_mode N;
valid store modes are 0, 1, 2, 4"* — the §7(5) preflight; and
`bishengir/lib/Template/lib/RegBase/Vector/Copy1D.cpp:265` is the rank-one store body whose
`isAddress32ByteAligned` gate falls back to `store_ubuf_to_gm_1d_by_scalar`, i.e. exactly the
conservative-subset condition. (2) Both repos stay
clean and untouched; nothing in this round modified Golden, the v1 oracle, or any production file —
**the round itself wrote no production code.** (The pre-existing D012 worktree change set was
committed afterwards, separately and on the user's explicit instruction — see below.)

### D012 改动已提交为规范 commit（2026-10-13 19:47 local，用户指令）

The user instructed that the current code changes be formed into a proper commit. **Only
AscendNPU-IR-1 had anything to commit**; `hivm-poc-control` and `hivm-poc-evidence` are **not git
repositories** at all, so `docs/` and the evidence tree are unversioned by design and were not
touched by this.

| | |
|---|---|
| repo | `/home/s00653124/workspace/AscendNPU-IR-1` |
| branch | `feat/tilelang-hivm-late-pipeline` (already a feature branch — **not** master; no upstream is configured, so this is purely local) |
| commit | **`80e29e6f27192833d2d6cd3fd846e0b78ee02d1c`** — `[HIVM] fix: carry the store L2 cache policy through to the DMA descriptor` |
| parent | `e878f3492` (the HEAD recorded throughout this round's evidence) |
| content | exactly the 11 files / 288 insertions / 32 deletions recorded in §2 of the ruling, all under `bishengir/` |
| worktree | clean after the commit (porcelain 0), no hooks, **no untracked or build artifacts swept in** |

**One commit, deliberately, not a stack.** The implementation and the two test files **cannot** be
split into a green sequence: committing the implementation alone leaves the lit `CHECK` lines
asserting a 3-operand store libcall that no longer exists (red), and committing the tests first is
equally red. A layered history whose intermediate states were never compiled would be the *appearance*
of a tidy history without the substance — and re-verifying each intermediate build is not possible
here anyway, since CLAUDE.md forbids concurrent builds against one build directory. The repo's own
convention is one commit per logical change (`[HIVM] fix: run A5 code motion before cross-core sync`),
so a single atomic commit is also the house style.

**Provenance effect — recorded, and not a re-verification trigger.** Every piece of this round's
evidence was produced while **HEAD was `e878f3492` with those 11 files modified in the worktree**. HEAD
is now `80e29e6f2`. **No file content changed** — `git commit` records the worktree bytes verbatim and
never rewrites them, so the compiler (`0761c6e6…`), the controlled artifact (`d1023ac1…`), the linked
bitcode (`5e7ccae2…`) and the 47-row digest are all still exactly what was verified, and **nothing in
Task B, Task C or the Challenger's verdict is invalidated**. What is superseded is the *phrasing*: the
Challenger's §7(7) line "Current source HEAD remains `e878f34927fb5b77fd8c7257fda3a95a3560f2ec`" now
describes the **parent** commit, i.e. the state as reviewed. The verdict file is verbatim and stays
untouched; this note is where the correction lives. **Nothing was pushed** — CLAUDE.md requires
explicit consent for that, and none was given; the branch has no upstream in any case.
