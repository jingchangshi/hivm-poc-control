# POC status

> **记录约定（用户指令，2026-10-13）— 本文件与 `docs/decisions.md` 是唯一的权威状态记录。**
> 此后一切阶段状态、决策、验收结果与交付物都记录在 **`hivm-poc-control`**。`hivm-poc-evidence/`
> 保留为**仅本地**的原始证据存放处：**不发布、不推送到任何外部托管**（GitHub 等），用户已明确
> 表示不再管理它。该树内含一个**已放弃的本地 `git` 仓库**残留（2026-10-13 20:05–20:08 由本会话
> 之外的操作创建：分支 `master`、**零 commit、零 ref、从未推送或拉取**，远端指向一个外部
> `hivm-poc-evidence` 仓库——其 URL 含凭据段，因此**刻意不复制进本记录**）。**保持原样，不要清理、
> 不要重置、不要推送、不要据它推断已发布任何内容。** 需要"不依赖证据树即可读懂"的状态时，以本
> 文件为准。

- Date: 2026-10-13
- **现状（最新，2026-10-13 晚）— 以此条为准；其下为历史记录，不改写。** 新版 P1（D025 版本化重固定，
  登记为 **D031**）**已正式 CLOSED**：Verifier 权威入口 **rc=0**，**32 PASS / 0 FAIL / 0 BLOCKED /
  0 NOT_RUN**（完整性闸门 I1–I7 → 契约闸门 18/18 → 冻结 checker 三步全过）；D012 fixtures
  **11 GREEN**；D012 oracle **C1–C8 8 PASS**。**D012 修复已落地并独立验证**——改动集 11 文件 /
  288+32 行（全部在 `bishengir/`），已提交为 **`80e29e6f2`**（`AscendNPU-IR-1` 分支
  `feat/tilelang-hivm-late-pipeline`，父 `e878f3492`，**未 push**）。**Challenger 裁决 APPROVE**，
  D025 §7(1)–(7) 全部确认，全文逐字存于
  `p1v2-20261013-d025/review/challenger-d025-final-verdict.md`。**原始 P1 记录逐字节未变**
  （31 PASS / 1 FAIL 作为历史保留）。**P2 = AUTHORIZED（登记为 D032，2026-10-13）**——用户以
  `docs/p2-goal-1013.md` 第 7 行给出明确授权，补足准入条件 **C8**；范围 **P2-A…P2-D**，完成后
  停在 P3 门禁，**P3–P6 仍未授权**。P1 自带的尚余风险（未被任何准入
  条件关闭）：冻结 checker 单独不足、摘要共享可写信任域与验证—执行 TOCTOU 窗口、契约闸门非通用
  CFG 证明、硬件数值/ELF/CodeGen E2E/A5 ABI/TVM-FFI/运行时 UB 全部 NOT_RUN、P2 的 Late 阶段边界
  与重复 lowering 问题未解。详见本文件 "D025 versioned P1 re-pin"、"D012 改动已提交为规范 commit"
  两节与 `docs/decisions.md` D031 行。
- Current stage (historical): **P0 gate CLOSED by user adjudication (D021); P1 AUTHORIZED.** The user decided every
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

## P2 启动（2026-10-13 晚）— 用户授权与准入复核

**授权来源。** 用户指令 `docs/p2-goal-1013.md`（15 222 B，2026-10-13 20:16 创建）。其第 7 行即为
**P2-ADMISSION 条件 C8 所要求的明确用户授权**：*"我已审阅 P1 v2 的 D031 验收记录，现在正式授权
P2-A 至 P2-D 的生产代码实施。本提示词即为 P2-ADMISSION 中 C8 所要求的用户授权。"* 登记为
**D032**（`docs/decisions.md`）。范围：P2-A…P2-D；**P3–P6 仍未授权**；完成后停在 P3 门禁并向用户
提交结果（goal §9）。

**§8 第 1 步：准入复核（Lead，仅做必要核对，不重跑 E0/P1 全量验收）。** 全部证据与准入记录见
`hivm-poc-evidence/p2-20261013-late-pipeline/lead/p2-admission-check.md`。结论：

- 本地齐全：snapshot（含 `INTEGRITY.json`、`run_p1v2.sh`、冻结 checker）、Verifier
  `verify/TASK-C-report.md`、Challenger `review/challenger-d025-final-verdict.md`、`P2-ADMISSION.md`。
- **带外 47 条摘要 `sha256sum -c` → 47 OK / 0 NOTOK**（含任何树内检查都无法提供的那一条：
  `INTEGRITY.json` 自身 `7f9b92bf…`；以及编译器 `0761c6e6…`、bitcode `5e7ccae2…`、5 个 Golden、
  整个冻结 v1 树）。
- 权威结论取自**本地实际日志**，不以文档叙述代替缺失证据。本地证据与登记状态**无冲突**，因此
  **没有重建、放宽或重新生成任何历史 Oracle / Golden**。
- Git：`AscendNPU-IR-1` 工作区 **clean**，`80e29e6f2`（父 `e878f3492`），分支
  `feat/tilelang-hivm-late-pipeline`，**未 push**；`tilelang` 工作区 **clean**，`cc56643d`，
  分支 `feat/hivm-backend-poc`。单一 worktree，增量构建目录完好，未新建 worktree。

**模型路由实测（架构评审前必须报告，goal §3）。** 网关回显的 `model` 字段：Lead/implementor 槽
`group/auto-deepseek-v4-1-flash` → **`deepseek-v4.1-flash`**；verifier 槽
`opencode-go/mimo-v2.6-flash` → **`mimo-v2.6-flash`**；challenger 槽 `codex/gpt-6.1-sol` →
**`gpt-6.1-sol`**。**与用户要求的分工不一致**：启动脚本
`~/SoftwareProfiles/claude/run_claude_using_magpie.sh` 中用户期望的
`codex/gpt-6.1-sol` / `opencode-go/claude-haiku-5-5` / `claude/claude-opus-5-5` 三行是**被注释掉的**，
生效的是上面三行。**本项目没有任何 agent 由 Claude 模型服务**；`implementor-sonnet` /
`verifier-haiku` / `challenger-opus` 只是 **agent-type 路由标签**。**任何裁决不得记作 "Opus 5.5
审查"，任何验证不得记作 "Haiku 5.5" 运行。** Lead 不擅自改动启动脚本（属环境改动，超出本任务授权）。

**本轮交付文档（goal §9）。** 已建 `docs/p2-architecture.md`（入口设计，待 Challenger 评审）；
待建 `docs/p2-pass-trace.md`、`docs/p2-binary-abi.md`、`docs/p2-verification.md`。证据树本轮为
`hivm-poc-evidence/p2-20261013-late-pipeline/{lead,impl,verify,review}/`。

**下一动作。** §8 第 2、3 步：Verifier 冻结 P2 独立验收测试（agent `verify`，已启动）；
Challenger 评审入口设计与 Pass 切入点（agent `review-2`，已启动）。二者完成后再启动 Implementor。

### P2 构建身份预规划（2026-10-13，Lead，实测）

**实测事实。** `bishengir-opt` 链接全局属性 `BISHENGIR_LIBS`
（`bishengir/tools/bishengir-opt/CMakeLists.txt:24`），而 `add_bishengir_library`
（`bishengir/cmake/modules/AddBiShengIR.cmake:28-38`）把 `BiShengIRRegBaseCompileLib` 也追加进同一属性。
执行确认：`bishengir-opt --help` 列出 `--bishengir-compile-regbase`。推论：**regbase 库的任何改动都会
重链 `bishengir-opt`**。

**后果。** P1 带外摘要把 `build/bin/bishengir-opt`（`0761c6e6…`）与
`build/lib/meta_op.aiv.c310.O2.bc`（`5e7ccae2…`）**按绝对路径** pin 住，`run_p1v2.sh` 第 1 步
（不变量 I5）每次都复核。因此 P2 只要构建任何目标，`bishengir-opt` 字节就会变，P1 入口会在编译器
那一行 BLOCK。**不得为迁就 P2 构建而修改快照或其摘要**（D031 与 goal §6.1 均禁止）。

**已执行与计划。** (1) **已保全**（任何 P2 编译之前）：`p2-20261013-late-pipeline/lead/
preserved-p1-compiler/` 存有 `bishengir-opt`（`0761c6e6…`）、`meta_op.aiv.c310.O2.bc`
（`5e7ccae2…`）与 **P2 之前**的 `bishengir-compile`（`c6838fc0…`，用于把行为变化归因于 P2 而非臆测）
的字节副本及 `SHA256SUMS.txt`；前两个哈希在复制后已与 P1 摘要**逐一核对一致**。(2) 在既有增量目录
构建（goal.md §6：不新建 worktree、不复制 3 GB 构建树）。(3) 构建后**立即**把新 `bishengir-compile`
/ `bishengir-opt` 复制到构建树之外的稳定位置并记录 SHA，**所有 P2 测试一律使用该绝对路径**，以免
测到被重链过的二进制。(4) 从保全副本**恢复** pin 住的 `bishengir-opt` 到 `build/bin/`，重跑
`sha256sum -c` 期望重新 **47/47 OK**；诚实记录后果：此后 `build/bin/bishengir-opt` 与源码树**不
一致**，直到下一次构建。此计划的第 4 步已提交 Challenger 反查（是否应改用独立构建目录）。

### P2 前端侦察（2026-10-13，Lead，只读实测；未写任何生产代码）

在步骤 2/3（Verifier 冻结测试、Challenger 审查入口设计）进行期间，Lead 就**尚未决定的两处拼写**
做了只读实测，工具均为 P1/D031 pin 住的 `build/bin/bishengir-opt`（`0761c6e6…`），**未构建、未改动
任何仓库**。证据：`p2-20261013-late-pipeline/lead/pass-names-and-instrumentation.md`。

1. **保守 legalization 尾部的真实 Pass 参数名（20 项）**已由 `bishengir-opt --help` 实测确定并写入
   `docs/p2-architecture.md` §3.1 表格。其中 **3 个不是朴素推导**：`--hivm-memref-alloc-to-alloca`
   （非 `--alloc-to-alloca`）、`--hivm-mark-disable-load`、`--convert-ascend-dpx-to-hivmregbaseintrins`
   （`hivmregbaseintrins` 连写、无连字符）。SyncBlockLock finalize 家族是**五个**独立 Pass
   （`--hivm-insert-infer-sync-block-lock-num-and-init-func`、`--hivm-mark-sync-block-lock-with-subblock`、
   `--hivm-bind-sync-block-lock-arg`、`--hivm-lower-create-sync-block-lock`、
   `--hivm-insert-free-lock-var-before-return`），**全部保留**。注意存在一套 **simt** 同名 Pass
   （如 `--simt-insert-init-and-finish-for-debug`），其出现在 `--help` 中**不构成**它在 Late 模式运行
   的证据。此前两次源码 grep 均失败，故以二进制为准。

2. **`--input-ir-stage` 选项机制在生成器层面核实**，并**修正了** §2.1 的草稿错误：`Options.td` 中
   `"string"` **零命中**，字符串选项一律用字面量 **`"std::string"`**（`InjectIRFromFile`、`SaveTemps`，
   `Options.td:330-348`），空默认写作 `"\"\""`；`ConfigOption::getType()`（`ConfigOptions/Options.cpp:144-146`）
   原样使用 TableGen `type` 字段，故 `"string"` 属**未经验证的生成器路径**。已改为
   `"std::string"` / `"\"hir\""`。派生身份由**已生成的 `.inc`** 核对：字段 `inputIRStageFlag`、链式
   `setInputIRStage(const std::string&)` / `getInputIRStage()`（访问器去掉 `Flag` 后缀，见
   `CompileConfigs.cpp.inc:1542-1552` 的 `SaveTemps` 范例）、生成的
   `cl::opt<std::string,true>("input-ir-stage", …, cl::location(inputIRStageFlag))`
   （`CompileOptions.cpp.inc:977-980`）、以及 `options.<member> = config.getInputIRStage();`
   （`ConfigUtils.cpp.inc:103,241`）。结论：`def InputIRStage` 一经声明即自动重生成以上四处，
   **手写部分只有 pipeline-options 结构体成员与分支本身**。

3. **Pass instrumentation 机制实测可用**（goal §5 要求"真实 MLIR Pass instrumentation，而非源码 grep"）：
   pin 住的二进制暴露 `--mlir-print-ir-before-all`、`--mlir-print-ir-after-all`、
   `--mlir-print-ir-before/after=<pass-arg>`、`--mlir-print-ir-after-change`、
   `--mlir-print-ir-tree-dir=<dir>`、`--mlir-pass-statistics`、`--mlir-pass-statistics-display=<value>`、
   `--mlir-disable-threading`、`--mlir-print-ir-module-scope`。∴ 该要求**无需新构建即可满足**；
   `--mlir-disable-threading` 为**必需**（否则 after-all 顺序不确定、排序断言不可复现）。
   §3.2 的"零执行"negatives 以 `--mlir-pass-statistics` 的逐 Pass 计数为证物。

以上三条已同步给 `verify`（作为事实输入，不断言其判据）与 `review-2`（供其反查 §2.1/§3.1）。

**状态**：本控制仓库 `hivm-poc-control` 非 git 仓库，记录不随版本化。两生产仓库仍干净且停在 D031 head
（`AscendNPU-IR-1` `80e29e6f2`、`tilelang` `cc56643d`）。**P2 生产代码 0 行**；`verify`、`review-2`
仍在运行（步骤 2/3）。下一步：待两者结论落地后进入步骤 4（Implementor 实施 P2-A）。

### P2 架构评审往返（2026-10-13，Challenger ↔ Lead；步骤 3 进行中）

Challenger（`review-2`，challenger-opus 槽位，实测 served model 为 `gpt-6.1-sol`）对入口设计给出
**REVISE**，两轮共 11 项。**全部经 Lead 独立复核源码后采纳**（Challenger 是只读评审者，其结论不
可凭信任记录）。逐字记录与复核表：
`p2-20261013-late-pipeline/review/challenger-entry-design-verdict-PARTIAL.md`。

> 文件名的 `PARTIAL` 是**有意的**：第一轮 verdict 经团队通道**截断**于第 5 项
> （"Test unsupported c…"），完整文本已向 `review-2` 索取，未到之前该文件不得被当作 Challenger 的
> 完整结论引用。

**第一轮五项**（均已落入 `docs/p2-architecture.md`）：

1. **TableGen 字符串类型**：`"string"` → `"std::string"`（Lead 在收到该条之前已独立改正）。
2. **Late 探测必须独立于 target 分流**——**这是 Lead 文档的真实缺陷**。`hasRegBaseTarget` 在
   `bishengir-compile.cpp` 被求值三次（`:114`、`:147`、`:184-186`），其余落到 `:188` 的
   `runBiShengIRPipeline`。∴ **不带 `--target=` 的 `--input-ir-stage=late-hivm` 会静默走普通
   membase 流水线**，可能产出看似合理的产物。§2.2 现要求无条件从 `argv` 探测一次并在 `:114` 前
   硬失败。
3. **ABI/bitcode/sync/output 选项策略**——Lead 复核**发现更尖锐的事实**：`EnableStaticBarePtr`
   带 `emitGetterSetter = 0`（`Options.td:153-160`、`CompileConfigs.cpp.inc:981`），
   ∴ 转移助手 `HIVMCFromBiShengIRConfig` **结构上无法**转移它，A5 消费者读取
   `HIVMCMainConfig` 默认值 `true`（`hivmc/Config.h:830`）。**在 regbase 路径上
   `--enable-static-bare-ptr=false` 静默无效**——这是 ABI 相关的真实发现。§2.3.1 新增：逐字调用
   转移助手 + 逐选项裁定表。
4. **C310 bitcode 硬失败有两个静默点，不是一个**——挂载期 `addIfExists`
   （`BiShengIRCompileMain.cpp:79`）；链接期 `HIVMCMainA5.cpp:110-117` 仅告警后继续，唯一硬失败是
   `bitcodePathList.empty()`（`:180`），而用户提供额外 bitcode 时它**永远不会触发**；另有
   `-Wno-override-module`（`:97`、`:227`）压掉重复符号诊断，使替代 bitcode 可冒充必需模板。
   §3.3 改为在入口对**解析后的路径**硬失败，并在 Late 模式**拒绝 `--link-aicore-bitcode`**。
5. **校验器必须是真正的预检**——D012 谓词 `verifyStoreCacheMode`
   （`HIVMToStandard.cpp:1131-1160`）由**转换 pattern** 调用，即运行在尾部第 10 项**之后**已运行
   1–9 项之时，无法满足 goal §4.2 的"破坏性变换之前"；`AppendDeviceSpec.cpp:145-148` 仅告警后
   **无条件覆写**（早退在 `:129-130`）。V1–V7 改为对未修改输入的独立预检。

**第二轮六项**（同样已采纳）：

- `../lib` 相对**真实可执行路径**解析，且 `real_path` **会解析符号链接**
  （`regbase/BiShengIRCompileMain.cpp:63-76`；路径来自 `lib/Tools/Utils/Utils.cpp:360-375` 的
  `getMainExecutable`(`/proc/self/exe`) + `real_path`）∴ 暂存必须是**真实 `bin/` + `lib/` 捆绑**，
  符号链接会静默指回构建树。
- 恢复 pin 住的 `bishengir-opt` 会使其 mtime 新于构建边，**Ninja 会认为该陈旧输出是最新的而不重链**；
  已记录，处置待 Challenger 的反例。
- "任何 ninja 目标都会重链 opt" **过宽，是 Lead 文档的错误**：两个二进制是**独立的链接边**，
  共享库只是共同输入；已改为**显式构建两者**。
- 追踪覆盖的四个前提（所有 PassManager 安装选项 / 关闭 `--mlir-print-ir-after-change` /
  完整日志经 `--mlir-print-ir-tree-dir` 归档 / **哨兵 Pass** 证明覆盖面）。已复核：
  `bishengir-compile.cpp:95,97` 在任何 PassManager 构造前注册选项；`runHIVMCCompileA5` 在
  `regbase/BiShengIRCompileMain.cpp:448` **进程内**调用（定义于 `hivmc/A5/HIVMCMainA5.cpp:944`），
  ∴ A5 的 MLIR 流水线（含唯一一次 `ConvertHIVMToLLVM`）**在同一追踪内**；ccec **子进程**不在，
  改由 P2-C 的二进制/ABI 检查覆盖。
- **函数不是 Pass 身份**：`inferLayoutOptimization`、`runRegBasePipeline`、`buildLowerAVEPipelines`、
  `runHIVMCCompileA5` 是 C++ 函数，追踪只显示它们添加的 Pass ∴ 其"未调用"须由**驱动级路由日志**
  （仿 `bishengir-compile.cpp:118` 的 `[INFO] Using merged native A5 regbase pipeline`）证明。
- 链接拓扑**不强制**校验器所在库，且**链接 ≠ 注册**：注册是独立显式动作
  （`include/bishengir/InitAllPasses.h:71` 组合各 dialect 的 `register*Passes()`，
  由 `GEN_PASS_REGISTRATION` 生成）。§2.4 改写为两部分要求，注册的**可执行证据**定为
  `bishengir-opt --help` 列出新参数。

**仍开放**：(a) 第一轮第 5 项截断尾部及其后各项 + 最终 APPROVE/REVISE/BLOCK 行；(b) §3.3
步骤 4 的建议（恢复进 `build/bin` vs 独立构建目录）；(c) 除四前提外是否还需要更强的 §3.2 证据；
(d) Challenger 第一轮第 1 项声称的**已执行**探针内容与写入位置（以保证"P2 生产代码 0 行"的陈述
准确）；(e) Challenger 正在构造的 Ninja 陈旧反例。

**状态**：P2 生产代码 **0 行**；两仓库干净、停在 D031 head；`verify`（约 26 分钟）与 `review-2`
仍在运行。步骤 4 继续等待步骤 2/3 收束。

### P2 架构评审第三轮（2026-10-13，Challenger 完整 verdict；步骤 3 收束中）

第一轮 verdict 的**截断尾部已由 Challenger 后续消息补齐**，完整逐字记录移至
`p2-20261013-late-pipeline/review/challenger-entry-design-verdict.md`（**取代**先前的
`…-PARTIAL.md`；后者原样保留，作为"当时收到什么"的诚实记录）。**最终 verdict：REVISE**（非
APPROVE、非 BLOCK）。**两项已 APPROVE 且不再是未决缺陷**：`"std::string"` 声明改正、以及冻结
匹配规格下的 case-2 `AppendDeviceSpec` 跳过。**三项是 Lead 自身文档的错误**，已改正。

**三项 Lead 文档错误：**

1. **§2.1 生成管线接线**：`FeatureControlOptions` 只设 `compileOptionCategories`
   （`Options.td:59-65`），`passGroup` 按选项声明；`ConfigsToOptionsConversionGen.cpp:80-82`
   对空组 `continue` ∴ **`ConfigUtils.cpp.inc` 不会自动生成该赋值**。§2.1 该行改为 **NOT generated**；
   驱动侧 `getInputIRStage()` 已足够，仅在确有 Pass 需要该值时才加 `passGroup`。
2. **§3.2 `inferLayoutOptimization` 的表述**：实为普通 `module.walk`
   （`regbase/Utility.cpp:100-120`），只在遇到 `linalg.batch_matmul` / `triton_print` /
   `scope.scope` 时置 `setEnableLayoutOptimization(false)`，**不添加任何 Pass**，且**无条件被调用**。
   ∴ "该函数零执行"**不是可测命题**；正确命题是"它可能置位的标志没有已调度的消费者"。已改写
   （`inferMixedCV` 同理，`Utility.cpp:95-97`）。
3. **§3.3 "任何 ninja 目标都会重链 opt" 过宽**：两者是**独立链接边**，共享库只是共同输入。

**两项 Challenger 的已执行实验（记为**其**证据，非 Verifier 验收证据）：**

- **case-2 跳过**：容器内 pin 住的 `bishengir-opt` `0761c6e6…` + 冻结输入 `d1023ac1…`，parse/print
  **带与不带** `--hacc-append-device-spec=target=Ascend950PR_9579` 均退出 **0**，规范 IR
  **逐字节相同**（SHA `884c947d25f491184adf975deeca60fd8afbd3808107e624e1f04a1d39a980e8`）。
  **完整尾部与二进制比较仍为 NOT_RUN**；附**核心数覆盖**告警（自定义 core-count 会改变 spec 条目，
  复用前须按预期默认值校验或拒绝该覆盖）。
- **Ninja 陈旧反例**：容器 scratch 中构建含 P2 字节的输出，用 P1 字节覆盖后 `ninja -n` 报
  **"no work to do"**——**已复现**。∴ §3.3 步骤 4 改为**带护栏的恢复**，四条护栏缺一不可：
  **G1** 每次暂存前显式重建 **opt 与 compile 两个**目标；**G2** 使恢复文件**早于其依赖**的 mtime
  （或另一经证明的失效手段）；**G3** 校验暂存包哈希**且**校验暂存 `bishengir-opt` **已注册新 Pass**；
  **G4** 冻结完整 `bin/`+`lib/` 暂存包。无法执行 G1–G4 时，回退**独立构建目录**（更干净，但
  **技术上非必需**）。P2 校验器/负例/D012 测试**必须**用**新建并暂存**的 opt。

**两项 Lead 复核超出 Challenger 结论（已请其反查）：**

- **Mix/Aiv 过滤不作用于 P1 Late 输入**——`PassExecutionPolicy.h:42-46` 仅在函数带
  `mix_mode = "aiv"` 时解析为 Aiv 模式，否则保持默认 full(Mix)。**实测冻结产物 `vadd_late.mlir`
  中 `mix_mode` 出现 0 次** ∴ 该 denylist 对本输入从不被查询。不把它留作"恰好成立"的性质，而
  升级为 **Late 不变量：V1 拒绝携带 `mix_mode` 或 `FilterPassesAttr` 的 Late 输入**。
- **`Pass.cpp:516-527` 的取舍不对称性**：`runBeforePass` 在 `:516-518`，`runOnOperation` 在
  `executeAction` 内的 `:521-527` ∴ **缺席**（在覆盖面成立下）证明**未调度**——正是 §3.2 负例所需，
  可靠；**出现**只证明**已调度**，不证明执行。∴ §3.1 "某 Pass 起了作用"一类断言不能靠追踪，须靠
  **产物 IR 效果**（如 `--convert-scf-to-cf` 后存在 `cf` 块、转换后无残留 `hivm.hir.*`），或
  在 `execute`/`runOnOperation` 边界计数器。

**Challenger 探针披露（已接受）**：其第 1 项语法探针测试的是**旧**声明
`llvm::cl::opt<string,true>` 而非改正后的 `std::string`；仅用 `clang++ -fsyntax-only` 在 `/tmp`
写入临时源文件，**未产生目标文件/可执行文件，未改动任何生产/测试/Golden 文件**。∴
**"P2 生产代码 0 行"的陈述依然准确**。

**仍开放**：Challenger 按其自列条件清单的**再评审 verdict**。Lead 已就 §2.2（无条件 Late 探测）、
§2.3.1（选项裁定表）、§3.3（入口侧 C310 硬失败）主动请其指出仍欠规定之处。`verify` 的测试冻结
仍在进行（约 33 分钟）。**步骤 4 继续等待**。

### P2 架构评审第三轮 — 收编完成（Lead，2026-10-13）

**阶段**：P2 步骤 3（架构评审）收尾；③ 轮的**全部**修订项已落入 `docs/p2-architecture.md`，
**仍等 `verify` 的测试冻结与 `review-2` 的再评审 verdict**。

**本轮新落入的六处修订（逐条先本地源码复核，再改写）：**

1. **§3.2 前置条件 4：端点 ≠ 覆盖面。** 两个哨兵 Pass（`--annotation-lowering` 与
   `--convert-hivm-to-llvm`）只证明追踪**两端**可达；覆盖面须**按 PassManager 逐个**建立 —
   枚举本次运行创建的每个 manager（regbase 尾部的、`RetriablePassManager` 重试包装的、A5 进程内的），
   并证明每一个都出现在追踪里。某个 manager 若在**未注册这些选项**的情况下被构造，追踪看起来完整，
   却**静默漏掉**该 manager 的一切。
2. **§3.2 新增"边界日志"表 —— 一行 Late 路由日志不够。** 单个 `[INFO] … Late …` 只证明**分支被走**，
   对"普通流水线 / 两个 infer 函数 / 重试循环此后是否运行"一无所知（不打印的函数在日志里不可见）。
   要求在**真实边界**放置**小日志**（非新框架）：普通流水线入口（`bishengir-compile.cpp:188`
   之前一行）、`inferLayoutOptimization`/`inferMixedCV` 顶部各一行（`regbase/Utility.cpp:100/:95`）、
   重试循环每轮一行（`regbase/BiShengIRCompileMain.cpp:345-465`，`tryTimes = 2` 于 `:376`）、
   `runHIVMCCompileA5`（`:448`）一行。判据是**缺席与计数**：日志须无普通流水线行、无重试行、
   且 A5 行**恰好一条**。
3. **§3.1 新增：步骤 5 必须在任何重试循环之外，而普通路线不是这么写的。** 实测普通路线的
   `runHIVMCCompileA5` 位于 `regbase/BiShengIRCompileMain.cpp:448`，**在重试循环**
   （`:345-465`，`:376` `tryTimes = 2`，`:393-455` clone/retry）**之内**。Late 路线**不得继承**
   该结构：`runLateHIVMPipeline` 在**唯一一次** `buildLowerToLLVMPipeline` 之后**只调一次**
   `runHIVMCCompileA5`，无重试包裹。§3.2 的 A5 计数正是区分这两种**结构不同**形态的实测手段。
4. **§3.1 新增：bitcode 经 A5 路径而非本尾部到达链接器。** `HIVMCMainA5.cpp` 以
   `-cce-link-aicore-ll-module`（`:181-188`）传入已链接模块；步骤 4 写入的 attach 期属性正是该路径
   读取的对象 —— 这就是 §3.3 硬校验所保护的东西。另补：`HIVMSyncSolverVersion`
   （`Options.td:210-215`）是 `"std::string"` 的**又一处**先例（§2.1）。
5. **§3.3 G2 补记反例的精确机制。** 临时 `build.ninja`：`rule copy`（`command = cp input output`）、
   `build output: copy input`；`input` 含 P2 字节；`ninja output` 构建之；`shutil.copyfile` 以保留的
   P1 字节覆盖 `output`；`ninja -n output` 报 **"no work to do"** 而 `output` 仍是 P1 字节。
   宿主机探针另证：使 `output` 的 mtime **比 `input` 早 10 秒**会让 `ninja -n` 重新排期。
   **两次探针均在隔离临时目录、未改动任何真实构建产物** —— 记为 **Challenger 的已执行证据**，
   **非**任何验收行。
6. **Challenger 自撰报告已单独披露。** 同一评审轮中 `review-2` **自行撰写**
   `review/challenger-p2-entry-design.md`（20,070 B）。这是**评审者自己的产物**，非 Lead 记录；
   已在 `review/challenger-entry-design-verdict.md` 的"残余项"中**单独披露**，理由：本阶段常设陈述
   "P2 生产代码 0 行"必须保持准确 —— 该报告是 `evidence/review/` 下的**评审文档**，
   **不含任何生产/测试/oracle/Golden 文件改动**，不改动证据树之外的任何仓库。内容（verdict REVISE、
   22 行"已核对的 HEAD 断言"表、E1–E7、NOT_RUN 清单、给 Lead 的决策记录）已在本文档摘要并经
   Lead 复核后并入架构文档。

**未变**：P2 生产代码仍为 **0 行**；两个仓库仍停在 D031 头（`AscendNPU-IR-1` `80e29e6f2`、
`tilelang` `cc56643d`）。**步骤 4 仍被 `verify` 与 `review-2` 阻塞。**

### P2 Lead 源码复核新增发现 — 尾部并非无参，且 20-Pass 计数依赖选项拒绝（Lead，2026-10-13）

**阶段**：P2 步骤 3 期间，Lead 直接通读 `regbase/PassPipeline.cpp:213-294`（而非只看 `--help`
测得的 Pass 名表），得到三处**架构相关**的新事实，已写入 `docs/p2-architecture.md` §2.3.1 与 §3.1。

1. **`buildBiShengHIRAVEToLLVMPipeline`（`:213-229`）有三道门，Late 只依赖其中一道。**
   `getCompileHost()`（`:215-218`）**提前 return**，会把整条尾部换成 HACC host 降级 —— §2.3 拒绝
   `--compile-host` 正是 Late 直接调用得以安全的前提；`getEnableHIVMCompile()`（`:220-225`）只
   门控 Late **要跳过**的 AVE 分支 ∴ 该标志对 Late **无关**，entry **不得**去强制它；
   `getLowerToLLVM()`（`:226-228`）是 Late 依赖的门控。**实测**：`LowerToLLVM` 是
   `BiShengIRCompileInternalStateOpt`（`Options.td:1557-1563`，文件内注释 `:1548-1550` 明言
   "仍是真实的 option/config 条目 …… 目前不作为常规生成的 CLI/pass option 暴露"），默认
   **`true`**，且**全树无任何 `setLowerToLLVM` 调用点**（对 `*.cpp`/`*.h`/`*.td` grep）。∴ 本构建上
   尾部总会运行，Late 的直接调用与常规路线**一致**而非特例；但 Late entry 仍**断言**该值而非假设
   —— 默认值不是契约。
2. **尾部不是无参的：两个 config 值会改变转换本身，第三个会静默覆盖第二个。**
   `regbase/PassPipeline.cpp:253-259`：
   ```cpp
   ConvertHIVMToStandardOptions hivmToStdOptions;
   hivmToStdOptions.isOpsAligned      = config.getEnableHIVMAutoStorageAlign();
   hivmToStdOptions.markLibCallNoInline = config.getEnableLibCallNoInline();
   if (config.getEnableSanitizer() || config.getEnableDebugInfo())
     hivmToStdOptions.markLibCallNoInline = false;      // 尾部内静默覆盖
   ```
   ∴ `--enable-hivm-auto-storage-align` 与 `--enable-lib-call-no-inline` 进入 §2.3.1 裁定表
   （**拒绝或钉死**，不得静默继承）；`--enable-sanitizer`/`--enable-debug-info` 的**静默覆盖**是
   第三重耦合，已在 §2.3 拒绝表中。
3. **"恰好 20 个 Pass"的断言依赖 §2.3 的选项拒绝，是源码**在 Late 选项集下**的性质。**
   `buildLowerToLLVMPipeline` 内含一个条件子块（`:269-289`：`AdaptGPUKernel`、
   `HoistCallScalarToCaller`，以及条件性的 `DPXDivOptimization`、`SIMTInsertInitAndFinishForDebug`），
   由 `if (config.getPureSimt() && config.getUseDPX())` 守卫。这四个 Pass **不在** §3.1 的 20 行表中，
   正因为 Late 模式拒绝 `--pure-simt`。P2-B 的追踪正是"该守卫为假"的实测证据。

**已修正**：§2.4 的 V1 行原先把 `cf` 列入白名单，与同节后文"`cf` **刻意排除**（`ConvertSCFToCF`
在尾部更晚才运行）"**自相矛盾** —— 已改为显式排除。另把 `mix_mode`/`FilterPassesAttr` 的拒绝写进
V1 行（原仅见于 §3.2）。

**未变**：P2 生产代码 **0 行**；两仓库仍在 D031 头。

### P2 Lead 复核 — §7(C) "D012 用例作为仓库内 MLIR 回归测试" 的落点已确证（Lead，2026-10-13）

**问题**：目标 §7(C) 要求 D012 用例成为**仓库内**的 MLIR 回归测试，而 Verifier 目前写的
`tests/negatives/*.mlir` 位于**证据树**（`hivm-poc-evidence/`），二者不是一回事；实现者需要知道
仓库内该放哪里、用什么形式。

**实测结论**（`AscendNPU-IR-1/bishengir/test`，共 **1119** 个 `.mlir`，lit + FileCheck）：

1. **校验器（§2.4 V1–V7）的拒绝诊断** → 走 `bishengir-opt <pass> %s -split-input-file
   -verify-diagnostics | FileCheck %s`，落在 `test/regbase/`（该目录已存在，含
   `regbase/Dialect/…`、`regbase/Conversion/…`；现有 RUN 行范式见
   `regbase/Dialect/HFusion/fold-unit-dims-subview-cast.mlir`、
   `regbase/Conversion/LinalgToHFusion/linalg-to-hfusion.mlir`）。这正是 §2.4"独立可测的注册 Pass"
   所换来的可测性 —— 负例用 `-verify-diagnostics` 断言诊断，而不是靠驱动退出码。
2. **驱动层的选项互斥/路由诊断（§2.2、§2.3、§2.3.1）** → **已有现成归属**：
   `test/bishengir-compile/commandline.mlir` 的既有范式恰是
   `not bishengir-compile %s --<opt>=… 2>&1 | FileCheck %s --check-prefix=<NAME>`
   （已用于 `--multibuffer-mode` 的 `ZERO`/`INCOMPLETE`/`TOO-LARGE`/`DUPLICATE`/`UNKNOWN`/`NEGATIVE`
   等前后缀）。Late 模式的 `--enable-triton-kernel-compile` / `--pure-simt` / `--compile-host` /
   `--link-aicore-bitcode` / `--append-bisheng-options` / `--enable-tuning-mode` /
   `--enable-auto-multibuffer` / `--enable-optimized-metaop` / `BISHENGIR_LEGACY_A5_REGBASE` 拒绝，
   以及"Late 无 target"与"非 Ascend950 target"两条诊断，**直接加进该文件或同名同级新文件**即可，
   无需新造测试框架。
3. **仓库内目前不存在任何使用 `l2_cache_mode` 的测试**（对 `bishengir/test` 全树 grep：0 命中）
   —— 即 D012 的这些用例**确实尚未落库**，§7(C) 是一项真实待做的实现工作，不是"已有测试的复跑"。

**归属**：这三类仓库内测试是实现者（implementor）的生产代码改动；Verifier 的证据树固件是**独立**
的验收材料。二者都要有，且不得互相替代 —— 这一点已在 `docs/p2-architecture.md` §5/§7 的口径下
保持（Verifier 的期望必须独立于生产实现冻结）。

**未变**：P2 生产代码 **0 行**（尚未开始实现）；两仓库仍在 D031 头。

### P2 §9 交付文档建立 + 模型身份证据锚定（Lead，2026-10-13）

**阶段**：P2 步骤 3 等待期。Lead 新建三份 §9 要求的验收记录文档（全部**只含 NOT_RUN 行与判据**，
不含任何预制结果），并把"模型身份"从断言改为**可追溯证据**。

**新建文档（`docs/`）：**

- **`p2-pass-trace.md`** —— §3.2 各条"零执行"断言的记录表：路由(A1–A5)、禁止项(B1–B7，含新发现的
  `getPureSimt() && getUseDPX()` 子块四个 Pass)、必需项与顺序/计数(C1–C4)、契约保全(D1–D11，
  D11 端子屏障**按入口作用域**)、以及追踪覆盖面五条前置条件(E1–E5)。文首明确：**覆盖面不成立的
  零读数一律 NOT_RUN，不是 PASS**；**出现**只证明已调度、不证明执行，故"某 Pass 起了作用"须另加
  产物 IR 效果或执行边界计数器。
- **`p2-binary-abi.md`** —— 构建身份(A1–A6)、C310 bitcode 身份与硬失败(B1–B8)、**目标 §6.3 的
  11 条 LLVM/Kernel ABI 检查逐条列出(C1–C11)**、最终产物真实性(D1–D7，并列出 P3 ACL harness 需要
  交接的项)。含 §6.2 的 `l2_cache_mode → zext → shl 60 → descriptor → MOV.UB.TO.OUT.ALIGN.V2.DV`
  链条，以及"不得只凭独立模板 dataflow 正确就宣称最终 Kernel 正确"。
- **`p2-verification.md`** —— 独立性(0.1–0.5)、A 入口/Pipeline(A1–A11)、**B 语义反例(B1–B11)**、
  C 编译器与旧路径回归(C1–C6)、D 二进制真实性(D1–D3)、E Challenger 审查(E1–E5)、F 模型身份。
  两条铁律写在文首：**测试必须因预期的语义断言失败**（不得因 SHA 不匹配或 MLIR 解析失败而"通过"）；
  **未执行即 NOT_RUN**。

**模型身份：证据已锚定，"实际服务模型"结论恢复到**有据**状态。** 过程记实：本文件初稿直接断言
served model（无出处）→ 我一度过度纠正为 NOT_CONFIRMED → 最终在**本会话各 agent 自己的 transcript
记录**里找到 gateway 回显的 `model` 字段，证据如下（路径
`~/.claude/projects/-home-s00653124-workspace-hivm-poc-control/ea69c136-c295-41a6-96a9-140265430a03/`）：

| 角色 | agent type 标签 | **实际 served model** | 证据 |
|---|---|---|---|
| Lead | team-lead | `deepseek-v4.1-flash` | 主 transcript，4173 条 |
| implementor | `implementor-sonnet` | `deepseek-v4.1-flash` | `agent-aimplementor-619b17ef9c142eac.jsonl` 381× 等 |
| verifier | `verifier-haiku` | `mimo-v2.6-flash` | `agent-averify3-c52add0e1050031b.jsonl` 800× 等，**当前** `agent-averify-fd1c402be6c7e059.jsonl` 169× |
| challenger | `challenger-opus` | `gpt-6.1-sol` | `agent-achallenger-00ae991f5d9687bd.jsonl` 266× 等，**当前** `agent-areview-2-744b9123f07b8871.jsonl` 224× |

**结论**：**本会话没有任何 agent 由 Claude 模型服务**；且目标 §3 所设想的
**GPT-6.1 Sol / Claude Haiku 5.5 / Claude Opus 5.5 分工在三个槽位上全部不一致** —— 本应由 GPT-6.1 Sol
承担的 Lead 实际是 `deepseek-v4.1-flash`，而 `gpt-6.1-sol` 是 **challenger** 的模型。按要求在架构评审
结束前**如实报告**。

**同时否证了一个错误证据源**：`refusal-capture/hivm-p0-20261012T124105Z.E3UJQ9/magpie-requests-at-exit.json`
是**上一会话**的**网关级**日志，且其 `model` 字段是混合集合（`gpt-6.1-sol` 266、`deepseek-v4.1-flash`
125、`step-5-preview-free` 87、`claude-haiku-5-5` 15、`claude-opus-5-5` 5、`mimo-v2.6-flash-free` 2），
与该会话自身配置不符；其自带团队记录写明 `"served_models": "NOT_VERIFIED"`。∴ **不得**用它给某个
agent 归属模型。

**未变**：P2 生产代码 **0 行**；两仓库仍在 D031 头；步骤 4 仍被 `verify` 与 `review-2` 阻塞。

### P2 Lead 复核 — `runRegBaseCompile` 中间那一步是死存储（Lead，2026-10-13）

**动机**：§2.2 要求 Late 分支在 `runRegBaseCompile` **首句**返回，早于 `inferLayoutOptimization`/
`inferMixedCV`。但实测 `regbase/Driver.cpp:63-88` 在两次 infer 与 `runRegBasePipeline` **之间**还有一步：

```cpp
config.setClArgs(filterRegBaseForwardedHIVMCOptions(originalCLArgs));   // Driver.cpp:79
if (failed(runRegBasePipeline(module, config))) { ... }                  // :81
```

`filterRegBaseForwardedHIVMCOptions`（`regbase/Utility.cpp:124-149`）把所有以 `-` 开头的 argv 原样转发，
只剔除 `-o/--output`（连同其值）、`--hivmc-args`、以及任何含 `enable-lir-compile` 的参数 —— 看起来像是
"CLI 选项到达下游工具链的通道"。**它不是。**

**实测**：`BiShengIRCompileMainConfig::clArgsFlag` 在 `Driver.cpp:79` 被写入，**在全树中无人读取** ——
树里唯一的两个 `getClArgs()` 是两个**不相关**的访问器（`include/bishengir/Tools/bishengir-compile/
Config.h:86` 属主配置；`include/bishengir/Tools/hivmc/Config.h:746` 属**另一个类**，其字段由
`readCLArgs` 自行填充），且 `HIVMCFromBiShengIRConfig` **从不**调用 `hivmcConfig.clArgs(...)`。
∴ Late 分支在 `:79` 之前返回**不会**丢掉普通路线本来会送达的任何选项。

**为什么仍要写进架构文档**：这个失效模式**天生静默** —— 仅看调用点无法区分死存储与活存储，
一旦驱动改动就会变成真丢选项。已在 §2.2 记录，并注明"驱动若变动须复核"。

**未变**：P2 生产代码 **0 行**；两仓库仍在 D031 头。

### P2 Lead 已执行探针 — 拒绝策略里每个选项名逐一验证存在性（Lead，2026-10-13）

**阶段**：P2 步骤 3。**动机**：§2.3/§2.3.1 会写成"拒绝 `--xxx`"的守卫；若名字拼错，或该选项根本
不是 CLI 选项，守卫就是空的，且负例测试会**因错误的原因通过**（未知选项报错冒充"Late 拒绝"）——
正是目标 §7 B 明令禁止的那种假 PASS。

**方法（容器内，因为该二进制在宿主机跑不起来）**：
`bash scripts/in-container.sh bash -c '$B --help > /tmp/h.txt; grep -- "--<name>" /tmp/h.txt'`，
`$B = /home/s00653124/workspace/AscendNPU-IR-1/build/bin/bishengir-compile`（SHA `c6838fc0…`，
与保留副本一致）。完整输出已存：
`hivm-poc-evidence/p2-20261013-late-pipeline/lead/option-existence-probe.txt`（`--help` 退出 0，543 行）。

**结果修正了三处（其中一处是真问题）：**

| 我此前的写法 | 实测 |
|---|---|
| `--multi-kernel` | **不存在**；真名 **`--enable-multi-kernel`** |
| `--enable-auto-multibuffer` | **不存在**；真名 **`--enable-auto-multi-buffer`** |
| `--compile-host` | **不存在，且不是拼写问题**：`$B --compile-host=1` 退出 0 并报 `Unknown command line argument '--compile-host=1'`。`CompileHost` 是 `BiShengIRCompileInternalStateOpt`（`Options.td:1551-1555`），文件内注释明言**不作为常规 CLI/pass option 暴露**；config 字段在（默认 `false`），CLI 旗标**没有** |

**∴ 第 3 项是测试设计陷阱**：写成 `not bishengir-compile --compile-host …` 会"通过"，但通过的原因是
**未知选项被拒**，而不是 Late 模式拒绝了 host 编译。故 `CompileHost` 守卫必须是**config 状态断言**
（`config.getCompileHost()` 为假，且 Late 入口永不走到 `buildLowerHACCToLLVMPipeline` 的提前 return），
其测试须按此写。**同一条谨慎适用于 §2.3.1 的每一行**：拒绝测试必须证明**指名的那条诊断**，
不得以通用 unknown-option 错误充当。

**其余全部 PRESENT**（17 项，含 `--enable-direct-hivm-lowering` —— 它**存在**为 CLI 选项，只是
在 regbase 路径上**无效**（D018/D10），这是"无效"而非"不存在"，两者不可混同）；
`--input-ir-stage` **ABSENT** ✓（确认 D12 无命名冲突，留给我们新增）。

**未变**：P2 生产代码 **0 行**；两仓库仍在 D031 头；步骤 4 仍等 `verify` 与 `review-2`。

### P2 Lead 单位核查 — `UB_SIZE` 2031616 是 **bits**，与 kernel 的 196608 **bytes** 是两个量（Lead，2026-10-13）

目标 §4.2 明令"不得把 `UB_SIZE=2031616 bits` 错当成 Kernel 的 `dyn_shared_memory_buf=196608 bytes`"。
本项在树内找到**直接证据**，不必依赖目标文档的措辞：

- `include/bishengir/Dialect/HACC/Targets/NPUTargetSpec.td:224`：
  `let UbSize = 2031616; // bits = 248KB, reserve 8KB for compiler` —— 源码注释直接写 bits。
- PlanMemory 自己的诊断也以 bits 计：
  `bishengir/test/regbase/Dialect/HIVM/RegBase/plan-memory-vf.mlir:453`
  `expected-error@below {{ub overflow, requires 2097152 bits while 2031616 bits available!}}`。
- 交叉印证：RegBase 模板 `lib/Template/lib/RegBase/Cube/include/ascendc_bisheng/utils/kernel_utils_constants.h:86`
  `const uint32_t TOTAL_UB_SIZE = 248 * 1024;`（= 253952 **bytes**）—— 恰等于 2031616/8。

**∴ 2031616 bits = 253952 B = 248 KB 容量；196608 **bytes** 是 Golden 的动态 arena。** 另有**数字陷阱**：
`HACCAttrs.td:309` 的文档示例写作 `#dlti.dl_entry<"UB_SIZE", 196608 : i32>` —— 与本案 arena 的数字**完全相同**。
∴ 任何"以数字相等来确认"的检查都会在错误的量上"通过"。C9 已按此改写（`docs/p2-binary-abi.md`）。

**未变**：P2 生产代码 **0 行**；两仓库仍在 D031 头；步骤 4 仍等 `verify` 与 `review-2`。

### P2 Lead 界定 C9 的断言边界 —— `dyn_shared_memory_buf` 在编译器树内**没有消费者**（Lead，2026-10-13）

**阶段**：P2 步骤 3。目标 §6.3 第 9 条说"dynamic UB=196608 作为启动契约保留，不误写为设备容量"。若不把
它的**断言边界**写清，这一行会被读成"要求 AscendNPU-IR 消费它"—— 而它并不消费。

**实测**：`grep -rn 'dyn_shared_memory_buf' AscendNPU-IR-1/bishengir`（`*.cpp`/`*.h`/`*.td`/`*.py`）
**零命中**。它在 TileLang/TVM 一侧：写于 `tilelang/src/transform/lower_device_kernel_launch.cc:305`，
读于 `tilelang/tilelang/jit/adapter/wrapper.py:488-489`，运行时由
`tilelang/src/ascend/runtime/ascend_module.cc:482-489` 消费（设 `kAclLaunchKernelAttrDynUbufSize`）。

**∴ P2 内 C9 只能断言两件事，且都在发出的产物上**：(i) 入口函数上该 func attribute 仍在且值为
`196608`；(ii) 设备规格里的 `UB_SIZE` 仍是 `2031616`（bits）。启动契约的消费属 TileLang runtime + P3；
**P2 若去编译器里找消费者，会一无所获，且不得把这种"找不到"读成失败**。冻结产物自身也记了这一点，含
它对早期错误说法的自纠（`vadd_late.mlir:38-46`：早前称 arena "has no carrier in the Late IR"，"That was
WRONG —— 它把'在 AscendNPU-IR 中没有消费者'（真）与'没有 IR 载体'（假）混为一谈"）。

**已落文档**：`docs/p2-binary-abi.md` §C（C9 边界段）。

**未变**：P2 生产代码 **0 行**；两仓库仍在 D031 头；步骤 4 仍等 `verify` 与 `review-2`。

### P2 Lead 预冻结 B5/C4 的期望值 —— 从模板源定义，先于任何实现（Lead，2026-10-13）

**阶段**：P2 步骤 3。**动机**：CLAUDE.md 要求"期望结果独立于生产实现、在实现之前定义"。目标 §6.2 的
"第四个 i32 Store libcall 参数与模板 ABI 一致"与 §7 C 的"非 C310 路径保留旧 ABI"此前只有措辞、没有
**数值化的期望**，等到实现后再去读 IR 就有"照着输出写期望"的风险。现从**模板源**（ABI 的定义处）取出：

`lib/Template/lib/RegBase/Vector/include/RegBase/DMAUtils.h`（D012 提交改动的文件之一）：

| | C310 臂（`:268-294`，`#if defined(__DAV_C310__)`） | 非 C310 / m300 臂（`:295-319`） |
|---|---|---|
| core 函数 | `store_ubuf_to_gm_intrin_core(ub_ptr, ub_offset, gm_ptr, gm_offset, burst_cnt, burst_len, src_gap, dst_gap, uint32_t l2_cache_mode)` —— **9 参数** | 同上去掉 `l2_cache_mode` —— **8 参数** |
| 发出的 intrinsic | **`copy_ubuf_to_gm_align_v2`**（`:284`、`:288`）—— 注意 `_v2`，与 P1 的 `MOV.UB.TO.OUT.ALIGN.V2.DV` 对应 | **`copy_ubuf_to_gm_align`**（`:308`、`:312`）—— **无** `_v2` |
| 位置参数 | `(gm_ptr+off, ub_ptr+off, 0, burst_cnt, burst_len, l2_cache_ctl, dst_stride, src_stride)` | `(gm_ptr+off, ub_ptr+off, 0, burst_cnt, burst_len, src_align_gap, dst_gap)` |
| 策略 | `uint8_t l2_cache_ctl = static_cast<uint8_t>(l2_cache_mode);`，注释 `:278-279`：策略在**整条 public/forwarding ABI 上都是 32 位**，只在描述符字段宽度处收窄 | 无 |

**∴ 目标 §6.2 的"第四个 i32 参数"落在 `l2_cache_ctl` 上**：在
`(ptr, ptr, i32=0, i32=burst_cnt, i32=burst_len, i32=l2_cache_ctl, …)` 中它是第 4 个 i32 类型参数。
**两条告诫**（均须在**真实发出的 LLVM IR** 上裁决，不得以本表代替）：(i) 两臂**不是**"同一次调用多一个
参数"——第 7/8 个参数的**名字与取值都不同**（`dst_stride, src_stride` vs `src_align_gap, dst_gap`），
只数参数个数的 diff 会在错误调用上通过；(ii) `l2_cache_ctl` 同时是冻结产物陈旧注释中所说的"可丢弃
op 属性"的拼写 —— 二者同名不同义（`p2-architecture.md` §2.4）。

**已落文档**：`docs/p2-binary-abi.md` §B（B5 期望值块 + C4 期望值），B5/C4 行加指针。

**附注（记录一致性）**：D031 行（`docs/decisions.md:73`）称"`hivm-poc-control` 与 `hivm-poc-evidence`
**不是** git 仓库，故 `docs/` 与证据树不受版本管理"。**该句已过时**：`hivm-poc-control` 现为 git 仓库，
`master` 跟踪 `origin/master`，已有提交 `d8a5d2b feat: p1`；本轮 P2 文档即以新提交记录在此。D031 的
其余结论不受影响，此处按"不改写既有裁决行、只追加事实"的惯例记录，而不回改 D031 原文。

**未变**：P2 生产代码 **0 行**；两仓库仍在 D031 头；步骤 4 仍等 `verify` 与 `review-2`。

### P2 Lead 溯源核查 — 冻结产物的注释矛盾属既有记录；构建树的两处模板身份风险（Lead，2026-10-13）

**阶段**：P2 步骤 3（`verify` 仍在跑、`review-2` 已被要求给出第 4 轮裁决）。Lead 在等待期间做源级
溯源核查，全部结论来自本地实测，未改动任何生产/测试/Golden 文件。

**一、`vadd_late.mlir` 的注释与算子矛盾 —— 不是新发现，已属既有记录，不得据此改冻结产物。**
冻结产物头部（`:25`、`:27`）与行内注（`:184-188`）称该 store "carries NO cache mode"、pre-D012 拼写
`{l2_cache_ctl = 4 : i64}` "PARSES BUT IS SILENTLY DISCARDED"、"the emitted libcall receives 0, not 4"；
而 `:189` 的算子实际带着 `{l2_cache_mode = 4 : i32}` —— 即 `StoreOp` **已识别**的属性
（`HIVMDMAOps.td:181,198`，合法 store 值恰为 {0,1,2,4}；`HIVMDMAOps.cpp:391-413`）。
该矛盾**早在 D025 轮已记录**：`challenger-d025-zqumt69b/entry-evidence/p1v2/scoped-vf/INTEGRITY.json`
的 `stale_header_comment` 字段写明"written BEFORE the D012 fix … That prose is STALE. It is deliberately
NOT edited, because editing it would change the reviewed artifact identity (`d1023ac1…`, APPROVED by the
independent Challenger)"。∴ 结论是**"读解析后的算子，不读注释"**，产物**保持不可变、不得编辑**。
同一份记录还写明冻结 P1 检查器的 `B8.TERMINAL`（`p1_check.py:1561-1576`）**没有 FAIL 分支**（只记录、
不断言，删掉末端 `PIPE_ALL` 零个判定改变）—— 故 P2 的终端屏障行必须是**我们自己的真断言**，不能继承
P1 oracle 的行为。

**二、HEAD 溯源**：`AscendNPU-IR-1` HEAD `80e29e6f27…` 恰好**只比** `e878f3492`（冻结 P1 产物的
创作 HEAD）**高一个提交**，而这一个提交**就是 D012 变更集** ——
`[HIVM] fix: carry the store L2 cache policy through to the DMA descriptor`（2026-10-13 19:47 +0800，
11 个文件：store 的 `l2_cache_mode`、`verifyStoreCacheMode`(+155)、`StoreOp::verify`、C310 模板源
`RegBase/Vector/{Copy1D,Copy2D,Copy3D}.cpp` 与 `RegBase/Vector/include/RegBase/DMAUtils.h`、2 个测试）。
实测：字符串 `invalid store l2_cache_mode` 在 HEAD 存在，在 `e878f3492` **全树不存在**。∴ store 侧
cache 策略正是 P2 所构建的那一个提交引入的。

**三、树里有两个 C310 模板，其一早于 D012 —— 这是 §6.2/B7"不得用旧 bitcode/回退模板"的现成陷阱。**
`build/lib/meta_op.aiv.c310.O2.bc` = `5e7ccae2…`（31,206,796 B，13:46）；
`build/install/lib/meta_op.aiv.c310.O2.bc` = `44e4387b…`（31,179,496 B，**2026-10-09** 15:01）。
`getLibDirFromExecutable` 按 `<real exe>/../lib` 解析且 `real_path` **会解符号链接**；而
`build/install/bin/` 自带一份 10-09 的 `bishengir-compile`/`bishengir-opt`/`hivmc`。∴ 若从
`build/install/{bin,lib}` 暂存，`../lib` 会静默落到 **pre-D012** 模板，而外观完全合法。

**四、被 pin 的那份模板本身早于committed模板源。** `RegBase/Vector/include/RegBase/DMAUtils.h`
写于 **14:42**，该目录在 ccec 包含路径上（`Copy1D.aiv.c310.O2.bc.ccec.args` 逐字记录
`-I …/RegBase/Vector/include`），而逐文件模板 bitcode 产于 **13:37:55**、合并件产于 **13:46:45**，
**14:00 之后没有任何 `RegBase/Vector/*.bc` 被重建**；`DMAUtils.h` 正是 D012 提交改动的文件之一
（79 行变更）。14:42 那次写入是"改了内容"还是"原样重写"，**文件系统无法判定**；`ninja -n` 也无法判定
（它停在 "Re-running CMake"，因为 `build.ninja` 本身就过期）。**∴ P2-C 必须从 P2 树重建模板并把自己
的 SHA-256 记为身份，不得假定 `5e7ccae2…` 对应 committed 模板源** —— 否则目标 §6.2"证明新 kernel 的
链路消费了正确 bitcode"在模板未验证的前提下不成立。

**已落文档**：`docs/p2-architecture.md` 头部（提交身份与 D012 内容）、§2.4（V7 读解析算子 + 注释矛盾
已记录 + D11 不得继承 P1 oracle）、§3.3（两个模板 + 重建要求）；`docs/p2-pass-trace.md` D7、D11；
`docs/p2-binary-abi.md` §B 前置条件。**注意**：以上均为控制仓文档，**不是生产代码**。

**未变**：P2 生产代码 **0 行**；两仓库仍在 D031 头；步骤 4 仍等 `verify` 与 `review-2`。

---

## P2 架构门（§8 step 3）Challenger 第 4 轮 REVISE —— 四项修正已逐条独立复核并落入架构文档

**阶段**：P2-A 架构门（goal §8 step 3）。**仍未写任何 P2 生产代码**（P2 生产代码 0 行）。

**裁决**：`review-2`（agent type `challenger-opus`，**实际 served model `gpt-6.1-sol`**，取自其自身
transcript 的 gateway `model` 字段，非由 agent-type 推断）第 4 轮裁决 **REVISE —— 设计范围**。
Challenger 明确写出：Verifier 的测试冻结未完成**不**阻塞本裁决，**修正所列各点后即可给出 scoped
entry-design APPROVE**。裁决原文（三条消息）已由 Lead 逐字留存于
`p2-20261013-late-pipeline/review/challenger-entry-design-verdict-round4.md`（Challenger 本轮**未**自写
文件，故由 Lead 按既定规则代为逐字持久化；来源 transcript 记录 `rec394`/`rec420`/`rec426`）。

**四项修正 —— Lead 均先回到本地源码独立复核，再改文档，未采信任何一条**：

1. **冻结选项处置**。`--enable-static-bare-ptr=false` 显式出现即**拒绝**（该 flag 在本路径上经
   `cl::location` 写入且 transfer helper 从不传递，故为静默无效；`emitGetterSetter = 0`，
   `hivmc/Config.h:830` 默认 `true`）；`--enable-optimized-metaop` **显式 pin 为 true（O2）**
   （`Options.td:818-826` 默认即 true）；`--enable-hivm-auto-storage-align` **pin true**
   （`Options.td:662-666`）；`--enable-lib-call-no-inline` **pin true**。**关键发现**：TableGen 默认值
   **不是** regbase 路线上的有效默认值 —— `applyArchDependentCompileDefaults`
   (`BiShengIRCompileConfig.cpp:374-383`，`:463`/`:481`/`:486` 调用) 在 `isRegBasedArch` 时、且仅当用户
   **未显式传参**（`hasExplicitCLOption`）时，把 `enable-lib-call-no-inline`、`enable-hivm-unit-flag-sync`、
   `enable-preload` 三者设为 **true**，而三者在 `Options.td` 中都写着 `"false"`；`--enable-lib-call-no-inline`
   的自身描述也写着 *"Enabled by default on RegBase and disabled by default on membase."* ∴ 照选项表写出的
  检查会有三行方向相反。**`--save-linked-ir` 与 `--save-temps` 拆开**：前者不是输出路径而是**产物类型**
   （`HIVMCMainA5.cpp:68-71` 把 `.o` 换成 `.ll` 且文件名加 `_linked`；`:158-163` 追加
   `-Xclang -disable-llvm-optzns -S -emit-llvm`，即同时**关闭优化**）∴ Late 模式**拒绝**，不得出现在任何
   用于产出被验收二进制的调用中；后者 `std::string` 默认 `""`（`Options.td:339-345`）确为目录，仅作诊断。

2. **恢复"两个推断函数零调用"**。此前 §3.2 的一版把该主张弱化为"该 flag 无消费者"，理由是二函数只
   `module.walk`、不加 pass。理由对、结论错：它描述的是**普通路径**。实测 `inferLayoutOptimization` 在
   `regbase/Driver.cpp:68` **无条件**调用、`inferMixedCV` 在 `:73` 调用，两者都在 `runRegBaseCompile`
   (`:63-85`) 内 —— 而 §2.2 的 Late 分支正是以其**首语句**返回。∴ Late 路径上二者**一次也不会被进入**，
   这正是目标 §4.1(1)"分流"的要求，主张既有意义又可断言。二者不矛盾：**零调用**由**各自函数入口**
   （`Utility.cpp:100`、`:61`）的边界日志**缺席**来断言；**pass 轨迹不能代替**，因为二者是普通
   `module.walk`，不加 pass，轨迹按构造看不见它们。边界日志的位置也一并修正为**函数入口**：
   `runRegBasePipeline`（`BiShengIRCompileMain.cpp:345`）有**两个**调用者（`Driver.cpp:81` 与
   `PassPipeline.cpp:564` 的 in-pass 形式），`runHIVMCCompileA5`（`hivmc/A5/HIVMCMainA5.cpp:944`）也有
   **两个**（`BiShengIRCompileMain.cpp:448` 与 `hivmc/HIVMCMain.cpp:45`）—— 记在调用点上就不成其为计数。

3. **模板主张与反例修正**。`-Wno-override-module` **不是**重复符号抑制：实测它对应
   `DiagGroup<"override-module">`，唯一诊断是 *"overriding the module target triple with %0"*
   （`third-party/llvm-project/clang/.../DiagnosticFrontendKinds.td:47-48`），两处出现
   （`HIVMCMainA5.cpp:97` 紧邻 `-cce-bitcode-is-aicore`；`:227` 在 `--save-linked-ir` 的回显里）都与
   **前端告警抑制**一致。替换向量**不依赖**该 flag，而完全由两处静默跳过解释：
   `bitcodePathList` 由 `getExtraDeviceBCPaths()` 起头、meta-op 路径仅在 core type 命中时追加
   （否则只告警 `:112-114`）、`bitcodePathList.empty()`（`:180`）是唯一硬失败**且**在用户已提供额外 bitcode
   时不可能触发、每个存活路径都被 `-cce-link-aicore-ll-module` 链入（`:185-188`）。∴ **断言该 flag 缺席的
   反例什么都证明不了**。反例拆成**四个**：N1 必需模板缺席（存在性）、N2 模板在位但**非记录身份**（这正是
   抓住 pre-D012 `44e4387b…` 的那行）、N3 提供 `--link-aicore-bitcode`（选项策略）、N4 两者同时（**不得**
   规定哪个诊断优先，因为选项拒绝会合法地先触发）。"今天会产生告警和二进制"是**由源码预判的假设，执行前
   一律 NOT_RUN**；未能证明"没有写出二进制"的行同样保持 NOT_RUN，不得由退出码推断。

4. **轨迹与白名单精确化**。引 `--help` 实测：pinned 二进制上**四个**过滤器都存在 ——
   `--mlir-print-ir-after-change`、**`--mlir-print-ir-after-failure`**（*"only print if the pass failed"*，
   即"仅失败才打印"，最易被误留、且会让干净轨迹与"什么都没跑"无法区分）、以及逐 pass 选择器
   `--mlir-print-ir-before=<pass-arg>`/`--mlir-print-ir-after=<pass-arg>`。覆盖率运行必须用**未过滤**的
   before-all + after-all。管理器按**实际进入者**枚举，**被正确绕过的 retry wrapper 无须出现**（反向也写进
   了：要求所有可构造的管理器都出现会使正确的 Late 运行不可伪证）；计数**按函数作用域**，"二十个构造条目
   不等于二十次执行"（`PassPipeline.cpp:564` 的 in-pass 构造正是反例之一）。

**4d —— Lead 的复核超出 Challenger 所指，并抓出第三处缺陷。** Challenger 点名要求白名单用规范身份
（`builtin.module`、`func.return`）。Lead 没有手改清单，而是跑了白名单本该据以生成的**实测 census**：
容器内 pinned `build/bin/bishengir-opt` `0761c6e6…` 对冻结输入 `d1023ac1…` 执行
`--mlir-print-op-generic`，退出码 **0**，得 **21** 个不同算子。census 证实了 Challenger 点的两个名字，
**并发现评审未点名的第三处**：**`scf.yield` 原清单里完全没有**（`scf.for` 恰有一个体终结符）。
三处遗漏后果相同：V1 会**拒绝被接受的冻结输入**，使每个 Late 测试都因错误原因失败 —— 正是目标 §7 B 所禁。
census 命令已写入 `docs/p2-architecture.md` §2.4 作为**再生成规程**（并明确白名单必须是**全名闭集等值**
比较，`startswith("hivm.")` 之类前缀测试会放进 `hivm.hir.mmad` 等，等于完全失去区分能力）。

**已落文档**：`docs/p2-architecture.md` §2.3.1（选项表逐行冻结 + 新增"TableGen 默认≠有效默认"块）、
§2.4（V1 精确白名单 + 21 算子 census 表 + 三处修正）、§3.1（`getLowerToLLVM` 措辞精确化：Late 直调绕过
全部门闸，断言 `LowerToLLVM=true` 是**入口不变量**而非依赖）、§3.2（六条前置条件：四过滤器、实际进入的
管理器、按函数作用域计数、恢复零调用主张）、§3.3（`-Wno-override-module` 更正 + 四反例表 + NOT_RUN 纪律）；
`docs/p2-pass-trace.md` A2/A2b/A4、过滤与计数规则、E1/E2、C1；`docs/p2-binary-abi.md` B8 + 更正段；
`docs/p2-verification.md` A1/A2b/A9/A12/A13、B10、E0。**以上均为控制仓/证据树文档，不是生产代码。**

**下一步**：请 `review-2` 就修正后的架构做 scoped entry-design 复审（goal §8 step 3 收口）；同时
`verify` 的测试冻结（§8 step 2）仍在进行。步骤 4（Implementor P2-A）仍须等二者报告后开始。

**未变**：P2 生产代码 **0 行**；`hivm-poc-evidence/` 仍为**本地专属**（不发布、不推送、不托管）；
两仓库远程配置未改、未推送；本轮全部改动仅落在 `hivm-poc-control/docs/` 与
`hivm-poc-evidence/p2-20261013-late-pipeline/review/`。

---

## P2 §8 step 2 —— Verifier 测试套件已冻结（v1），Lead 复核提出 3 项覆盖缺口 + 1 项绑定形态问题

**阶段**：P2 §8 step 2（Verifier 冻结独立验收测试与预期失败模式）。**P2 生产代码仍为 0 行**（冻结时点亦然）。

**冻结事实（Verifier 自有证据）**：`p2-20261013-late-pipeline/verify/`，`frozen_utc = 2026-10-13T13:28:57Z`，
35 个测试（A×13、B×12、C×6、D×4）；`P2-TEST-MANIFEST.json` 固定 29 个产物的 SHA-256，另有对 manifest 本身的
sidecar；`tests/config/p2-bindings.json` 是模板的逐字副本。退出码语义 0/1/2/3/4 明确，
判定词表仅 PASS/FAIL/BLOCKED/NOT_RUN。**C6 是唯一无需 P2 编译器即可执行的测试，冻结即 PASS**（20 个
fixture 全部对上计划 §1 标定表：19 个 parse rc=0，`b03_cache_bad7.mlir` rc=1 并给出 D012 值门诊断
`invalid store l2_cache_mode 7; valid store modes are 0, 1, 2, 4`）；其余 34 个 **NOT_RUN**。
**固定身份不匹配时报 BLOCKED、不报 FAIL/PASS** —— 这条失败安全设计正确。三项冻结时实测基线：
裸 parse rc=0；普通路径吃 Late 输入、默认 target rc=1（D012 target 门先触发）；换 950 target 则
**rc=134 SIGABRT in PlanMemory**。∴ "普通路径处理不了 Late 输入"是**实测**，不是推断。

**Lead 复核（对冻结件，未改动其任何断言）—— 四项问题已发回 Verifier，均源自 Challenger 第 4 轮要求**：

1. **轨迹覆盖不足**。A10/A11/A12 只用 `--print-pass-id --mlir-print-ir-after-all`；`--print-pass-id`
   在 `bishengir-compile` 与 `bishengir-opt` 上**均存在**（Lead 实测 `--help`：*"Print pass ID before each
   pass"*），机制本身成立。缺的是 (a) **没有断言四个过滤器处于关闭状态**，(b) 没有 `before-all`。
   四者在 pinned 二进制 `--help` 中都在：`--mlir-print-ir-after-change`、**`--mlir-print-ir-after-failure`**、
   以及逐 pass 选择器 `--mlir-print-ir-before=<pass-arg>`/`--mlir-print-ir-after=<pass-arg>`。这直接影响
   A10 自己的"零次"主张：`after-change` 若开着，跑了但没改 IR 的 pass 不打 header，"零出现"就不再等于
   "未被调度"。建议加**阳性对照**（在某段 IR 不可能改变的执行区间里，仍能看到至少一个 pass 打出 header），
   这比"命令行里没写这个 flag"更能判别。

2. **B12 一个反例 ≠ 要求的四个**。第 4 轮要求缺席 / **身份错误** / 禁止的额外 bitcode / 组合**分开测**；
   B12 目前只覆盖"缺席"。最要紧的未覆盖项是**身份错误**：`<stage>/lib` 下放着**非记录身份**的模板 ——
   具体就是树里现成的 pre-D012 `build/install/lib/meta_op.aiv.c310.O2.bc` = `44e4387b…`
   （31,179,496 B，2026-10-09 15:01），而 `getLibDirFromExecutable` 的 `<real exe>/../lib`（`real_path`
   **会解符号链接**）会静默解析到它。B12 的"false-PASS mode"散文其实已预见到"an alternative template"，
   但**没有对应测试**。同时缺 `--link-aicore-bitcode`（选项策略）与"缺席+额外"组合。
   注意：组合用例必须要求"拒绝且无二进制"，但**不得规定哪个诊断先触发**（选项拒绝会合法地先触发）——
   这正是"缺席"必须自成一例的原因。

3. **选项策略无测试**。计划与 runner 中**没有任何**测试点名：`--save-linked-ir`（Late 模式必须**拒绝**：
   它把 ccec 产物切成 `.ll` 并追加 `-Xclang -disable-llvm-optzns -S -emit-llvm`，不能出现在任何用于产出
   被验收二进制的调用里）、`--enable-static-bare-ptr=false`、`--enable-optimized-metaop`、
   `--enable-hivm-auto-storage-align`、`--enable-lib-call-no-inline`、`--enable-hivm-unit-flag-sync`、
   `--enable-preload`。A13 只覆盖 SIMT/MixCV/Host/legacy。两个设计陷阱已一并告知：(i) 因"未知选项"而
   拒绝的测试是**错因通过**，`--compile-host` 是实测例子（它根本不是 CLI 选项）；(ii) 接受侧各行**不能**
   照选项表写，因为 `applyArchDependentCompileDefaults`（`BiShengIRCompileConfig.cpp:374-383`）在 regbase
   且用户未显式传参时会把 `enable-lib-call-no-inline`、`enable-hivm-unit-flag-sync`、`enable-preload`
   强置为 true，而 `Options.td` 三者都写着 `false` —— 照表写会有三行方向相反。

4. **绑定形态**（唯一一处非"加测试"的问题）。`bitcode.aiv_c310_o2_path`/`_sha256` 被**非空**冻到
   **构建树**的 `build/lib/meta_op.aiv.c310.O2.bc` = `5e7ccae2…`，而 `p2_compiler`、`late_stage_flag`
   都正确地用了 null 纪律 —— 前后不一致，且会产生第 4 轮裁决所警告的那种错误产物。两条理由：
   其一，验收时被执行的编译器是**暂存副本** `<stage>/bin/bishengir-compile`，其模板按 `<real exe>/../lib`
   解析为 `<stage>/lib/…`，**不是**被 pin 的那个构建树路径；按 pin 路径生成的 identity sidecar 会记录一个
   **本次运行并未使用**的模板。其二，`5e7ccae2…` 是 P1 pin 的构建产物，它与 **committed D012 模板源**的
   关系**未定**：D012 改动过的 `RegBase/Vector/include/RegBase/DMAUtils.h`（79 行变更，在 ccec 包含路径上）
   写于 **14:42**，而逐文件模板 bitcode 是 **13:37:55**、合并件 **13:46:45**，**14:00 之后没有任何
   `RegBase/Vector/*.bc` 被重建**。∴ P2 树重建出的模板未必等于 `5e7ccae2…`，而 P2-C 自己的规则就是
   "记录重建后的 SHA 作为 P2 身份，不得假定"。**建议**：把 bitcode 的 path+sha 改成**验收时解析的 null 绑定**
   （不匹配即 BLOCKED，而不是被误断言），并把 `5e7ccae2…` 另立为 `p1_historical` 字段用于对照。
   **已明示 Verifier**：套件归它所有，Lead 不会去改；若它有异议，宁可把分歧记录下来，也不要让覆盖静默缺失。

**修订纪律**：若补测，必须是**具名的、有记录的 v2 冻结**（各自的 manifest 与时间戳），**v1 manifest 与 sidecar
逐字不动**，且**不得**是静默编辑；实现尚未开始，∴ 此刻补测仍属"实现之前"，符合 CLAUDE.md 与目标 §7。

**已落文档**：`docs/p2-verification.md` §0.1（冻结事实）与新增 §0.1a（Lead 复核四项）；
`docs/p2-architecture.md` §2.3.1/§2.4/§3.1/§3.2/§3.3（第 4 轮四项修正，见上一节）。

**下一步**：等 `verify` 对四项的答复（补测 → v2 冻结，或具名异议）；等 `review-2` 的 scoped entry-design
复审结论。**步骤 4（Implementor P2-A）在二者报告之前不开始。**

**未变**：P2 生产代码 **0 行**；`hivm-poc-evidence/` 本地专属；未推送、未改远程配置。

---

## 2026-10-13 第 5 轮：Challenger scoped 复审 → REVISE（仅剩两项规格缺口），已全部落实

**阶段**：目标 §8 步骤 3（Challenger 评审入口设计）收口。`review-2`（`challenger-opus`，实际 served
model **`gpt-6.1-sol`**，取自其 transcript 的 `model` 字段，非由 agent type 推断）在 commit `d2e6fae`
上给出：**REVISE，范围仅限两项规格缺口**，并明言这两处改完即可给 scoped **APPROVE**，不被 Verifier
未完成的测试冻结阻塞。裁决以 `SendMessage → team-lead`（rec458，13:31:13.555Z）送达，Challenger 未写
文件；按既有规则由 Lead **逐字**保存至
`hivm-poc-evidence/p2-20261013-late-pipeline/review/challenger-entry-design-verdict-round5.md`。

**裁决先确认**：第 4 轮的四组修正"在 `d2e6fae` 上已满足"，且 Challenger **独立复现了 21 个操作数的
census**（exit 0，含 `builtin.module`、`func.return`、`scf.yield`，计数与文档一致）。

### 阻塞项 1 —— 三个指令选择选项给具体值

原 §2.3.1 第 4 行只写"pin to the measured values"而不给值。现冻结为**全部 `false`**，显式 `true` 以
命名该选项的诊断拒绝。Lead 未采信，独立复核并**扩大**了证据面：

- `DisableFMA` 是 `FeatureControlOptions`，`bool`，默认 `"false"`（`Options.td:962-968`）；
  `DisableDecomposeReduction`（`:1340-1346`）与 `DisableReorderInstruction`（`:1348-1354`）是
  `SIMTOptimizationControlOptions`，同为 `bool` 默认 `"false"`。
- 全树 grep `setDisableFMA|setDisableDecomposeReduction|setDisableReorderInstruction` 只命中
  `hivmc/Config.h:106`（hivmc 自己的 setter）与转发点（`regbase/BiShengIRCompileMain.cpp:318`、`:325-326`；
  `BiShengIRCompileMain.cpp:212`、`:218-219`）；**通读** `applyArchDependentCompileDefaults`
  (`BiShengIRCompileConfig.cpp:371-384`) 全文，三者一个都没出现。生成 flag 的 grep（`disableFMAFlag` 等）
  只命中 `hivmc/Config.h` 的 `{false}` 与 hivmc 自己的 `cl::init(false)`（`HIVMCConfig.cpp:89`、`:331`、`:337`）。
  ∴ **pinned 值就是 regbase 路线上的实际生效值**。
- 三者**不是死旗**：`PassPipeline.cpp:349-350` 把其中两个喂进 pass options struct，A5 侧读 `setDisableFMA`。
  故该行属二进制身份，不是可有可无的清理。

### 阻塞项 2 —— 计数：**按 scope 测量，按整次调用断言**

原 §3.2 前置条件 6 把每个计数都"取在一个 function scope 内"。这会让**模块级** pass（如
`ConvertHIVMToLLVM`）在两个进入的 manager 里各跑一次仍读作 "once"。现改为一张测量/断言对照表：

| 断言 | 每 scope 测量 | 整次调用断言 |
|---|---|---|
| `ConvertHIVMToLLVM` | A5 pipeline 的模块 scope 计数 | **总计恰 1**；"每 scope 一次"或"tail 一次 + A5 一次"明确写成 **FAIL** |
| 禁止 pass | 每个进入的 scope 计 0 | **总计 0**；在一个 manager 缺席、在另一个出现是 FAIL，不是局部豁免 |
| `runHIVMCCompileA5` | 函数入口边界日志 | **总计 1** |
| tail 各 pass | 按 pass **且**按 operation scope（嵌套函数 pass 在 entry 与 VF helper 上各跑一次是**合法**的） | 跨整次运行的 §3.1 **顺序**，且每个 pass 的效果出现在产出 IR 中 |

已传播：`docs/p2-pass-trace.md`（规则条目、B 节导语、C2 行）、`docs/p2-verification.md` A9。另修 §2.5：
`AppendDeviceSpec` 的 "exactly once" 也改为整次调用级 —— 该 pass 非幂等，"每 scope 一次"正是其失效模式。

### 三项非阻塞清洁（均已落实）

1. **复用 helper 不证明 ABI 保持**。§2.3.1 原句称 ABI 不一致"by construction 是 Late pass 序列的 bug"。
   共用 transfer helper 约束的是**管道**不是**取值**：Late 入口若把 §2.3.1 各行 pin 成与 Golden 不同的值，
   会**经由同一个 helper** 产出不同 ABI。现改为 "very likely" 并说明理由，ABI 断言仍落在**产出二进制**上
   （§6.3 C1–C11 与 `p2-binary-abi.md` §C）。
2. **host 守卫是 config 断言，不是 CLI flag**。`CompileHost` 是 `BiShengIRCompileInternalStateOpt`
   （`Options.td:1550-1554`），文件注释（`:1547-1549`）自述 *"not currently surfaced as normal generated
   CLI/pass options"* —— 根本没有 `--compile-host` 可"拒绝"。§3.1 原 bullet 写"§2.3 拒绝 `--compile-host`，
   这才使 Late 的直接调用安全"，属于把不存在的 flag 当机制。现改为 `config.getCompileHost() == false`
   不变量 + config 状态断言测试。
3. **显式参数必须在解析之前送入**。`hasExplicitCLOption`（`BiShengIRCompileConfig.cpp:366-371`）测的是
   `getNumOccurrences()`，由 `bishengir-compile.cpp:102` 的唯一一次 `cl::ParseCommandLineOptions` 计数
   （位于 `registerAndParseCLIOptions`，`:89` 定义，唯一调用点 `:144`）。解析之后再注入只改 config 字段、
   `getNumOccurrences()` 仍为 0，随后 `applyArchDependentCompileDefaults` 会把值**覆盖回去**。

**Lead 的再推导比裁决多给出的一个约束（已向 Challenger 明确请其复核）**：deadline 比"解析之前"更早，
是**必须在 `:121` 之前**。因为 `main` 在 `:114` 先裸扫 target、在 `:121-124` 把 `argv` 拷进
`originalCLArgs`，而下游 `:186` 转发的是**这个 vector**；若在 `:121` 之后、`:144` 之前注入，会满足
`hasExplicitCLOption` 却让下游命令回显与 config 不一致 —— 正是 §2.3.1"一条规则"要防的 log-vs-binary
分歧。**落点**：与 §2.2 的 Late 检测/目标校验一道放在 **`:114` 之前**即可满足（validator 需要的 target
从同一次裸扫即可取得，不必来自已解析的 config）；放在 `:114`–`:121` 之间也满足，但**晚于 `:121`
不满足**。原文曾把窗口写成 `:114`–`:121`，读起来像"必须在 `:114` 之后"，与 §2.2"在 `:114` 之前"
并置会产生表面矛盾，故改为以 `:121` 为 deadline 的表述。

### 顺带修掉的三处既有引用错误

第 4/5 轮审查暴露 `Options.td` 行号错位，已改：`CompileHost` `:1551-1555` → `:1550-1554`；
`LowerToLLVM` `:1557-1563` → `:1556-1562`；其文件注释 `:1548-1550` → `:1547-1549`。

**已落文档**：`docs/p2-architecture.md`（§2.3 行 193、§2.3.1 第 4 行与两段新论述、§2.5 第 1 项、§3.1 host
bullet、§3.2 前置条件 6）、`docs/p2-pass-trace.md`（规则/B 导语/C2）、`docs/p2-verification.md`（A9、A12、
E0 第 5 轮、新增 E0b）、`review/challenger-entry-design-verdict-round5.md`（逐字裁决 + 处置表）。

**下一步**：已向 `review-2` 发出 scoped entry-design **APPROVE** 复审请求（含请其复核 `:114`–`:121`
窗口主张）；仍等 `verify` 对四项的答复（补测 → v2 冻结，或具名异议）。**步骤 4（Implementor P2-A）
在两者到齐之前不开始。**

**未变**：P2 生产代码 **0 行**；所有执行类验收行 **NOT_RUN**；`hivm-poc-evidence/` 本地专属；未推送、
未改远程配置；未改动 D031 的编译器 pin。

---

## 2026-10-13 第 5 轮收口：Challenger 给出 scoped entry-design **APPROVE** —— **门禁 3（§8 步骤 3）关闭**

`review-2`（served model **`gpt-6.1-sol`**）在 13:39:18Z 给出 **Scoped entry-design verdict: APPROVE**，
13:42:07Z 再次确认为 APPROVE（并专就"注入 deadline"一条作结）。两条消息均以 `SendMessage → team-lead`
送达，Challenger 未写文件；按既有规则由 Lead **逐字**保存于
`hivm-poc-evidence/p2-20261013-late-pipeline/review/challenger-entry-design-verdict-round5.md`
的"Gate 3 closing verdict"一节（转录记录 `rec480`、`rec504`）。

**裁决确认已解决**：三处指令选择选项为**具体 false pin 且显式 true 被拒**；计数表区分"每 scope 的 tail
执行"与"全局 `ConvertHIVMToLLVM=1` / 禁止=0 / A5=1 / `AppendDeviceSpec` 按状态 1 或 0"；解析窗口论述
"技术上正确"（`hasExplicitCLOption` 只在 `compile.cpp:102/144` 计数，`originalCLArgs` 在 `:121-124`
捕获、`:186` 转发）；21 操作数 census "现在正确"（Challenger 在容器内独立复现，pinned opt generic print
exit 0）；V1 闭集语义成立；模板 N1–N4 与 D012 的 parsed cache4 / 终端 barrier 要求明确；A–D 包装器限定
与源码一致（直接 Late builder 绕过三重门；`LowerToLLVM=true` 是入口不变量；`CompileHost` 仅 config 态；
pure-SIMT 守卫被排除）。

**"before `:121`" 被判为正确且无需弱化**：`originalCLArgs` 在 `:121-124` 快照 argv，`Driver.cpp:79`
把这一**不可变** vector 过滤进 config clArgs；在 `:121` 之后、解析 `:144` 之前注入会让
`getNumOccurrences()`/config 反映该选项而下游 clArgs 遗漏它，且"没有任何源码路径会在捕获之后
reconcile 该 vector"。置于 `:114` 之前属自然但非必需。

**APPROVE 附带实现义务 —— 已写成设计条文，而非留在评审消息里**（评审消息里的义务不可审计）。新增
`docs/p2-architecture.md` §2.3.1 四条义务：

1. **重复的用户 occurrence 是拒绝，不是合并**。`llvm::cl` 接受重复且 `getNumOccurrences()` 会计**每一次**
   occurrence，故用户传 `--enable-preload=true` 与本入口的 pin 同时存在会产生两次 occurrence、下游回显
   带两条。∴ 必须在注入**之前**拒绝矛盾的用户 occurrence，诊断须点名该选项与两个值；**不得**通过改写
   用户参数来"修好"重复 —— 那是换了件外衣的静默覆盖。
2. **每个 pinned 选项恰有一条规范 occurrence 同时抵达解析与下游回显**。注入后该选项 post-parse
   `getNumOccurrences()` 必须为 **1**，且 `:186` 转发的 vector 必须含**同一条**规范参数。只断言 config 值
   会漏掉翻倍的回显；只断言回显会漏掉被默认值挪动的 config。**两者都要**，否则该行未证。
3. **不得原地改 `argc`/`argv`**。`main` 在 `:114` 与 `:121-124` 读 `argv`，而捕获之后没有任何路径会
   reconcile，这正是 deadline 为 `:121` 的机械原因。入口**只构造一次**增广 argv，并让目标扫描与
   `originalCLArgs` 捕获看到**同一份**参数。
4. **契约范围写明，避免两种读法**。本结论成立于**当前转发契约下的 argv 注入**（`originalCLArgs` →
   `Driver.cpp:79` → config 的 `clArgs`）。Challenger 指出实现者**可以**在捕获之后用
   `config.setClArgs` reconcile —— 并非逻辑不可能，但那会**改变转发契约**且须自证 config 与下游 argv
   一致。本设计选择 argv 注入，∴ 事后 `setClArgs` reconcile **在本契约之外**，未修订本节不得引入。

另两条义务属**后续**门禁的条件，不属本门禁：validator 注册与 `--help` 证明；以及在 Verifier 给出执行
证据之前，**不得**对生产路径做任何 pass 计数 / ABI / 二进制 / 硬件声明。

**已落文档**：`docs/p2-architecture.md` §2.3.1（四条注入义务）；`docs/p2-verification.md` E0b →
**APPROVE**；`review/challenger-entry-design-verdict-round5.md`（追加"Gate 3 closing verdict"逐字节 +
义务处置表）。

**下一步**：仍等 `verify` 的 v2 冻结（它正在改 `run_p2_suite.py`：已记录修订、正在实现 pass 列表与
插桩化编译 helper，并新增 dump-pair 正控制）。CLAUDE.md 要求**测试先于实现**定义，目标 §8 步骤 4 的
门禁是"二者报告"，∴ **步骤 4（Implementor P2-A）在 v2 冻结落地后立即开始**，不早于此。

**未变**：P2 生产代码 **0 行**；所有执行类验收行 **NOT_RUN**；D031 的编译器 pin 未动；
`hivm-poc-evidence/` 本地专属；未推送、未改远程配置。

---

## 2026-10-13 第 5 轮追加：APPROVE 附带的三项非阻塞清洁 —— 其中一项是**文档中的假陈述**

Challenger 的 APPROVE 把三项事实性清洁列为非阻塞。Lead 逐条独立复核后发现**第一项不是清洁，是错误**，
另两项也确实需要收紧。

### 1. §2.3.1 "The frozen Golden's ABI was produced through **this** helper" —— **假陈述，已删并改写**

复核方式：直接读冻结件自身的头部，而不是回忆。`verify/tests/negatives/vadd_late_p1v2.mlir:9-11`
（sha `d1023ac1…`）自述：

> This is **HAND-AUTHORED text, not compiler output**. It is the input to the P2 Late
> pipeline; it is not itself evidence of numerical correctness.

且 P1 **没有**产出任何被验收的 A5 二进制 —— 该件是 P2 Late pipeline 的**输入**，不是任何路线的**输出**。
∴ 不存在"经由该 helper 产出"的参考 ABI，helper 也就不可能是"Golden 所依据的契约"。原文的推论链
（"Golden 是经此 helper 产出的 → 复用 helper ⇒ ABI 差异必属 pass 序列"）**两环都断**：前提假，且即便前提
为真，复用 helper 也约束不了取值。

改写后：§6.3 的 C1–C11 被明确为 P2 的**首次** ABI 测量（不是对既有基线的回归），其期望值来自冻结件与模板
**源**，**绝不**来自"同一个 helper 跑过"。helper 被降格为其真实作用：只排除"Late 路线漏接/多接管道"这**一
类**疑点。

### 2. Mix/Aiv 模式必须**逐 pass manager 重读**，V1 的入口检查必要但**不充分**

Challenger 第 3 轮即已指出 "For each later PM still check current input/reported mode because preceding
passes may write attributes"，此前只落实了"V1 拒绝携带 `mix_mode`/`FilterPassesAttr` 的输入"，**未**落实
逐 PM 重读。现补为 §3.2 的第三条后果：策略在**每个 manager 运行时**从当时的 module 重新解析
（`PassExecutionPolicy.h:42-46`），∴ tail 中若有 pass 写入这两个属性，其后所有 manager 的模式都会改变，
而 V1 的入口结论描述的是**更早的** IR。

已实测（本轮新测）：生产树中**没有**任何写入者 —— `mix_mode` 的 in-tree 写入只出现在
`unittests/Pass/PassExecutionPolicyTest.cpp:155`，`FilterPassesAttr` 只出现在
`unittests/Pass/FilterPassesAttr*`/`FilterPassesTest.cpp`，`OptimizeScalarTransfers.cpp:443` 只**读**。
∴ 重读预期与 V1 一致，但**保证不得建立在这一"缺席"之上** —— 与 §2.2 的死存储发现、C9 的零消费者发现
同理。

已传播：`p2-pass-trace.md` 的"每行须随模式报告"规则条目（改为"该段 trace 所属 manager 解析出的模式"）
与 **E5**（由"整次运行记录一次模式"改为"**每个进入的 manager** 记录"）。

### 3. `setLowerToLLVM` 的"无调用点"须限定范围

原文写"inspected BiShengIR source 中不存在 `setLowerToLLVM` 调用点"，范围未写明，读者可能误读为"不存在
该 setter"。实测：生成器**确实**产出 setter ——
`build/tools/bishengir/bishengir/include/bishengir/Tools/bishengir-compile/CompileConfigs.cpp.inc:1402`
（`setLowerToLLVM`，`:1407` 为 `getLowerToLLVM`）；全树搜索 `setLowerToLLVM` **仅此一处**。该文件由
TableGen 生成到 `build/`，∴ 只 grep 源码树会**静默**看不到它。现写明：被测量的是手写树中**无人调用**它，
而"生成但无人调用"的访问器改不动该 flag。

**已落文档**：`docs/p2-architecture.md`（§2.3.1 首段与 policy 段重写、§3.2 第三条后果、§3.1
`getLowerToLLVM` 条目）；`docs/p2-pass-trace.md`（模式报告规则、E5）。

**性质说明**：这三项不改架构切分，也**不**改变已发出的 scoped entry-design **APPROVE**（Challenger 明言
它们不改变已接受的切分）。第 1 项修正的是一个**若不修就会把 P2 的验收标准搞错**的假前提 —— 若照原文写，
C1–C11 会被当成"对 Golden 的回归比较"，而实际上没有任何可比较的参照物，测试会因此被写成错的形状。

**未变**：P2 生产代码 **0 行**；所有执行类验收行 **NOT_RUN**；`hivm-poc-evidence/` 本地专属；未推送。

**追加自查（同一轮）**：第 2 项要求若写成"**每一个**后续 pass manager 都要重读模式"，是不可实现的空头
支票 —— 入口无法拦截 **pass body 内部**嵌套构造的 manager（`PassPipeline.cpp:564` 正是这种）。已改写为
**可实现的**范围：重读发生在**入口自身调用的每个 manager** 之前（tail，尤其是 A5 —— 因为入口在两次调用
之间持有 module），并**明说**嵌套 manager 覆盖不到 —— 对它们，保证只来自 V1 加"生产树无写入者"这一实测。
把限度写出来，而不是用一句覆盖一切的话糊过去。

**第 5 轮最终措辞修正（Challenger 接受 E5 要求，但要求改措辞）**：Challenger 指出模式**不是**每个 pass
action 都重算，而是**在每个 manager 的首次 action 惰性检测并冻结该次 run 的其余部分**。回源码复核确认：
`PassExecutionPolicy.h:76-99` —— `modeDetected` 是**一次性** atomic，`getMode()` 其后只读 `mode`；类注释明写
模式是"state of a single compilation/run"、"detected lazily … on the first action of the run, and freezes
it for the rest of the run"。∴ 粒度是**per manager run**，不是 per action。已按此改写 §3.2 第三条后果与
`p2-pass-trace.md` E5 及模式报告规则：

- **不变量改为"记录每个 manager 边界的模式并断言无禁止迁移"**（而非"每个 action 重算"）。
- 对本冻结输入，**每个边界都必须读到 Mix；此后一旦迁移到 Aiv 即为 FAIL** —— 因为 denylist 会据此丢弃
  冻结契约所依赖的 pass body。
- 并记下类注释给出的**结构性**理由（而非仅靠实测缺席）：*"the passes that would rewrite `mix_mode` are
  not in the denylist"* —— 即这类 pass 在 Aiv 模式下**仍会执行**，且对**其后**的 manager 不构成任何冻结，
  这正是 V1 入口检查看不到的窗口。

**第 3 门最终确认（review-2，`idle_notification`，2026-10-13T13:54:00.601Z，逐字）**：该消息在 `094c730`
（E5 措辞改写）**之后**到达，∴ 它确认的是**改动后**的措辞，而非改动前：

> Scoped entry-design verdict: APPROVE. Recording the resolved execution mode at each entered pass manager
> is justified. PassExecutionPolicy detects mode at that manager's first action and freezes it for the run;
> a preceding manager could change attributes that a later manager observes. V1 validates the original
> input, so it cannot establish the mode of every later manager.
>
> The E5 wording in …/docs/p2-pass-trace.md:117 captures that distinction correctly. Each entered manager
> should report Mix; a later Aiv transition fails the accepted design. This does not require recomputing
> mode for every pass action.
>
> The accompanying corrections are sound: C1–C11 are P2's first ABI measurements, with expectations
> defined independently from the frozen input and template source; the absence of setLowerToLLVM callers
> is scoped to handwritten source, since a generated setter exists.
>
> This approval closes the entry-design review only. Actual Late routing, validator execution, pass counts
> and effects, C310 template consumption, ABI preservation, and binary authenticity remain NOT_RUN. The
> verifier's test-freeze amendments recorded in …/docs/p2-verification.md:22 remain a separate obligation
> before implementation. Hardware numerical correctness belongs to P3.

已核对：`p2-pass-trace.md:117` 正是 E5 行，措辞与要求一致；`p2-verification.md:22` 正是 §0.1a（v1 冻结的
Lead 复核，含四项缺口）。∴ **第 3 门 APPROVE 覆盖 `094c730` 之后的文档状态**，无遗留措辞项。

**`verify` 进展（同刻实测）**：转录 543 条，最后活动 13:55:01Z —— 正在把测试清单从 v1 的 **35** 项扩到
**40** 项（改写 A13、新增 A14/A15、更新 REGISTRY），即 v2 修订本身，不是卡住。**§8 步骤 4 的门禁仍
是这一冻结落地**；P2 生产代码仍 **0 行**。

---

## 阶段：§8 步骤 4 —— v2 冻结准入 + P2-A 开工（2026-10-13，控制提交 `72c5dff`、`1e6e686`）

**阶段**：测试冻结完成（v2），Lead 准入检查通过；Implementor 已开工 P2-A。
**结果**：**准入 = 通过**；P2 生产代码自此**不再是 0 行**（Implementor 运行中）。

### v2 冻结（不受信采纳，逐项独立复算）

| 项 | 声称 | Lead 独立复核 | 结论 |
|---|---|---|---|
| v2 manifest | `5b25ea9b…` | 复算一致；sidecar `sha256sum -c` OK；**30/30 条 pin 的文件全部重算一致** | ✅ |
| v1 manifest 未被改动 | `801b1edb…` | 复算一致（字节未变） | ✅ |
| 无 fixture 漂移 | 23 个 byte-identical | 23 个 fixture 对 **v1** pin 全部一致 —— **零漂移** | ✅ |
| 未弱化 v1 断言 | 声称 | 从 runner 重新推导：v1 的 A1–A13/B1–B11/C1–C6/D1–D4 全在；**forbidden-pass zero = A10、required order = A11、LLVM exactly once = A12**，均在且被加强 | ✅ |
| 40 项 / 计数 | A15 B15 C6 D4 | runner REGISTRY 实测 40，计数一致；唯一 id 差 = **B12→B12a–d**（已在修订文档声明）+ 新增 A14/A15 | ✅ |
| 冒烟 | PASS=1(C6) / NOT_RUN=39 | **Lead 亲自复跑**：`python3 -I tests/scripts/run_p2_suite.py` → exit 0、PASS=1 FAIL=0 BLOCKED=0 NOT_RUN=39，**与报告逐字一致**；日志 `p2-20261013-late-pipeline/lead-admission/lead-v2-smoke-repro.log` | ✅ |

v1 manifest 对 4 个文件（README、两个 bindings、runner）已不再匹配 —— 这**正是**被声明的修订，不是漂移。

### 两项记录缺陷（已上报所有者，未自行修改）

1. **声明时间晚于封存时间**：`P2-TEST-MANIFEST-v2.json` 声明 `frozen_utc = 14:05:00Z`，而其自身 mtime 为 **14:00:53Z**（宿主 UTC+8，已用 `date -u` 核实）——声明**晚于**封存 4 分 07 秒，且我 14:02:57Z 读取时**该时刻尚未到来**；`p2-bindings.template.json` 声明同一 `14:05:00Z` 而写于 13:56:18Z。v1 manifest 此项**完全正确**（声明 == mtime），∴ v2 是回归。
2. **一处措辞过宽**：修订文档称"no v1 assertion is weakened, removed, or **re-numbered**"。前两项成立（已确认无断言被删）；但 **B12→B12a–d 就是一次重编号**，虽然同文档已声明且严格更强。

**处置**：两份文件都是 Verifier 的、且**已封存**，Lead 改动会破坏封存 ∴ 未自行动手；已请其在**未封存的**规范位置（如 `verify/ERRATUM-v2-freeze-time.md`）记录真实封存时间 14:00:53Z，并修正措辞；30 条 pin 的 SHA 才是封存本体且正确，**不建议**为元数据再出 v3。
**并已披露**：Lead 的冒烟复跑重写了 `verify/logs/suite/*` 并新建 `verify/work/run`（两 manifest 均**不 pin** 任何日志；复跑后两个封存**复验 ALL OK**）。

### 验收矩阵重新推导（我的表曾与冻结套件脱节）

**发现**：`docs/p2-verification.md` 里 Lead 写的 A/B/C/D 表与冻结 runner **编号不一致**（B 从 B4 起错位一位、C 编号不同、D 漏了 D4）。**冻结套件是权威** ∴ 已按 runner **重新推导 A1–A15 / B1–B12d / C1–C6 / D1–D4**，并**显式记录**三条旧表有、冻结套件无的覆盖空白（**不是**悄悄删掉）：

1. Late 但**缺 CLI `--target`** —— `a*` 用例覆盖的是**输入**属性（stage/module core/device spec/hacc target），CLI target 缺失是另一回事，**无任何用例**。
2. **路由以 boundary log 断言**（Late 行在、ordinary 行不在）—— A1 成功**蕴含**走了 Late，但该断言从未由 trace 承载。
3. **`inferLayoutOptimization`/`inferMixedCV` 零调用** —— trace 对其**天生盲**（plain `module.walk`），需函数入口 boundary log。
4. 另记:goal §9 要求的**仓内可维护回归测试**在 C1–C5 中**没有**（它们是验收测试，不是仓内回归）。

⇒ 列为 **§8 步骤 8 的覆盖问题**（Challenger 裁定），**不是**开工阻塞；已明确**不**要求 Verifier 现在补测。

### 当前执行

- **Implementor（`implementor`）已启动 P2-A**，brief 含 APPROVE 的四条 argv 义务、validator 注册须有 `--help` 证明、不得覆盖 P1 pin（按 §3.3 G1–G4 暂存）、不得改封存测试/golden、**不得做任何生产路径断言**（pass 计数/ABI/bitcode/二进制只能由 Verifier 执行后成为证据）。
- `verify` 仍活跃（已收到勘误请求）。

**未变**：所有执行类验收行 **NOT_RUN**（除 C6 冻结时 PASS，且 Lead 复跑一致）；`hivm-poc-evidence/` 本地专属；**未推送**（本地 ahead）。
**下一步**：等 Implementor 交付 P2-A（源码 diff + 构建退出码 + 暂存包 SHA + `--help` 证明 + `getNumOccurrences()` 回读）→ 交 `verify` 做验收 → 再启 P2-B。
