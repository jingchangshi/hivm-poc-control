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

### v2 封存在准入后被**重封**（已披露，非静默）—— Lead 重新复算并加记限定

**事实（Lead 复算，2026-10-13T14:15Z）**：`P2-TEST-MANIFEST-v2.json` 的 sha 已由准入时的 `5b25ea9b…`
变为 **`bd9643b2…`**（mtime **14:14:39Z**）；30 条 pin **30/30 复算一致**、sidecar OK、v1 仍 `801b1edb…`、
23 个 fixture 对 v1 pin 仍**零漂移**。∴ 新封存**内部自洽**，实体内容可信。

**改动内容（均为**加强**，非弱化）**：
- **item 8**：A10 的模式断言不再只依赖输入的 `mix_mode`；现在**扫描每个 dump body**，若中途首次引入
  `mix_mode` / `filter_passes` / `FilterPassesAttr` 则 **FAIL**，并指名 dump 序号与引入它的 pass
  （否则"全程 forbidden=0"的主张不成立 —— 后续 manager 会重新解析模式）。输入级检查保留为第一道门。
- **item 9**：A2/C5 的 ordinary 路径字节比较**重新标注**为"对 pre-P2 **ordinary** 基线的回归"，
  以免被误读为 Late/A5 ABI 参照；断言本体未动且正确（ordinary 路径本就是既有的、先前已接受之物）。

**两项限定（已记入 `docs/p2-verification.md` §0.1d）**：
1. **冻结在实现开始后移动过**。Implementor 起于 ~14:07Z，重封 mtime 14:14:39Z。CLAUDE.md 要求测试先行 ——
   确实先行（v1 13:28:57Z、v2 14:00:53Z，均在开工前）；本次是对 **A10** 的**加强**，而 A10 断言的是**管线**
   行为、P2-A 入口本身并不实现它 ∴ **未改变 P2-A 的接口**。但这是一次**流程偏离**，记录在案、不当无事。
   ⇒ **v2 之后冻结对本阶段关闭**：再改必须是正确性修复、必须显式声明，且**作废 Implementor 的契约**并触发重验。
2. **时间戳缺陷未被修复，反而加重**。新 manifest 同时声明 `frozen_utc = 14:05:00Z`（沿用旧值）与
   `revised_utc = 2026-10-13T15:30:00Z`，而文件写于 **14:14:39Z** —— 声明时间**晚于写盘约 74 分钟**
   （已用 `date -u` 核实当前 14:15:34Z）。"no test was weakened, removed, or re-numbered" 的措辞**也未修正**，
   B12→B12a–d 的重编号仍被描述为"未重编号"。两项均在本次重封**之前**已上报，**仍为 open**。

**处置**：接受重封（实体可信、方向是加强），但①要求冻结此后稳定；②再次就时间戳与措辞措辞精确上报 ——
这不是风格问题：本项目的记录纪律正建立在时间戳与身份的可复原性上（D025 的教训）。同时**通知 Implementor
重读套件**（它在 14:09 读的 `run_p2_suite.py` 早于 14:14 的重封，其缓存的契约已过期）。

### 两项 v2 记录缺陷以**未封存勘误**关闭（§0.1e），非 v3 重封

**事实（Lead 从磁盘复核，2026-10-13T14:2xZ）**：`verify` 写了 `verify/ERRATUM-v2-freeze-time.md`
（5682 bytes）。复核三点：① 该文件**不在 v2 manifest 的 30 条 pin 之列** ⇒ 确为未封存件，写它**未破坏任何封存**；
② 其时间表与 Lead 自己的独立测量**逐秒一致**（v1 `13:28:57Z` == mtime；首次 v2 封存声明 `14:05:00Z` vs mtime
`14:00:53Z`；当前封存声明 `frozen_utc 14:05:00Z` + `revised_utc 15:30:00Z` vs mtime `14:14:39Z`；bindings 模板
声明 `14:05:00Z` vs 写盘 `13:56:18Z`）；③ 它以**实测 mtime 为记录时间**，且**准确**陈述缺陷 2：未弱化/删除任何
v1 断言，`B12 → B12a–d` 是一次**已披露的、纯加强的拆分**（增 3 条负例，原"缺失即失败"断言完整保留为 B12a）。

**为何是正确形态**：`README.md` 与 `P2-TEST-PLAN-AMENDMENT-v2.md` **两者都被 v2 manifest pin 住**，
改其中任何一个来承载更正都会打破上一行刚刚准入的那个封存。在它们**旁边**放一份未封存勘误，两个性质同时保住。
封存内容零改动，`verify` 事后复算 30/30 与两个 sidecar 仍 OK。

**处置**：**接受**，记入 `docs/p2-verification.md` 行 **0.1e**（"the correction of record"）。§0.1c 两项缺陷由
**open** 转为 **CLOSED**。§0.1b 的修订条目数由 7 更正为 **9**（item 1–7 首次封存、item 8–9 重封），
其结尾指向由不存在的 §0.1e 悬空引用改为已存在的 §0.1c 与 §0.1e。提交 `0c960df`。
**冻结对本阶段仍关闭**（§0.1d 限定 ① 不变）。

**未变**：所有执行类验收行 **NOT_RUN**（除 C6 冻结时 PASS，Lead 复跑一致）；D031 的 P1 编译器 pin 未动；
`hivm-poc-evidence/` 本地专属；**未推送**（本地 ahead：`1e6e686`→`0c960df`）。
**下一步**：等 Implementor 交付 P2-A（源码 diff + 构建退出码 + 暂存包 SHA + `--help` 证明 + `getNumOccurrences()` 回读）→ 交 `verify` 验收 → 再启 P2-B。

### P2-A 动工前的 Lead 预核查（§3.3 G1 的证据，2026-10-13T14:3xZ）

Implementor 已在生产树上动工（`80e29e6f2`，仅 4 个 P2-A 文件脏：`Options.td`、`HACC/Utils/Utils.h`、
`AppendDeviceSpec.cpp` 修改，新增 `regbase/LateInputVerifier.cpp`）。动工前 Lead 自己复核了 §3.3 的
"保留 P1 快照可复验性"，结论如下（**尚未构建**，`build/` 未被触碰）：

| 项 | 期望 | 实测 | 结论 |
|---|---|---|---|
| 保留件 `lead/preserved-p1-compiler/` | 三个文件 + SHA256SUMS | `bishengir-opt`(214 MB)、`meta_op.aiv.c310.O2.bc`(31 MB)、`bishengir-compile.pre-p2`(205 MB) + `SHA256SUMS.txt` | 存在 |
| 保留件自校验 | `sha256sum -c` 3/3 OK | **3/3 OK**（`0761c6e6…` / `5e7ccae2…` / `c6838fc0…`） | PASS |
| 活体 `build/bin/bishengir-opt` | 仍为 P1 pin `0761c6e6…` | `0761c6e6…`（mtime 14:46:23） | 未被覆盖 |
| 活体 `build/bin/bishengir-compile` | 仍为 pre-P2 `c6838fc0…` | `c6838fc0…`（mtime 13:46:21） | 未被覆盖 |
| 活体 `build/lib/meta_op.aiv.c310.O2.bc` | 仍为 `5e7ccae2…` | `5e7ccae2…`（mtime 13:46:45） | 未被覆盖 |
| Implementor 是否已构建 | 否 | 382 条记录内 **0** 条 `ninja/cmake/make/install` 命令；仅 1 条只读 `git status` | 未见越界 |

⇒ G1"在任何 P2 编译之前完成保留"**成立且有据**。P2 构建一旦发生，`build/bin/bishengir-opt` 与
`build/bin/bishengir-compile` 将同时改变（二者链接同一 `BiShengIRRegBaseCompileLib`），届时
`snapshot/run_p1v2.sh` 的 I5 会因**构建树移动**而非 P1 漂移而失败 —— 这不是缺陷，是预期，且
**不得**为迁就它去改快照或摘要（§3.3 已写明）。

### P2-A 动工中的 Lead 独立复核：新选项的**生成面是一条，不是两条**（已更正）

Implementor 在 `Options.td` 新增 `def InputIRStage`（`--input-ir-stage`，默认 `hir`）。我**不采信其注释**，
从生成器源码独立复算，结论两分：

**（a）三个身份复算一致 —— 正确**：`lowerLeadingAcronym("InputIRStage")`→`inputIRStage`（`result[1]='n'`
非大写，走早退分支）；`convertToSnakeFromCamelCase` 逐步推得 `input_ir_stage` → **`--input-ir-stage`**；
配置字段 `inputIRStageFlag`（`cppName + "Flag"`）；访问器 `setInputIRStage`/`getInputIRStage`。
且**顶层注册确实会生成**：`Option` 默认 `externalStorage=1`、`emitOptionRegistration=1`、`emitGetterSetter=1`
（`ConfigOptions/Options.td:63/119/122`），故 `emitCompileOptions` 不会跳过它 ⇒ Challenger 要求的
`--help` 证明**可达成**。

**（b）"没有 pipeline-options plumbing"—— 错，且错在把一个生成器当成了全部**：
- `ConfigsToOptionsConversionGen.cpp:80-82` 空组 `continue` —— 属实，但只管 **`ConfigUtils.cpp.inc`**。
- `PassOptionsGen.cpp:96-103` 是**另一个**生成器，**先无条件** `passGroup2Options["ALL"].emplace_back(R);`
  **再**判空组 `continue`；而 `GEN_ALL_OPTION_REGISTRATION` 在
  `PassPipeline.cpp:241` 与 `regbase/PassPipeline.cpp:570` **两处**都已 `#define` 并 `#include
  PassOptions.cpp.inc`。∴ 该选项**会**被生成为 `Pass::Option<std::string> inputIRStage`，且**两份**
  （`BiShengIRCompilePass` 与 `regbase::BiShengIRCompilePass`）。

**路由决策不变**：Late 分支在 driver 读 `config.getInputIRStage()`，不需要 pass-options 成员 ⇒
**不加 `passGroup`** 的决策**维持**。变的是**生成面的描述**，以及 §2.3.1 义务 2 的一个前提。

**为何这不是学究式纠正**：义务 2 要求"每个 pin 选项**恰好一次** canonical occurrence 同时到达 parse 与
下游 echo"。对该选项而言，这一主张现在依赖于"两处声明是不同 registry 中不同的 `cl::Option`，故顶层
`getNumOccurrences()` 不会被 pass 级声明翻倍"—— 这是**读源码的论证，不是测量**（`PassOptions : protected
llvm::cl::SubCommand`，`mlir/include/mlir/Pass/PassOptions.h:92`）。本项目恰有因"读源码当测量"而被迫撤回
主张的前例，故**记为待检义务，不记为事实**：写入 `docs/p2-architecture.md` §2.1、§2.3.1 义务 2，
以及 `docs/p2-verification.md` §A **覆盖缺口 4**（封存套件的 A13 只管 §2.3.1 的 pin/reject 行，既未断言
pass 命名空间无法取值，也未承载两个命名空间的 `--help` 证据）。**未改封存套件**。提交 `3349809`。

已通知 Implementor：只需**更正注释**（生成面含两份 `Pass::Option`），**不要**加 `passGroup`；测试是
Verifier 的，不需要它写。

### Lead 自查出**自己准入记录里的一个缺陷**（§0.1f）—— 以同一标准对待

`verify` 在其勘误消息里指出：我的准入引用 `5b25ea9b…`，而权威封存是 `bd9643b2…`，且我的 smoke 复跑
"ran at ~14:04Z — before that re-seal landed"。**我不接受断言、去量**，结果比它说的更严重：

| 事件 | 实测 (UTC) |
|---|---|
| Lead smoke 复跑 #1 的日志 mtime | **14:04:03Z** |
| `run_p2_suite.py` 最后一次写入 | **14:10:23Z** |
| v2 manifest 重封 | **14:14:39Z** |

⇒ 复跑 #1 **既不是**当前 runner，**也不是**准入的封存件：它跑的是 `14:10:23Z` 之前的 runner 修订版，
**早于**它本应验证的 item 8–9 加强。∴ §0.1b 那句"Lead smoke re-run reproduced the Verifier's result
exactly"把它当成了准入证据，**这是我自己记录里的缺陷**，与我在 §0.1c 上报的两项**同类**。

**处置（对称，不双重标准）**：①**保留**原日志，不删除（`lead-admission/lead-v2-smoke-repro.log`）；
②对**封存件**重跑 #2（`2026-10-13T14:31:27Z`–`14:31:36Z`，exit 0，runner sha
`0677be1a7ba217d7587c9c65436d93966ec5d40c03f5b8cc28c61989beea5621`），结果**完全相同**
PASS=1 FAIL=0 BLOCKED=0 NOT_RUN=39，日志 `lead-v2-smoke-repro2-current-runner.log`（两日志并置，顺序不会丢）；
③§0.1b 显式写出顺序；④两个 sidecar 事后复验 **OK**；⑤记入新行 **§0.1f**。

**该缺陷未改变结论**（C6 是 fixture 自检，item 8–9 不触及它）——但这是**重测之后**才成立的论证，
不是当初引用可以接受的理由。run #2 再次改写了 `verify/logs/suite/*` 与 `verify/work/run`（均非 pin），
已披露。

### 勘误件被**加强**（Lead 复读并复验，§0.1e 已更新）

`verify` 主动把勘误从"列表"升级为"认领 + 权威时间线"：`ERRATUM-v2-freeze-time.md` 由 5682 → **8903 bytes**
（sha256 `52d1c1f7…`，mtime 14:32:08Z），**仍不在任一 manifest 的 pin 之列**，两个 sidecar 复验 **OK**。
新增内容：①**认领严重性**（不是风格问题，是"未测时钟即写声明时间"的**做法缺陷**，第二次数值更糟——写进封存
manifest 的未来时间约 +74 分钟）；②**权威时间线**（以 mtime 为准：`13:28:57Z` / `14:00:53Z` / `14:14:39Z`
/ `13:56:18Z`，与我的独立测量**逐秒一致**），并声明 `frozen_utc`/`revised_utc` **两字段作废**；③新增 §5：
流程偏离、冻结关闭、**范围纪律**承诺（小修不得被并入大改）。

**两处残留不准确（已回报其作者，不由我代改——那是它的文件）**：
1. §1 括注写 *"host clock is UTC"* → 实测主机时区为 `Asia/Shanghai`（UTC+8，`date` 显示 `+0800 CST`），
   UTC 来自 `date -u`。措辞**颠倒**，且恰出现在"时间戳纪律"这句话里；表中数值仍全部正确。
2. §4 仍只描述"一次" Lead 复跑并仅引 `lead-v2-smoke-repro.log`；而针对**封存件**的 run #2 才是要紧的那次
   （§0.1f 已记）。

**未变**：所有执行类验收行 **NOT_RUN**（除 C6 冻结时 PASS，现以 run #2 对封存件复现）；D031 pin 未动；
`hivm-poc-evidence/` 本地专属；**未推送**。提交待 `0c960df`→`58d9aa0` 之后的下一条。

### 勘误两处残留不准确**已被作者修正**（Lead 复验）；并补强 §2.1 的安全论证

**复验（不采信报告）**：`ERRATUM-v2-freeze-time.md` 现 **9907 bytes**，sha256 `54a77755…`，
mtime `14:37:25Z`；`grep -c ERRATUM` 于两个 manifest 均为 **0**（未被 pin）；两 sidecar `sha256sum -c`
**OK**。§1 已改为"本地时钟为 UTC+8（`date` → `Asia/Shanghai`，`TZ` unset），UTC 由 `date -u` 取得"；
§4 已描述**两次**复跑并写明哪一次可采信，且注明 run #1 的引用已记为 Lead 自身记录的缺陷，故不会与 §0.1f 漂移。
∴ `§0.1e` 行内的"两处残留"由 *reported* 改为 *FIXED + 复验*（该行**必须**跟着改，否则它自己就变成我一直在
上报的那类陈旧缺陷）。

**§2.1 补强——省略 `passGroup` 是**承重**决策，不只是"够用"**：pass-options 面本身并非惰性，但
config↔pass-options 的**唯一**生成桥 `genConfigToOptions`（`ConfigsToOptionsConversionGen.cpp:37-43`）
只生成 `options.<cppName> = config.get<CapitalizedCppName>();`，**单向（config → options）**且按 pass group
用 `#ifdef GEN_<GROUP>_OPTION_SETUP` 守卫；空组选项因 `:80-82` 的 `continue` **根本不在任何 group map 里**
⇒ **没有任何生成路径会用 config 填充 `inputIRStage` 的 `Pass::Option`，也没有任何生成路径把 pass-option
的值回写进 config**。另一个收集器 `emitCollectConfigs`（`CompileOptionsGen.cpp:108-136`）以
`isSharedWithDownstreamToolchain` 过滤，而 `InputIRStage` 未置该位（默认为 0）。∴ 省略 `passGroup` 正是
让第二处声明**无法**成为第二条设置 stage selector 的路径；**不得**在没有重新推导的情况下加回。
**真正剩下的未决问题因此收窄为**：pass 命名空间声明**能否从 argv 取值**，以及两个命名空间的 `--help` 证据。

**冻结未重开**（我的裁定，回复 `verify`）：第五项覆盖缺口（`InputIRStage` 双命名空间）**维持为 §8 step 8
的 Challenger 待议项**，不因它重开封存 —— 理由：封存已在实现开始后移动过一次；上述安全论证已把
"能否写回 config"从**开放问题**降级为**构造性结论**；剩余 argv 可达性问题可随 §2.3.1 义务 2 已有的
`getNumOccurrences()` 回读与 `--help` 证明一并覆盖。若 Challenger 在 step 8 判需要执行级检查，则按**已关闭
冻结的协议**走：显式声明为 correctness fix、作废 Implementor 契约、重新验证 —— 由我在该 gate 决定，
`verify` 表示不会自行开启（正确）。

### §2.1 安全论证提升为**实测 + 可证伪预测**；并发现第三条"静默忽略"之门（已界定）

§2.1 此前的"单向桥"论证是读生成器源码得出的。本轮改为**在生成产物上核实**，并因此得到对验收门**可证伪的预测**：

| 生成文件 | 实测（P2 前） | P2 构建后的预测 |
|---|---|---|
| `ConfigUtils.cpp.inc` | `inputIRStage`/`inputIRStageFlag` **0** 次；**3 个** `GEN_<GROUP>_OPTION_SETUP` 组（HFUSION / HFUSION_TO_HIVM / HIVM，共 9 行匹配、127 条 `options.X = config.getX();` 体）（**更正**：此前记为"9 个守卫"，把匹配行数当成了组数） | 仍为 **0**（无 passGroup ⇒ 任何守卫块都不可能含它）。非 0 ⇒ 前提被证伪 |
| `ConfigUtils.cpp.inc` | `GEN_PASS_OPTION_TO_CONFIG` **0** 次 | 仍为 **0** —— `regbase/PassPipeline.cpp:540` 的 `#define` **什么都不守卫**；**反向**（pass options → config）压根未生成，旁边的 `// TODO now` 是唯一痕迹 |
| `PassOptions.cpp.inc` | 单一共享文件，276 个 `Pass::Option<`，其中 **150** 个在 `GEN_ALL_OPTION_REGISTRATION` 块内（441–1669 行）；该宏仅在 **2** 个 TU 中 `#define`（`lib/Tools/bishengir-compile/PassPipeline.cpp:241`、`regbase/PassPipeline.cpp:570`） | 文件中**恰好 1 个** `inputIRStage`，且位于 `GEN_ALL_OPTION_REGISTRATION` 块内、**不在**任何 `GEN_<GROUP>_REGISTRATION` 块内。**"2" 是编译单元计数，不是文件计数** —— 单文件 grep 查到 2 是不可满足的 |
| `CompileOptions.cpp.inc` | 整文件位于 `#ifdef GEN_OPTION_REGISTRATIONS` 下；条目形如 `static llvm::cl::opt<std::string, /*ExternalStorage=*/true> appendBishengOptions( "append-bisheng-options" …, llvm::cl::location(appendBishengOptionsFlag))` | 恰好 1 个 `inputIRStage` / `"input-ir-stage"` / `llvm::cl::location(inputIRStageFlag)` —— 这是**顶层 CLI 注册面**，`--help` 证据与 `getNumOccurrences()` 回读所作用的那一面 |

⇒ "没有任何路径把 pass-option 的值回写进 config"是**生成器的性质**，而非该选项组列表的性质；且这不仅靠读源码，
而是在产物上有 0 计数支撑。

**第三条门（已界定，非 P2 缺陷）**：`regbase::BiShengIRCompilePass::runOnOperation` 构造**默认**
`BiShengIRCompileMainConfig`，并依赖那个**空操作**宏把 pass options 映射进 config ⇒ 该路径上**所有**选项都被
同一机制忽略，在那里给 `--input-ir-stage=late-hivm` 会被静默忽略并跑普通管线。**这不是 P2 引入的回归**
（该路径从来不映射 pass→config），而 Late 入口的既定范围是 compile 工具（§2.2 item 1）⇒ **不要求 P2-A 改动**。
记录原因：§2.2 的最坏情形正是"Late 请求静默走普通管线"，这是 §2.2 **未列名**的第三条门。是否在范围内属
§8 step 8 的 Challenger 待议项。

**"0" 有既存对照件，因此不是预测。** `AppendBishengOptions`（`Options.td:879`）是**已发布**的无组
`std::string` 选项，实测**今天就不在 `ConfigUtils.cpp.inc` 中**（0 次），同时出现在 `CompileOptions.cpp.inc`
与 `GEN_ALL_OPTION_REGISTRATION` 块里 —— 这正是预测给 `InputIRStage` 的**三文件指纹**，由树中既有选项展示。
⇒ 验收可与**已知良好对照件**比对，而非与预测比对；且存储类型（`std::string`）完全一致。

**把这个表变成 grep 时的两个陷阱，两者都会给出假判定。**
*(a) 生成的声明是**多行**的* —— `Pass::Option<std::string>` 结束一行、`inputIRStage{` 起下一行（`CompileOptions.cpp.inc`
同理：类型在 `static llvm::cl::opt<…>` 行，拼写与 `cl::location` 在后续行）。单行模式
`grep 'Pass::Option<std::string> inputIRStage'` 在**正确构建上返回 0**，而按"0 即声明缺失"的读法就会 BLOCK。
应锚定行首名（`^inputIRStage`）或拼写（`"input-ir-stage"`），并核对上一行的类型。
*(b) "2" 是编译单元计数，不是文件计数*：`PassOptions.cpp.inc` 是**单一共享文件**，正确构建在其中只得 **1** 个
`inputIRStage`；两个对象之所以存在，是因为两个 TU 各自编译那一个 `GEN_ALL_OPTION_REGISTRATION` 块。对文件 grep
"2" 不可满足；"2" 的那一半须在源码层（上述两个 `#define` 点）或预处理层验证。

同期确认：Implementor 的 `regbase/Driver.cpp` 改动把 `config.getInputIRStage() == kStageLate` 的提前返回放在
`runRegBaseCompile` **首句**，与 §2.2 item 2 一致；`bishengir-compile.cpp`（`main`，§2.2 item 1 的 argv 扫描与
`:114` 前的诊断）**尚未出现**在 dirty 列表 —— 属预期，仍在施工中。

**以上三面计数由 Verifier 独立复测，逐项与我的值一致**（`ConfigUtils.cpp.inc` 8869 B、`PassOptions.cpp.inc`
67362 B / 276 / ALL 块 441–1669；`CompileOptions.cpp.inc` 60169 B）⇒ 表中数值不是单方声明。Verifier 同时**主动
收回了**它自己那份 check 2：其"文件级 grep 到 1 即声明缺失 ⇒ BLOCK"的规则把**文件计数**与**编译单元计数**混为
一谈，会在**正确构建**上触发 BLOCK；并确认单行 `type+name` 模式只能通过**畸形**构建。其三项 grep 现已全部改为
多行锚定（`^inputIRStage` / `"input-ir-stage"` + 上溯类型行）。argv 可达性仍留在 §8 step 8。

## P2-A 构建前锚点（Lead 独立复核，2026-10-13T14:59Z，构建**尚未**启动）

Implementor 在动工前写下 `impl/scratch/prebuild-state.txt`（14:50:51Z），钉住 HEAD 与两个**即将被覆盖**的二进制。
我在构建启动**之前**从磁盘独立复核了这两条，并把它与保留件对齐 —— 这是 P1 可复验性最脆弱的一刻：

| 对象 | 实测 SHA-256 | 对齐关系 |
|---|---|---|
| `AscendNPU-IR-1/build/bin/bishengir-opt` | `0761c6e662ee015e30425ee29e0c2314753aa356b21eb4119ff99747327a1977` | **等于 D031 的 P1 钉**（`p2-goal-1013.md:249`），亦等于 `lead/preserved-p1-compiler/bishengir-opt.0761c6e6` |
| `AscendNPU-IR-1/build/bin/bishengir-compile` | `c6838fc0a175ec6bb05214780a2a3e99ab478285620996dd5bea4c69f26aabe9` | 等于 `lead/preserved-p1-compiler/bishengir-compile.pre-p2.c6838fc0`（P2 前的普通路径基线） |
| `meta_op.aiv.c310.O2.bc` | `5e7ccae20f4e2a1887d1bdd87f787ea97f2732f7564a67013e737a163482fb42` | 保留件，C310 bitcode |

`sha256sum -c lead/preserved-p1-compiler/SHA256SUMS.txt` → **3/3 OK**。（首次尝试报 FAILED 是我自己在错误目录下执行
`-c`，清单内是相对路径 —— **我的路径错误，不是保留件缺失**；在清单所在目录重跑即 3/3 OK。记录此事以免后人把该
次输出误读为证据损坏。）

**未运行的构建**：`pgrep -x ninja` 全机仅 1 个（pid 3517234，`ninja -j 192`，属 `/home/c00910088/AscendNPU-IR/build`，
非本树）。**本树 `AscendNPU-IR-1/build` 无任何编译进程** ⇒ "同一构建目录不得并发构建"未被违反，且此刻本树产物
仍是 P1 身份。Implementor 亦自行做了 `ps` 扫描后才进入构建准备阶段（它正在比对本树与参考树的 `config.h` / ZLIB
配置，属构建前置诊断，尚未编译）。

## P2-A 交付准入（Lead 独立复核，2026-10-13T15:20Z）— 全部结论由本人从磁盘/容器复算

Implementor 交付 P2-A（入口 + 失败即止的 preflight），并**主动停止**在 Verifier 验收前。**未提交、未推送、
未进入 P2-B/P2-C，未作任何验收声明**。以下是我**不接受转述**而逐项复算的结果。

### 1. 复算通过项（每条都有真实命令与输出）

| 项 | 我的实测 | 判定 |
|---|---|---|
| 暂存包是**实拷贝**而非符号链接 | `ls -la` 为 `-rwxr-xr-x`，`bin/` 与 `lib/` 同级 ⇒ `getLibDirFromExecutable` 可得 `<stage>/lib` | 通过 |
| 暂存二进制 SHA-256 | `2bf961bf…`(compile) / `179788df…`(opt)，与 `stage-p2a-SHA256SUMS.txt` 逐字节一致 | 通过 |
| 暂存 C310 bitcode | `5e7ccae2…`，**等于 P1 历史钉** ⇒ "P2 构建未重生成模板"是真测量，非假设 | 通过 |
| P1 恢复 | `build/bin/bishengir-opt`=`0761c6e6…`（**= D031 钉**）、`bishengir-compile`=`c6838fc0…`，`cp -p` 保住 mtime（14:46:23 / 13:46:21） | 通过 |
| P1 快照门禁 | 容器内 `python3 -I verify/check_snapshot_integrity.py` → **rc=0**，`PASS: all integrity invariants hold (I1-I7)`，FAIL 行 0 | 通过 |
| 恢复的**同一性**证明 | 容器内恢复后的两个二进制 `--help` 为 rc=0（543 / 2420 行），新 flag 出现 **0** 次 ⇒ 恢复是真恢复 | 通过（见 §2 自曝） |

### 2. 自曝：我第一次做"恢复同一性证明"时得到了**空洞的 0**

我在**宿主**上跑 `build/bin/bishengir-compile --help | grep -c input-ir-stage` 得 0，几乎据此宣布"恢复为真"。
实测该二进制在宿主上**根本跑不起来**：`GLIBCXX_3.4.29/3.4.30 not found`、`GLIBC_2.32 not found`（宿主 libstdc++/glibc 过旧），
rc=1、6 行错误。**0 是因为进程没起来**，不是因为不含该 flag —— 与 §A 所述"错误原因通过"同一类缺陷，
只是发生在**我的**手上。已改为容器内经 `scripts/in-container.sh` 复跑，得到 rc=0 且 0 次的**非空洞**结果。
教训入账：**本仓二进制只能在容器内执行，宿主上的任何 `--help`/`--version` 结果一律无效**（这是先前若干次
"零"读数的通用前提，值得所有行共用）。

### 3. 运行时指纹：`input-ir-stage` 出现 **0 / 1 / 2** —— file-grep 做不到的那半

容器内实测（证据 `lead/p2a-admission/help-staged-{opt,compile}.txt`）：

| 二进制 | `--input-ir-stage` 出现次数 | 含义 |
|---|---|---|
| P1 `bishengir-opt`（已恢复） | **0** | 旧身份 |
| 暂存 `bishengir-compile --help`（:80） | **1** | 顶层 CLI 注册面（`CompileOptions.cpp.inc`）—— 义务 2 的 `--help` 证据 |
| 暂存 `bishengir-opt --help`（:258、:517） | **2** | **两个 `Pass::Option` 对象** ⇒ 与"2 个定义 `GEN_ALL_OPTION_REGISTRATION` 的 TU"在运行时对齐 |

**这正好补上 `verify` 的三项 grep 结构上无法覆盖的一半**：`PassOptions.cpp.inc` 是单一共享文件，文件级只能得 1；
"2"必须在源码层或**运行时**验证。`bishengir-opt --help` 的 2 次是**运行时**证据，比源码声明更强。
另：`--hivm-verify-late-input` 在 opt 的 :1452 出现，注册为真（不是 unknown-option 错误）。
⇒ §8 step 8 的"argv 可达性"问题的**一半**（两个 pass 命名空间声明确实被注册且可见）已在 P2-A 有运行时证据；
剩下的一半（能否从 argv 赋值）仍属 step 8。Implementor 的 §4 未报告 opt 里的这两次，属**漏报**而非错报。

### 4. §5 环境事故：我独立交叉验证了"修复已复原原始配置"

Implementor 坦承：它一度在**宿主**上跑 `ninja`，触发 CMake 重跑，把宿主发现写进了 `CMakeCache.txt` /
`build.ninja` / `include/llvm/Config/llvm-config.h`（宿主有 zlib）。随后自诊断并修复。
**这不是预期行为，也触及"未经明确同意不得做环境重建"这条红线；已检测、已诊断、已修复、已留证，但必须上报。**

它的"已复原"结论**正确**，但它给出的**论证不足**：`llvm-config.h` 读出 `LLVM_ENABLE_ZLIB 0`，而
"选项 ON + 找不到 zlib"**同样**得到 0 —— 该单一数值无法区分"已复原"与"仍受损"。我改用**事发前的记录**交叉验证：

- `build.log:41/46`（**Oct-9 原始 configure**）：`Check for working C compiler: /home/s00653124/Softwares/clang+llvm-15.0.6/bin/clang[++]`
  ⇒ 现缓存中的 `CMAKE_CXX_COMPILER` **就是原始值**，宿主那一次没有改掉工具链。
- `build.log:134`（同一原始 configure）：`-- Could NOT find ZLIB (missing: ZLIB_LIBRARY ZLIB_INCLUDE_DIR)`
  ⇒ **原始配置本就找不到 zlib**，故 `LLVM_ENABLE_ZLIB:STRING=ON` + `ZLIB_LIBRARY:NOTFOUND` + `llvm-config.h` 为 0
  **正是原始状态**，不是漂移。修复日志 `cmake-ccache-path-restore.log` 的那一行与它逐字相同。
- `CMAKE_CACHE_*`=3.31.12、`CMAKE_COMMAND=/home/s00653124/Softwares/cmake/bin/cmake`：与修复所选一致。
- `build.ninja` 中 `c00910088` 路径 0 次 ⇒ 未把参考树路径混入。

**代价（诚实记录）**：因 `llvm-config.h` 被重写，ninja 判全树过期，G1 是 **~4385 边全量重建**而非增量。
"保留原有增量构建能力"这一条在该时刻被**消耗**掉了；重建后各对象（23:07–23:08）新于该头文件（23:05），
树已自洽，**增量能力已恢复**。P1 可复验性未受损（门禁 rc=0，保留件 3/3）。

### 5. 作用域裁定：P2-A 下这些行是 **NOT_RUN**，不是 PASS，也不该记成 FAIL

我读了冻结 runner 的 `_instrumented_compile`（`run_p2_suite.py:665-666`）：**正例编译 rc!=0 时直接判 FAIL**
（`instrumented positive compile rc=1 (expected 0)`）。P2-A 的 Late 路由按设计在 `LatePipeline.cpp:544` **失败即止**（rc=1），
因此把它绑上去跑全套会产出**几十个 FAIL**——而那些 FAIL 的原因是**作用域未实现**，不是实现缺陷。

- **不是 PASS**：§A 已写明"文件存在或编译器 exit 0 不足以宣布成功"；trace 型零值在尾段缺席时是**空洞零**
  （B 行"未调度"的强命题需要尾段存在才可证），把它们记 PASS 就是假绿。
- **也不是 FAIL**：记 FAIL 等于宣称"入口是坏的"，而入口的失败即止**正是本阶段的设计目标**。
- **正确verdict = NOT_RUN + 写明作用域理由**，且必须满足一条**判别性要求**：
  **一次 Late 调用必须走到"预期的那个拒绝点"，且已越过 preflight** —— 否则就是因错误原因而止。
  实测证据：`impl/scratch/late-trace.log` 的 `[ERROR]` 是 `:540` 那条（"pass sequence and the bitcode identity gate
  are not implemented"），而 `:487`/`:511` 两条更早的拒绝消息**没有**出现 ⇒ `verifyPinnedOptions` 通过、
  `config.getCompileHost()` 为假、且 `runPipeline(module, buildPreflight, …)` **返回成功** ⇒
  **LateInputVerifier（V1–V7）在真实 P1 v2 Late 输入上通过了**，`AppendDeviceSpec` 按 §2.5 三态决策调度。
  这是 P2-A 的**实质正结果**，但该日志未加 `--print-pass-id`，故属**间接**证明。

**因此 P2-A 的作用域内可执行且非空洞的行**（验收应据此用 runner 的 `--only` 选择执行，而不是全跑）：
黄金 Late 输入走到预期拒绝点（rc=1、**无产物**、带 `--print-pass-id` 的 trace 中可见 `hivm-verify-late-input`）；
V1–V7 的**反例族**（错 target / 残缺 spec / 缺 pb32 载体等必须被拒且给出 validator 自己的诊断，而非 MLIR 解析错误）；
**A5**（普通请求逐字节不受影响——本阶段最有价值的回归行）；**C6**（已在冻结时 PASS，且它是 P1 钉的唯一消费者）；
选择器反例 **B12a–B12d**。**其余全部（A1–A4、A10–A12、B1–B11、C1–C5、C7+、D1–D11）记 NOT_RUN 并写明理由**，
P2-B/P2-C 落地后转为强制。
—— 由此**更正 Implementor §8 的一处过度声明**：它称"A3–A9/A13/A14/B3–B11 become executable once bound"。
按代码与 runner 的前置条件，这些行要么**不能通过**（A4 要求 `runHIVMCCompileA5` **恰好 1 次**，尾段缺席时是 0），
要么只能给出**关于 preflight manager 的弱零**，其强命题属于尾段。可执行 ≠ 可判 PASS，此处必须说清。

### 6. 对 Implementor 三项请示的决定

- **(a) 提交**：**准许提交但不得推送**，且 commit message 必须显式写明"**尚未验收**（pending Verifier acceptance）"，
  以免未验收状态被后读为已验收。提交范围仅限这 10 个文件。理由：diff 快照（`p2a-implementation.diff`, sha `9cea956e…`）
  虽已有保全，但本地提交是更稳的检查点，且提交本身可逆、不触红线。
- **(b) `docs/status.md`**：**不要写**。该文件是 Lead 的记录，本块由我亲自写（即本段）；避免并发编辑。
  它应把自己的报告落到 `impl/P2A-REPORT.md` 留档。
- **(c) O0 模板**：**暂存**，但**不得替换**已绑定的 O2 钉；`SHA256SUMS` 中按名列出，O2 仍为 `bitcode.aiv_c310_o2_path` 的唯一绑定。
  理由：§6.2 明确要求检查 O0/O2 选择，P2-C 一定会要；现在多存一个只读文件成本为零，可省一轮往返。

### 7. 计数对账（46 vs 47）

`INTEGRITY.json` 的四个块 sha256 值分别为 18 / 5 / 2 / 21，**合计 46**，我此前记的 47 是把 `compiler` 块里一个
`path` 键当成摘要值数了进去。**已对账，无失败**。

### 8. P2-A 生成面实测（预测 → 实测，Lead 亲自复算，2026-10-13T15:25Z）

§2.1 表的三行预测在 P2-A 构建后已全部变为**实测值**，三条都符合设计，且第一条比预测更强：

| 生成文件 | 预测 | 实测 |
|---|---|---|
| `ConfigUtils.cpp.inc` | 仍为 0 | **0**，且文件与其 **Oct-9 原件逐字节相同**（sha `e54c65ae…`，mtime 仍为 `2026-10-09T14:55:24`）——P2 构建**连重写都没发生** ⇒ check 1 以最强形式通过 |
| `PassOptions.cpp.inc` | 文件级**恰好 1** 个声明，位于 `GEN_ALL` 块内、**不在**任何 group 块内 | **1** 个，位于 `:1193`；`GEN_ALL` 块现为 **441–1675**（因该声明六行而增长），下一个 group 守卫在 `:1678` ⇒ 确在 ALL 内、确不在 group 内 |
| `CompileOptions.cpp.inc` | 恰好 1 个 `inputIRStage` / `"input-ir-stage"` / `llvm::cl::location(inputIRStageFlag)` | **1** 个声明（`:741`），形状与对照件 `appendBishengOptions` 完全一致（含 `,llvm::cl::cat(featCtrlCategory)`） |

**并发现第三种 grep 陷阱（已并入 `p2-architecture.md` §2.1 的"两个陷阱"段，现为第三条）**：名称是 flag 名的**子串** ——
在 `CompileOptions.cpp.inc` 里朴素质数 `inputIRStage` 得 **2**（`:741` 的声明 + `:744` 的
`llvm::cl::location(inputIRStageFlag)`），而声明**只有一个**；对照件同样如此（`:8` / `:11`）。故 check 3 必须以拼写
`"input-ir-stage"` 或声明行形状锚定。**该陷阱只存在于 `CompileOptions.cpp.inc`**：`PassOptions.cpp.inc` 用
`Pass::Option`，无 `Flag` 形式，朴素质数本来就是 1。

### 9. P2-A 身份清单（Lead 复核后入账，2026-10-13T15:35Z）

| 项 | 值 | 我的复核 |
|---|---|---|
| 生产代码提交（**仅本地**） | `ddc212e299f4493cc1ed8fa8a24cb6c9408e42b6`，parent/base `80e29e6f27192833d2d6cd3fd846e0b78ee02d1c`，分支 `feat/tilelang-hivm-late-pipeline`，10 文件，+1785/−19 | 提交内容确为那 10 个文件；工作树干净 |
| 可否被误推 | 分支**无 upstream**（`git rev-parse @{u}` → no upstream configured），`git branch -a --contains` 仅本地分支 | 结构性不可误推；远端配置未动 |
| 提交时可读性 | message 含 **"NOT YET ACCEPTED — pending Verifier acceptance"**，并写明**验收对象是暂存包而非 `build/bin`**（后者已回 P1） | 逐字核对通过（`impl/scratch/p2a-commit-message.txt`） |
| 权威 diff 导出 | `impl/p2a-commit.diff`，92,625 B，sha `929d1684fa46cc1ff1222b8272b2e1c3a7ea41ed3a23be1d8257d9fad12cbcf9` | 与 `git format-patch -1` 一致；旧 `p2a-implementation.diff`（`9cea956e…`）**已标注被取代**（它只含 7 个修改文件，且早于 §4 注释更正） |
| 暂存二进制 | `bishengir-compile` `2bf961bf…` / `bishengir-opt` `179788df…`（实拷贝） | 逐字节复算通过 |
| 暂存 bitcode | O2 `5e7ccae2…`（**= P1 历史钉**）；O0 `0a34be8759a082d2b446e5369455d20c26d432eea19a5ca516c0e189ddafc258`（27,695,180 B） | 两者 mtime 均 `13:46` ⇒ P2 构建**未重生成任何**模板；**O2 仍是唯一绑定**，O0 仅并列存放 |
| 报告 | `impl/P2A-REPORT.md`（18,362 B），§5 已改用**事发前记录**（`build.log:41/46`、`:134`，且全文 `Found ZLIB` 0 次）证明修复，§4 已补报 opt 的两次运行时证据 | 已核 |

**记录可用性缺陷（已要求 Implementor 修正）**：`impl/stage-p2a-SHA256SUMS.txt` 位于其相对路径基准目录的**上层**，故
`cd impl && sha256sum -c stage-p2a-SHA256SUMS.txt` 得 **4× FAILED open or read**，而正确基准
（`cd stage-p2a && sha256sum -c ../stage-p2a-SHA256SUMS.txt`）为 **4/4 OK**。**我本人先读成了"包损坏"**，随后才发现是基准问题 ——
与我在 P1 保留件上犯的同类错误。已请其把清单移入 `stage-p2a/`（与 `lead/preserved-p1-compiler/SHA256SUMS.txt` 一致），
并已即时提醒 Verifier：**`sha256sum -c` 的结论必须连同清单基准目录一并记录**，因为"路径错"与"哈希不符"在该输出里无法区分。

### 10. 暂存清单基准缺陷已修复 + 绑定期身份不变（Lead，2026-10-13T15:22Z）

**Implementor 已按请求把清单移入其基准目录**，并把"基准"写成文件自身的一部分（这是一条记录纪律，不只是搬文件）：

- 旧路径 `impl/stage-p2a-SHA256SUMS.txt` —— **已不存在**；
- 新路径 `impl/stage-p2a/SHA256SUMS.txt`（928 B，基准 = 其自身所在目录，头部注释明写
  `cd stage-p2a && sha256sum -c SHA256SUMS.txt`，并注明"错误的 cwd 打印 `FAILED open or read`，
  与哈希不符无法区分"）。

**我在绑定仍在进行时复算，四个身份逐字节未变**（这是要点：绑定期不得重构建/重暂存，否则 Verifier 的绑定对象就失效了）：

```
cd impl/stage-p2a && sha256sum -c SHA256SUMS.txt   ->  4/4 OK
2bf961bf80f99e7690f1def6bc7f6165bfe557404ebbd39fb81170f1ad2444c2  bin/bishengir-compile
179788df75f07ca19b301eef7d787992eed428c78787914c2e92256ed1beb0cb  bin/bishengir-opt
5e7ccae20f4e2a1887d1bdd87f787ea97f2732f7564a67013e737a163482fb42  lib/meta_op.aiv.c310.O2.bc
0a34be8759a082d2b446e5369455d20c26d432eea19a5ca516c0e189ddafc258  lib/meta_op.aiv.c310.O0.bc
```

容器侧同时确认**本树无任何 `ninja` / `cc1plus` / `bishengir` 进程在跑**（唯一在跑的构建属于他人树），
故"一个构建目录同一时刻只有一个构建"与"绑定期不重构建"两条同时成立。

**已即时告知 Verifier 该路径变更**（其脚本/笔记若仍引用旧路径，现在会读失败，而不是哈希不符），并附上四个身份值。
**同时向 Verifier 指出一个证据缺口**：`impl/scratch/late-trace.log`（Implementor 自有）只证明了 9 个 pinned 选项
`occurrences=1 echo=1` 与 `:540` 的 fail-closed 行，**不含 pass 流**，因此判别式的"preflight 真的跑过"这一半
目前在盘上无证据；`:540` 一行无法区分"preflight 跑过并通过"与"`runPipeline` 静默什么都没做"。
`--print-pass-id` 在暂存编译器上是**真实存在的旗标**（`lead/p2a-admission/help-staged-compile.txt:30`），
故要求验收运行带该插桩并记录 `hivm-verify-late-input` 出现在拒绝行之前 —— 这正是让周边 NOT_RUN 成为**正当范围裁定**
而非规避的东西。

### 11. 第三种 grep 陷阱经复核确认为**生成器固有行为**（非 P2-A 产物）—— Lead 独立复算（2026-10-13T15:2xZ）

Implementor 提出异议：裸名计数为 2 不是 P2 引入的，而是 TableGen 生成器对**该表面上每个选项**都这样发。
**我没有采信其转述，而是在一个完全不含 P2 代码的树上独立复算**：

| 复算项 | 实测 |
|---|---|
| 参照树（只读） | `/home/y00897153/workspace/tilelang-mlir-ascend/3rdparty/AscendNPU-IR-Dev/build/…/CompileOptions.cpp.inc`，52,204 B，**mtime `2026-09-11T04:06`**（早于 P2 约一个月） |
| 该树既有选项的裸名计数 | `grep -c appendBishengOptions` → **2** |
| 两行原文 | `:8` `static llvm::cl::opt<std::string, /*ExternalStorage=*/true> appendBishengOptions(`；`:11` `,llvm::cl::location(appendBishengOptionsFlag)` |

**结论（已写入 `p2-architecture.md` §2.1 的陷阱清单，并把标题"两个陷阱"更正为"三个"）**：倍增是**生成器**对
该表面的固有行为，**任何年代的任何构建（含 P1 钉定件）皆然**。因此验收断言**必须**以
`llvm::cl::opt<…>` / `Pass::Option<…>` **声明**、或带引号拼写 `"input-ir-stage"`（实测恰好 1）为准；
**以裸名计数并要求 1 的检查会在正确构建上误报 BLOCK** —— 这正是本项要防的假 BLOCK。

**暂存包复核（同一时刻，绑定仍在进行）**：新清单 `impl/stage-p2a/SHA256SUMS.txt` sha256
`6d1d4a06d64c50823fbf938105aef7c4a6d33d8d26ba6531a9de102f85de033f`（与 Implementor 自报一致），
全 `impl/` 下**无残留旧清单**；`cd impl/stage-p2a && sha256sum -c SHA256SUMS.txt` → **4/4 OK**。
**四行摘要与旧 358 B 版本逐行一致**（旧版我在本会话中已完整读过，两相对照即为独立确认），故这次搬迁是**保内容的**。

---

## P2-A 验收裁定：**不接受（1 个真实 FAIL，1 个已披露的假绿），授权定向修复第 1 次**（Lead，2026-10-13T15:4xZ）

Verifier 的 P2-A 验收已落地（绑定时刻 `2026-10-13T15:24:48Z`）。**我从盘上独立复算了其每一条裁定，不采信其转述**，
结论：**P2-A 不予接受**。这不是流程形式，而是冻结测试集抓到了一个**载荷性设计偏离**。

### 1. 我独立复算通过的部分（这些确实成立）

| 项 | 我的复算 |
|---|---|
| 绑定 | `p2_compiler` = 暂存 `2bf961bf…`；`late_stage_flag` = `--input-ir-stage=late-hivm`；O2 = `5e7ccae2…`（**与 P1 历史钉逐字节相同** ⇒ "P2 未重生成模板"由假设升级为事实）；**O0 保持 null**；基线 `c6838fc0…` 与 P1 opt `0761c6e6…` 未被覆盖 |
| 判别式（**直接证据，非间接**） | stdout `[PassID] hivm-verify-late-input/module/0`；stderr `:2` `IR Dump Before … LateInputVerifierPass (hivm-verify-late-input)`，`:48` `IR Dump After …`（**全文件 `Failed` 出现 0 次**），`:95` pass statistics，`:99` **唯一** `[ERROR]` = `:544` 那段 P2-A stub；`:487`/`:511` 的诊断均未出现 |
| 判别式所示输入 | 即 golden Late 输入本身：`hivm.module_core_type<AIV>`、三参数 `memref<524288xf32, gm>`、`dyn_shared_memory_buf = 196608`、UB pointer_cast 基址 `0/65536/131072`、四个 set/wait、store 带 `{l2_cache_mode = 4 : i32}`、末尾 `pipe_barrier[<PIPE_ALL>]` |
| 三面 grep（重生成树上） | ConfigUtils 8869 B **mtime 仍 2026-10-09**（从未重生成）；PassOptions 277 `Pass::Option<`（276+1），`^inputIRStage` 恰 1（`:1193`，在 `GEN_ALL` 441–1675 内、不在任何 group 块内），单行错误锚点得 0（多行陷阱成立）；CompileOptions `"input-ir-stage"` 恰 1（`:742`），裸名 2（陷阱 (c)） |
| 运行时指纹 | 暂存 compile **1**（`:80` 顶层）/ 暂存 opt **2**（`:258`,`:517`）/ P1 opt **0** |
| 作用域套件 | `23 PASS / 1 FAIL / 0 BLOCKED / 0 NOT_RUN`（`work/p2a-acceptance/suite-summary.json`，utc `15:28:29Z`）—— 与 §3 范围裁定一致，**A1/A10–A15/C1–C5,C7+/D1–D11 全部 NOT_RUN 且非 PASS** |
| golden 里 `UB_SIZE` 与 `dyn_shared_memory_buf` 并存 | 判别式 dump 中二者**同时出现且各自正确**（`UB_SIZE 2031616 : i32` 属 target_system_spec；`dyn_shared_memory_buf = 196608` 属 entry）⇒ goal §4.2 line 161 的混淆在当前记录里**未发生** |

### 2. 真实缺陷：**B2 FAIL**，根因是 V7 未实现 `cache4` 合取项（Implementor 侧）

我复算了 B2 的日志（`verify/logs/suite/B2.stderr`，48 行）：

- `:2` 是 `IR Dump After … LateInputVerifierPass` —— 该 pass **跑完且未失败**；
- `:25` 的 store 是 `outs(%subview_1 …)`，**完全没有** `l2_cache_mode` 属性；
- 全文件**没有任何** `Late input validation` 行；**唯一** `[ERROR]` 是 `:48` 的 `:544` stub。

对照 B4（同类但形状缺陷）：`:2`/`:3` 有两条真实诊断
`Late input validation V6 (geometry/sync) FAILED: …` 与 `Late input validation V7 (store cache/DMA subset) FAILED: unsupported DMA layout …`，
`:52` 为 `[ERROR] Late mode: input validation failed`。**故 V7 确实存在且会触发** —— 它只覆盖 DMA 形状那一半。

**并已定位到源码层面的刻意决定**（`regbase/LateInputVerifier.cpp:854-858`）：

```
// V7 is the narrower Late-mode DMA-shape gate and deliberately DOES NOT
// require the value 4 (an explicit 0 and an absent carrier are both legal
// inputs on this path).
```

这**违反已接受的设计** `p2-architecture.md` §2.4（V7 的断言原文即 "`l2_cache_mode = 4 : i32` on the
entry's single store"），也违反 goal §6.2（"cache4 未被 cache0 取代"）与 D025/D031 的整条谱系 ——
那条谱系存在的全部理由就是 cache-4 载体是**载荷性**属性。`StoreOp` verifier 的合法值集是 `{0,1,2,4}`
（**0 在其中**），故"op verifier 已挡住"不成立：它挡不住"降到 0"。

**且该偏离未被上报**：`impl/P2A-REPORT.md` 对 V7 只有一行"V1–V7 checks"，全文未出现该决定。
即：偏离写在代码注释里，**不是**写给我的；是冻结测试集抓到的，不是自查报的。

### 3. B1 的 PASS 是**假绿**（Verifier 主动披露，未改 oracle —— 处置正确）

`verify/logs/suite/B1.stderr` 我逐行读过：`.rc=1`，`l2_cache` 出现在 `:25`，而该行是
`--mlir-print-ir-after-all` 的 **IR dump 正文**（`{l2_cache_mode = 0 : i32}`），**不是**校验器诊断；
全文件 `Late input validation` 行数为 0，唯一 `[ERROR]` 仍是 `:48` 的 stub。故 B1 记录的 PASS
**即使 V7 从未运行也会 PASS** —— 与 P1 的 `B8.CACHE4` 属同一类 oracle 弱点：key 搜索可被 dump 正文满足。

**处置**：Verifier **没有**事后编辑 sealed 断言（那正是被禁止的"失败后加强 oracle"），而是把它作为
生产侧发现上报。**我裁定：B1 的冻结裁定不作为任何验收证据**，且**不得**为了修它去动 sealed 测试集；
修复后必须用**判别性补充探针**证明捕获是语义的（见下 §5）。

### 4. 我自己的文档是这次偏离的**促成条件**，已修正

§2.4 原句"V7 is the *narrower* Late-mode gate"中的 "narrower" 可被读成"范围更窄 ⇒ 只管 DMA 形状"。
**这是我文档的歧义，不是 Implementor 的凭空发明。** 已在 §2.4 就地补一段显式消歧：V7 有**两个**合取项
（载体存在且**恰为 4** 且类型 `i32`；DMA 形状），"narrower" 仅指比 `StoreOp` verifier 多挡形状，
**不**指只挡形状，并写明 `verifyStoreCacheMode` 同样不提供该合取项。

### 5. 裁定与授权

- **P2-A = 不接受**（NOT ACCEPTED）。理由：V7 缺失载荷性合取项，导致 goal §6.2 的 cache4 证据链在 P2-A 这一环无门可守。
- **授权定向修复第 1 次**（CLAUDE.md：同一根因最多两次定向修复后必须由 Challenger 独立复核）。
- **不得**以"B2 是作用域产物"为由豁免 —— 已核：同批 B4–B11 拿到了真实诊断，证明 preflight 跑过且有机会触发，B2 是"错因拒绝"而非"rc≠0"。
- **不得**修改任何测试/夹具/sealed 套件；**不得**触碰 P1 保留件、D031 的 compiler pin、以及 `build/bin` 的 P1 身份。
- 修复通过后由 Verifier 重新绑定**新**构建的身份并重跑作用域套件；被取代的 `2bf961bf…` 须在身份清单中**留痕**（不得静默替换）。
- 若 Implementor 仍不同意该方向，**回给我裁决**，不得自行按原读法再次实现。
- 若修复第 1 次仍 FAIL，**立即唤醒 Challenger**（属"focused hard failure"），不再自行尝试第 2 次。

### 12. 修复进行中 + 我自己的指令歧义已更正（Lead，2026-10-13T15:50Z）

**修复已开工**：`bishengir/lib/Tools/bishengir-compile/regbase/LateInputVerifier.cpp` 工作树为 `M`（mtime `23:43:53`），新合取项文本已在（`:861` "carry l2_cache_mode, its value must be exactly 4, and its type must be i32"；`:890` "but the Late contract requires exactly 4"），而原先把该行为写成"刻意不要求"的注释已不再匹配。

**我自己的指令歧义（同类问题的第 N 次，记下来）**：修复简报里我写了 "no touching … `build/bin`'s restored P1 identity"，
但**修复本身必然要重构建**，因此按字面读会自相矛盾。已即时更正为"**不得让 `build/bin` 停留在未钉定状态**"，
并明确重复 `p2-architecture.md` §3.3 的 G1–G4（该规则本身就写了"若 P2 构建被重跑，步骤 3–4 连同守卫一并重复"）：

- G1 显式重构建**两个**目标（`bishengir-opt` 与 `bishengir-compile` 是不同 link 边，不得指望 Ninja 自己发现）；
- G3 校验暂存包哈希 **且** `bishengir-opt --help` 仍列出 `--hivm-verify-late-input`；
- G4 冻结新的完整 `bin/`+`lib/` 暂存包；
- **受守卫的还原**须以 **G2 回拨 mtime** 的方式重做：`build/bin/bishengir-compile` → `c6838fc0…`、
  `build/bin/bishengir-opt` → `0761c6e6…`，并留存守卫产物。

**为何这是载荷性的而非仪式性的**：`c6838fc0…` 正是 **A2 的普通路径字节同一性回归**所比对的 `baseline_pre_p2_compiler`，
且 Verifier 在本次验收中刚复验过该钉。它一旦漂移，A2 那条本来干净的结果就作废。

**另有一点交办**：若修复**重生成**了 `meta_op.aiv.c310.O2.bc`，新值应记为 P2 自身身份而**不得**顶掉 P1 钉那一行 ——
驱动层改动本不应重生成模板，故若真的重生成，此事本身要在报告中单列一行。`docs/p2-binary-abi.md` §0
已开设"被取代身份须留痕"表，`2bf961bf…` 为第一行，后继项待 Implementor 报告后填入。

**Verifier 侧**：已于 `15:41` 转 idle，其三条消息与我的裁定在途中交错（不影响裁定）。已请其**待命**，
并重申三条承重项：B2 翻成带点名 V7 诊断的 PASS；撤掉 `--mlir-print-ir-after-all` 的 dump-抑制探针
（`b01`/`b02`）以证明捕获是语义的；B4 诊断措辞不变且 V6 仍触发。

### 13. 复验计划补丁：**冻结套件里没有正例能发现"V7 开始全拒"**（Lead，2026-10-13T15:5xZ）

我在给 Verifier 的复验计划里漏了一条，**这是我自己的疏漏，已即时补上并记为我的遗漏**：

**修复后必须重跑判别式，确认 V1–V7 仍然"接受" golden Late 输入。**

理由是结构性的：**B 行全是负例**，而唯一的冻结正例 A1（Late 正向编译期望 rc=0/ELF）因 P2-A 在 `:544` 刻意封闭而为 NOT_RUN。
因此修复之后，**冻结套件里没有任何一条能察觉"V7 变成了见谁拒谁"** —— 而这种修复会把 B2 与 B1 一起翻绿，看起来像干净的修复。
判别式是唯一能区分"V7 现在强制了载体合取项"与"V7 现在连合法输入也拒"的仪器，而它是我下令的探针、不是冻结行，
所以第二轮最容易漏掉。

修后须在报告中并列两半：

| 输入 | 必须结果 |
|---|---|
| `vadd_late_p1v2.mlir`（golden，含 `l2_cache_mode = 4 : i32`） | preflight **接受** —— PassID + After dump 头部非 `Failed` + 无校验器行；rc=1 只来自 `:544` |
| `b01_cache0.mlir`（值 0，撤 dump） | 被**点名的 V7 诊断**拒绝 |
| `b02_cache_carrier_deleted.mlir`（缺失，撤 dump） | 被**点名的 V7 诊断**拒绝 |

**若 golden 输入开始被拒，那是修复的 FAIL，不是更严格的 PASS** —— 必须照实报，不得报成 B2 转绿。
"靠打破正例来抓住负例"正是这个配对要防的失效模式。

### 14. 修复的暂存与提交纪律（Lead 裁定，2026-10-13T16:0xZ）

Verifier 提出了一个不该由它自己决定的问题：修复重暂存时，旧的 `2bf961bf…` 包是**留盘**还是**原地替换**。
我裁定如下，并已同时告知两侧以免分叉：

**（1）新开目录 `impl/stage-p2a-r1/`，旧目录 `impl/stage-p2a/` 逐字节原样保留（含其 `SHA256SUMS.txt`）。**
理由：Verifier 的绑定与报告针对的就是 `2bf961bf…`，且 D034 正是把该身份记作"产生 B2 FAIL 的那个"；
原地覆盖会让我的裁定引用到**已不存在的字节**。故无需快照，两侧并列引用即可。

**（2）修复是 follow-up commit，绝不 amend。** `ddc212e299f4493cc1ed8fa8a24cb6c9408e42b6` 是**经评审并已被否决**的状态，
不是草稿：它已在身份清单里，且有验收报告引用它。重写会让那个 sha 变成孤儿。
故在同一条本地分支上追加第二个 commit（message 写明是 D034 修复第 1 次：V7 载体合取项），**仍不 push、仍无 upstream**。

**一般化规则（本项目已有此规则，此处显式写到"修复/提交"这一层）**：**被取代的身份一律保留、不做垃圾回收** ——
与 P1 钉存活的原因同一条。这正是 `p2-binary-abi.md` §0 新表所记的东西，只是从"文件"扩展到"提交与目录"。

另：Verifier 已把复验清单持久化到 `…/memory/p2a-not-accepted-reverify-plan.md`（跨上下文丢失仍存活），
并主动承诺"若 B2 仍失败、或通过的理由不是点名的 V7 行，则再次 NOT ACCEPTED，**不会拿它与其余 23 条平均**"。
该纪律我明确背书：23/24 的通过计数**不是**裁定。若第 1 次修复失败，唤醒 Challenger 是**我的**决定 —— 它只报告失败并停下，
不得自行软化、不得自行开始提修复方案。

### 15. 修复第 1 次落地与 Lead 的独立初核（2026-10-13T23:5x +08 / 15:5xZ）

实现者在 `feat/tilelang-hivm-late-pipeline` 上追加 **`cfaa59677a8b63587140ab271d56db9e22c593ab`**
（"P2-A repair 1 (D034): V7 must enforce the cache-4 carrier conjunct"），单文件 `+45/−9`，
**follow-up，未 amend** `ddc212e2…`，仍无 upstream、未 push。构建是增量的：只重编 `LateInputVerifier.cpp.o`
与两个可执行文件的链接，**没有任何 `.inc` 被重生成** —— 与"仅改 `.cpp`"的说法一致。

**新身份（Lead 亲自从正确 base 复算，仅供定位；验收以 Verifier 的测量为准）：**

| 角色 | SHA-256 | 状态 |
|---|---|---|
| `stage-p2a-r1/bin/bishengir-compile` | `1a3f20f80d73908ce0b081608bb2af7132254261fd15f5d213e939d206df7c36` | 新的活动身份 |
| `stage-p2a-r1/bin/bishengir-opt` | `81bc93d43f37db34f33ae44f3b67ba71e9efd6bfb8a979da3715439a2d9c8b03` | 同上 |
| `lib/meta_op.aiv.c310.O2.bc` | `5e7ccae2…`（**未变**） | 仍为唯一被绑定的 bitcode |
| `lib/meta_op.aiv.c310.O0.bc` | `0a34be87…`（**未变**） | 仍不绑定 |

**这一条把一个先前悬着的问题就此关闭**：`p2-binary-abi.md` §0 曾写"若修复重生成 O2 `5e7ccae2…`，
则新值须记作 P2 自己的身份、且不得挤掉 P1 钉的那一行"。**实测未重生成**（两个模板与旧包逐字节相同），
故该分支未触发；O2 的"P1 历史钉"与"P2-A-r1 活动身份"仍是同一串字节，记录无需分叉。

**Lead 的独立初核（性质：定位与去风险，非验收；验收证据归 Verifier）：**

- 两个包**各自以自身为 base** 校验均 **4/4 OK**（`cd impl/stage-p2a-r1 && sha256sum -c SHA256SUMS.txt`；
  旧包同法）。旧包另加 `SUPERSEDED.md`，说明它**不是**记录物，且其清单对这四个文件仍内部自洽。
- `build/bin` 已回到 `c6838fc0a175ec6bb05214780a2a3e99ab478285620996dd5bea4c69f26aabe9`（mtime `13:46:21`）
  与 `0761c6e662ee015e30425ee29e0c2314753aa356b21eb4119ff99747327a1977`（mtime `14:46:23`）——
  **mtime 就是原来那两个**，说明 G2 的回溯生效、Ninja 不会把它当作"当前"。A2 依赖该基线钉。
- 容器 `s00653124_tilelang` 内**无构建进程在跑**，串行化规则未被破坏。

**实现者自报的运行（`impl/scratch/r1-probes/`）只作定位，不作证据。** 其内容为：
`b01_cache0`（值 0）与 `b02_cache_carrier_deleted`（缺失）在**撤掉 `--mlir-print-ir-after-all`** 的条件下
均 rc=1，且 stderr 是被**点名的 V7 诊断**（"l2_cache_mode = 0 but the Late contract requires exactly 4"、
"the entry's single store carries no l2_cache_mode attribute"）。撤 dump 后仍能匹配，说明拒绝是语义的。
但这是**实现者自跑**，按本项目规则**不能**充当验收证据；Verifier 必须以自己的脚本重跑。

**复验已派发**，绑定 `impl/stage-p2a-r1/`，清单九条（B2 转绿为验收承载行、撤 dump 探针、
**判别器必须仍接受 golden** 这一正例半边、B4 措辞、三个生成面 + `--help` 指纹、A2 基线钉、
NOT_RUN 保持、B1 oracle 弱点记为长期限制、新清单自述 base）。日志写入 `verify/work/p2a-acceptance-r1/`，
使第一次（被否决的）运行保持可重建。

**并发中的设计复核 —— 以及我在本次自查里先写错、随后更正的一件事。** 我在 Gate 3 关闭后改过
`p2-architecture.md`，故请 Challenger 做一次**窄范围**确认（V7 载体合取项是否是正确的契约；修复简报是否过/欠规定）。
给它的**第一版简报把前提写错了**：我说"§2.4 是 APPROVE 之后唯一落地的改动"。实测不成立 ——
以 `git log --date-order 6571851..HEAD -- docs/p2-architecture.md` 计，APPROVE 记录（`6571851`，21:43）之后
共有 **8 个 commit** 触碰该文件；其中 3 个（`f7fc1e7`/`a54c907`/`094c730`，21:48–21:50）已由
**`1e6e686`"record the post-edit re-affirmation of the gate-3 APPROVE"** 覆盖，而**其余 5 个没有再次确认的记录**
（`3349809`/`236c295`/`b258664`/`e4d7f1c`/`d6b02cf`，22:30–23:17；`git diff --stat 1e6e686 -- docs/p2-architecture.md`
→ **148 插入 / 17 删除**）。我已发出更正，并把问题拆成 (1a) 那 5 个 commit 是否改动过已批准的要求（我的判断：
是"已批准信封之内"的证据与勘误记录，因为它们**收窄**论断、改正数字），与 (1b) 我的 §2.4 消歧（32 插入 / 1 删除，
六个改动里**唯一**触及契约的那个）。

**门禁状态：验收在 Challenger 的设计确认回来之前不关闭。** 即便 B2 按点名的 V7 行转绿，
P2-A 的接受也是"**以设计承认为条件**"的，不得先行宣告通过。
P2-B 未开始；除 P2-A 验收子集外**每一行执行证据仍是 NOT_RUN**；未 push；`hivm-poc-evidence/` 仍为本地独占。

### 16. 自查订正：第一次验收运行**没有正例行**，以及为什么 B2 的转绿是承载行的原因（Lead，2026-10-13T16:2xZ）

等待复验期间我逐行读了冻结的跑测器与第一次运行的 `suite-summary.json`，发现**我自己的计划里有一处定性错误**，
并顺带把 B1 的"假绿"机制钉死。三件事，全部是我从盘上直接复算的：

**（1）"套件里没有正例"是错的；真实情况是第一次运行的 `--only` 选择把正例排除在外了。**
`tests/scripts/run_p2_suite.py:507` 的 `t_a1` **就是一个真正的正例**：它把 golden `vadd_late_p1v2.mlir`
经绑定的 `p2_compiler` 编到 Late 目标，要求 **`rc == 0`**、产出 `.o`、是 **ELF**、且 **machine == `0x1029`**，
否则 FAIL。**全线拒绝的"修复"会让 A1 FAIL。**
但第一次运行的 `--only` 选择是 `A2,A3–A9,B1–B11,B12a–d,C6` —— **24 行，A1 不在其中**
（`suite-summary.json` 的 `counts = {PASS:23, FAIL:1, BLOCKED:0, NOT_RUN:0}`，行数 24，可核对）。
**故那次运行一个正例都没有。** 我的计划写的是"A1 是 NOT_RUN"，方向对（确实没有正例跑过），
但机制说错了：不是"被选中后跳过"，而是"**根本没被选中**"；`need_late("A1")` 只是"若被选中会返回 NOT_RUN"的那道闸，
而验收时绑定已解析、A1 本来是可以跑的。**复验已订正：把 A1 加进 `--only` 选择。** 冻结 oracle 自带的正例，
证据力高于我自己临时造的判别器；判别器退居第二件仪器（它另外给出 PassID 与 Before/After dump）。

**（2）B1 的假绿机制，精确版。** 第一次运行的明细是 `B1 -> PASS | caught by validator: rc=1, names 'l2_cache'`。
问题在于 **V7 消息的尾部本身就含字符串 `l2_cache_mode`**：
"The store's `l2_cache_mode` carrier may only be lowered on the D012-approved conservative DMA shape." ——
于是 `l2_cache` 这个键**任何**形状失败都会匹配上，与载体取值毫无关系。
所以 B1 的 PASS 从来没区分过"取值闸触发"与"形状闸触发"，它对取值合取项是否存在**零信息量**。
这就是 `p2-verification.md` §0.7 背后的确切机制，原文照录。

**（3）套件是 V7 两个合取项的"外科式最小对"分解 —— 这正是 B2 的转绿足以承载验收的原因。**
以 `diff` 对 golden 实测：
- golden ↔ `b01_cache0`：**整份只有一行不同，且行内只差一个字符** —— `4` → `0`（两者皆 18659 B）。隔离**取值**合取项。
- golden ↔ `b02_cache_carrier_deleted`：**整份只有一行不同** —— 属性被删，18659 → 18633 B，**差 26 字节**，
  恰好是 ` {l2_cache_mode = 4 : i32}`。隔离**存在性**合取项。
- golden ↔ `b04_unaligned_store_cache4`：载体**逐字节相同且正确**；差异在 `:139`/`:142`，
  `131072` → `131076`（即 UB 指针偏移非 32 字节对齐）。隔离**形状**合取项。

于是：只补取值检查的修复能过 b01 但**仍会** FAIL B2；只补存在性检查的修复能过 b02 但**仍会** FAIL B1；
而 b04 把形状半边钉死，修复不可能悄悄改坏它。**这就是"不需要、也不允许做平均"的理由**，已同时告知 Verifier。

**附带记录（非问题，避免读者踩坑）**：golden 位于 `tests/negatives/`，而 `tests/positive/` 是空目录。
跑测器**不按目录 glob** —— 每一行都显式点名自己的 fixture（`_expect_catch(work, log, "B1", "b01_cache0.mlir", …)`）——
所以 `negatives` 只是**标签**而非**分类**，该放置不会把正例误判为"应当被捕获"。
**无需处理，也不得移动或改名**（fixture 冻结在 sha `d1023ac1fdfd22c8d7f2aed5c71241fdd83039744b383e43b19255d10a31f36b`）。

### 17. 事件：复验运行**就地覆盖**了第一次运行的行级日志（Lead，2026-10-13T16:0xZ；部分是我的指令缺陷）

复验进行中我复查盘面，发现 `verify/logs/suite/` 被**就地覆盖**：`B1.stderr`、`B2.stderr`、`B4.stderr`
的 mtime 全变成 `23:55`，且 `B2.stderr` 第 2 行现在**已经是修复后的** V7 诊断
（"the entry's single store carries no l2_cache_mode attribute"）。
**而 D034 引用的正是该路径** —— 于是 D034 的那条引注现在指向一个**读起来像 B2 通过了**的文件。

**责任划分（我不推给 Verifier）**：跑测器把日志目录写死为 `run_p2_suite.py:1725 log = VERIFY / "logs" / "suite"`，
再跑一次必然就地覆盖；而我在指令里只说了"日志写到 `p2a-acceptance-r1/`"，
**没有**考虑到**行级**日志走的是这条写死路径 —— 这是我的指令缺陷，与 §13 那次同源（都是我只想了结果目录、没想清写死路径）。

**损失边界（已核）**：第一次运行的**汇总**存活 —— `verify/work/p2a-acceptance/suite-summary.json`（`23:37`，24 行、`counts={PASS:23,FAIL:1,BLOCKED:0,NOT_RUN:0}`）在案；
**行级 stderr 未归档**，全树只有一份 `B2.stderr`，即覆盖后的那一份。
但**判定的内容并未丢失**：D034、§0.6/§0.7 与本文档均**逐字引录**了修复前的形态（无校验器行、只有 `:544` 桩），
且修复前的 golden 判别器证据另存于 `verify/work/p2a-acceptance/discriminator/`（`23:33`）。

**恢复方案（已派发，且正是"保留被取代身份"这条规则的用处）**：`impl/stage-p2a/bin/bishengir-compile`
**仍逐字节停在 `2bf961bf…`**，故请 Verifier 在 r1 运行**结束之后**（不得并发），
用**被取代的**编译器重跑 `b01`/`b02`，输出到 `verify/work/p2a-acceptance/pre-repair-reconstruction/`，
并在文件与报告中写明这是"**由保留的被取代二进制重建**"、**不是**第一次运行的原始文件。
这既修复了引注，又顺带**再次独立证明**被取代身份仍能复现该 FAIL。
程序性改正：**此后行级日志一律留在该次运行自己的目录**（运行结束把 `logs/suite/` 另存，或改为按次传日志目录），
使"后一次运行覆盖前一次证据"不再可能。

### 18. 修复新增的诊断文本会**扩大旧键的匹配面** —— 已派发去同因审计（Lead，2026-10-13T16:0xZ）

同一次复查还发现一件**由修复本身引入**的东西：`A9.stderr` 现在含**两条**校验器行 ——
V2（"the module has no hacc.target attribute"）**与** V7（"…carries no l2_cache_mode attribute"）。
即 `a03`–`a09` 这一组 fixture **本身就不带载体**（`l2_cache_mode` 缺失），修复前 V7 对它们保持沉默，修复后开始发声。

- **这一现象本身不是缺陷**：A9 仍 PASS，因为 `hacc.target` 仍命中；而 `_expect_catch`（`:955`）**只要求命中任一键、并不要求排他**，
  故多出来的 V7 行不影响 A 组判定。但**必须由 r1 汇总确认**，不能由我的推理代替。

- **真正的风险是同因（wrong-reason）通过**：`_expect_catch` 恒以 `blob = err + "\n" + out` 匹配，
  且**永远带 `--mlir-print-ir-after-all`**（`:968` 的固定 argv），所以 dump 文本始终在 blob 里。
  修复新引入的 token 有 `l2_cache_mode`、`carrier`、`cache-4`、`i32`；
  于是任何键表含 `cache`/`l2_cache`/`cache_mode`/`carrier` 的行，都可能被一条**它本来不该被**命中的 V7 行满足。
  落在风险面上的行是 **B1、B2、B3**，已要求 Verifier **逐行报告命中的是哪一条**而不仅报 PASS：
  - **B3**（`b03_cache_bad7.mlir`，值 7）最关键：它本意是考 **D012 `StoreOp` 校验器的解析期取值闸**。
    若其拒绝现在来自 V7，则 B3 **已悄悄不再考它该考的东西**。预期不受影响（解析期先失败，第一次运行记为 `Failed to parse input file`），但须由运行确认。
  - **B1** 必须由 V7 的**取值**诊断命中，而不是被形状失败的尾串命中 —— 尾串本身含 `l2_cache_mode`，这是个陷阱。
  - **B2** 必须由**缺失载体**诊断命中。

**裁定口径不变**：某一行若因"它本不该捕获的那行"而转绿，那是**同因通过**，按 D034 **不构成验收**。

### 19. 修复第 1 次的复验结果（Lead 独立复核；2026-10-13T16:01:58Z Verifier 报告）

Verifier 报告在 `verify/work/p2a-acceptance-r1/ACCEPTANCE-r1.txt`（`16:01:58Z`）。r1 套件于 `15:55:28Z` 完成：
**24 行全 PASS，B2 由 FAIL 翻为 PASS**。以下是**我自己**从盘上逐项复算的结果（不采信报告结论）：

| 检查项 | 我的独立测量 | 结论 |
|---|---|---|
| **B2 承载行** | `verify/logs/suite/B2.stderr:2` = `Late input validation V7 (store cache/DMA subset) FAILED: the entry's single store carries no l2_cache_mode attribute` | ✅ 由**缺失载体**诊断命中，非形状尾串 |
| **B1** | `B1.stderr:2` = `…FAILED: l2_cache_mode = 0 but the Late contract requires exactly 4` | ✅ 由**取值**诊断命中（不再靠 dump 正文） |
| **B3 未被改动** | `B3.stderr` = `'hivm.hir.store' op invalid store l2_cache_mode 7; valid store modes are 0, 1, 2, 4` + `Failed to parse input file` at `:189:5` | ✅ 仍是 **D012 `StoreOp` 解析期**闸门，未漂移到 V7 |
| **B4 措辞** | `b4-compare/first-V6.txt` vs `r1-V6.txt`、`first-V7.txt` vs `r1-V7.txt` —— `diff` 均 **IDENTICAL** | ✅ 形状半边未被扰动（但见下方待办 2） |
| **撤 dump 探针** | `probes/b01.stderr`（708 B）、`b02.stderr`（894 B），`l2_cache` 仅出现在校验器行 | ✅ 语义拒绝 |
| **判别器（正例半边）** | `discriminator/stdout` 41 B、`stderr` 10408 B；golden 仍被接受 | ✅ 非全线拒绝 |
| **两个 manifest** | 各自以自身为 base 均 **4/4 OK**；旧包有 `SUPERSEDED.md` | ✅ 无陈旧清单 |
| **`build/bin` 钉** | `c6838fc0…`（mtime `13:46:21`）、`0761c6e6…`（mtime `14:46:23`） | ✅ G1–G4 生效，A2 基线钉完好 |
| **容器** | 无构建进程在跑 | ✅ 串行化未被破坏 |

**§16 的订正 —— 我错了一处，且这一处会把 FAIL 误读成回归。**
§16 我写道"第一次验收运行没有正例行"，并据此**要求把 `A1` 加进选择**。**该指令是错的，已撤回。**
`regbase/LatePipeline.cpp:544` 在校验通过之后**无条件**打印
"[ERROR] Late mode: the Late pass sequence and the bitcode identity gate are not implemented in this build (P2-A
entry only). Refusing to emit an artifact. No output was written." 并 **`return EXIT_FAILURE;`**。
故 golden 在 P2-A **永远**是 rc=1 且无产物；而 `A1`（`run_p2_suite.py:507`）要求 **`rc == 0`** + `.o` + ELF + machine `0x1029`。
**所以 A1 在 P2-A 按构造不可能通过**，把它加进去只会得到一个新 FAIL 并被读成回归；
`need_late` 也拦不住它（绑定已解析，它会**真跑**）。**第一次运行排除 A1 是正确判断。**
**P2-A 的正例就是判别器**（golden 被校验器接受），正是 Verifier 跑的那件仪器。
**A1 在 P2-C/P2-D 尾段落地后成为必需行、且预期 PASS**；在 P2-A 若有人不加 `--only` 跑全套，A1 的 FAIL 属**设计内**，不得记为缺陷。

**我的失误模式（第二次自查纠错，值得记下）**：我先从**跑测器代码**推出"A1 是正例、被漏掉了"，
却**没有先核对这一行的可运行前提**（该行要求 rc==0，而 P2-A 的入口设计上就返回 FAIL）。
前一次（§15 给 Challenger 的简报）也是同型：先按印象陈述，后实测推翻。
**改正的做法：凡对"某行应当通过/应当被加入"下判断前，先核对它的前提在该阶段是否成立。**

**仍未结清（已派发给 Verifier，均为小项）**：
1. `first-V7.txt` 的**来源**必须写明 —— 它是否取自第一次运行、提前另存、还是用**被取代的** `2bf961bf…` 重建。
   （`logs/suite/B4.stderr` 已于 `23:55` 被就地覆盖，故来源不能从盘上推定；内容正确性我不怀疑，但**断言需要来源**。）
2. A 组回归的**显式陈述** —— `a03`–`a09` 本身不带载体，修复后 `A4.stderr`/`A9.stderr` 也各多出一条 V7 行；
   我判断无碍（`_expect_catch` 不要求排他，且 r1 汇总 A3–A9 全 PASS），但需写明而非留作隐含。
3. §17 的**修复前重建**（用保留的 `2bf961bf…` 重跑 b01/b02），以恢复 D034 的引注。

**门禁：P2-A 的接受仍以 Challenger 的设计确认为条件，不先宣告通过。** 证据面已经齐备且经我独立复核；
待设计确认回来后一并裁定。P2-B 未开始；未 push；`hivm-poc-evidence/` 仍为本地独占。

---

## 2026-10-14 第 19 轮追加（§20）：B4 的 "与首次运行逐字节相同" 这一分项，**实测无独立证据** —— 我先把结论写进记录，再回头测量，这是错的

**触发**：§19 未结清项 1 我原本写成"来源不能从盘上推定，内容正确性我不怀疑"。这是**把"大概为真"当成"已经核实"**。
我按要求去盘上实测来源，结论比原来的措辞严重：不是"来源不明"，而是**该断言目前没有可用的独立证据**。

**实测（全部取自盘上，`verify/work/p2a-acceptance-r1/b4-compare/` 与 `verify/logs/suite/`）**：

| 观测 | 值 |
|---|---|
| `b4-compare/` 四个文件的 mtime | **全部 `2026-10-13 23:59:48`**（同一次写入） |
| `first-V6.txt` vs `r1-V6.txt` | `cmp` **完全相同**（487 B，sha `3b6a835360c9f1d8…`） |
| `first-V7.txt` vs `r1-V7.txt` | `cmp` **完全相同**（496 B，sha `32666117e3afb75e…`） |
| `verify/logs/suite/B4.stderr` mtime | **`23:55:16`** = r1 那次就地覆盖的时刻（§17） |
| 首次运行的 B4 文本有无别处留存 | **无**。`verify/work/p2a-acceptance/` 下只有 `completion-utc.txt`/`suite-summary.json`/`three-greps.txt`/`discriminator/`/`help/`/`occ/`，**没有 b4-compare**；`verify/work/run/B4/` 是**空目录** |
| 全库含 V7 shape 行的文件（穷举 `grep -rl 'unsupported DMA layout'`） | `impl/p2a-commit.diff`(23:19，是源码 diff 本身)、`impl/scratch/r1-probes/b04_*.{dump,nodump}.err`(23:46，**implementor 用 r1 二进制**跑的)、`verify/logs/suite/B4.stderr`(23:55，r1)、两个 `b4-compare/*-V7`(23:59)。**没有一件早于 23:55 的首次运行产物** |
| `occ/ord-err`(23:36，首次运行期) 是否含 `first-V7.txt` 的字节 | 否，`grep -F` **不是子串**（该文件里的 `unsupported DMA layout` 不构成 V7 行） |

**结论**：`first-V6/V7.txt` 不可能是"从首次运行取出/提前另存"。23:55:16 之后盘上已无首次运行的 B4 文本，
而这两个文件是 23:59:48 写下的、与 r1 逐字节相同。它们只可能是二者之一：
(a) 从**已被覆盖**的 `logs/suite/B4.stderr` 抓的（即：把 r1 的字节标成了 "first"），或
(b) 用**被取代的** `2bf961bf…` 重跑得到后写在这里（那它其实是"重建"，且本该落在已委派的 `pre-repair-reconstruction/`，而该目录**尚不存在**）。
(a) 与 (b) 只能由 Verifier 说明，盘上无法区分。**但无论哪种，`ACCEPTANCE-r1.txt:18` 的
"B4 V6/V7 wording: byte-identical to first run" **都不是**一个由"独立的修复前见证"支撑的断言**——
(a) 是与自身比较（循环），(b) 是被取代二进制的重跑（可接受，但必须作为**重建**标注并给出该二进制身份）。

**两处细节更正**：
- 报告写 "486/495 chars"，盘上是 **487/496 B**（与换行计数口径有关）。数字本身无害，但它是**报告里一个未复核的数字**，按同一标准记下。
- **不作为**的诱惑：因为"repair 是加法的、B4 走的是 shape 连词、措辞本该不变"，所以"大概就是一样"。**这正是我在 §19 犯过的同型错误**——由代码/设计推出结论，而不先测量。

**这不改变 B2 的裁定**（承载裁定的那一行）：B2 的证据是 `23:55:16` 的 r1 日志里**具名**的 presence 连词诊断，
两个探针按构造就是 r1 期的产物。受影响的只是**报告的一个分项断言**，而正确处理是**把它标为无据**，
不是因为它"很可能为真"就让它留着。

### §20 追加（2026-10-14，同日）：我**撤错了**那个分项 —— 见证在**agent transcript** 里，我搜了"证据树"就以为搜完了

**Verifier 指出并给出了具体位置；我没有采信，而是自己去那个文件重测。结果它是对的，我错了。**

**我独立复测**（`python3 -I`，直接读 jsonl，不采信转述）：
`…/subagents/agent-averify-fd1c402be6c7e059.jsonl` **第 870 条**，`type: user`（tool_result），
`timestamp 2026-10-13T15:30:49.675Z` —— 比 `15:55:16` 的就地覆盖**早 24 分 26 秒**。
该记录体长 **17876 字符**，就是**首次运行的 B4 stderr**：带 `LateInputVerifierPass Failed` 的 After 头，
且同时含 V6 与 V7 行。用**精确字节比较** `json.dumps(line)[1:-1] in record`：
`first-V6.txt`(487 B) 与 `first-V7.txt`(496 B) **两条都在该记录里逐字出现** ——
（原始子串判断为 False 只是因为 JSON 转义，这正是必须用 escaped 形式比较的原因）。
去空白后 486 / 495 字符，**与报告数字一致**；我先前写的 "487/496 B" 是**量错了对象**（含换行的字节数 vs 去空白的字符数）。

**结论：那个"逐字节相同"的分项断言现在站得住，且不是循环论证** —— 它有**早于覆盖**的见证。
`first-*.txt` 不是文件拷贝、也不是用被取代二进制重建，而是**转写**（15:59:48Z 写下），与该记录逐字节吻合。

**边界（我同时记下其强度上限）**：该见证是**harness 的 agent transcript**，**在证据树之外、未做哈希钉死**，
所以它见证的是**内容与时间**，不是密码学身份。比哈希钉死的产物弱，但远强于"r1 的字节与自身比较"。

**我的失误模式（第三次，同型）：** 我做了"穷举搜索"，但**把"证据树"当成了"全部可及记录"**。
transcript 也是记录的一部分。**改正的做法：撤下任何分项断言之前，搜索范围必须覆盖 agent transcript，
而不只是 `hivm-poc-evidence/`。** 已写入记忆 `p2-suite-logs-overwrite-in-place`。

**另一件把举证责任倒过来的事（比 §0.6 更强的一条）**：Verifier 用**保留的被取代** `2bf961bf…`
重跑了 b01/b02（跑前重算该二进制 SHA；manifest 从 `impl/stage-p2a/` 自身 base 仍 4/4；
fixture 重新哈希到冻结前缀），按套件调用原样复现，结果写进 `pre-repair-reconstruction/`
并**标注为重建**（绝不放进首次运行的槽位）：
- **recon-b01**：rc=1，无产物，**零**条 `Late input validation`，After 非 Failed，键**只**在 `stderr:25`
  的 IR dump 体里命中 —— **复现了 B1 的空洞 PASS 机制**。
- **recon-b02**：rc=1，无产物，**零**验证行，**任何位置都不命中键**，唯一错误是 `:544` stub ——
  **逐字复现 D034 的 B2 FAIL**（"does not name any of ['l2_cache','cache_mode','carrier'] — wrong-reason rejection"）。
**即：旧二进制仍然 B2 失败、仍然以错理由通过 B1，而新二进制两者都不再 —— 翻转可归因于修复本身，
不是 oracle 漂移。这是"被证明"而不是"被论证"。**

**另两项按证据复核（r1 运行）**：
- **A 组回归为零**：两次运行 A2–A9 全部 PASS，且每条 A 行的 detail **逐字节相同**；V7 在无载体 fixture 上的
  共触发（`A4.stderr` 多一条 V7、`A9.stderr` 多一条 carrier-absent）**无害**，因为 `_expect_catch`
  只要**一个**键、且**不断言排他**。
- **错理由审计（报"命中哪一行"而非"PASS"）**：B1 的**设计值**诊断现在真的在 `B1.stderr:2`
  （`l2_cache_mode = 0 but the contract requires exactly 4`），dump 体那行仍在 `:26`；B2 命中 `:2` 的
  carrier-absent 行；**B3 仍是解析层**（`invalid store l2_cache_mode 7` / `Failed to parse input file`，
  零验证行），D012 门仍然完好。

**保留缺陷本身的修复**：r1 的逐行日志已**原样**复制到 `verify/work/p2a-acceptance-r1/logs-suite/`
（`cp -a` + `RETENTION.txt`，runner 的固定目录不动，留给下一次运行）。

**我自己的失误（第二批）**：我把这个未测量的分项**先写进了 §0.9 和 `docs/p2-verification.md`**，
再去测量。顺序错了。§0.9 现已按实测改写：该分项标为**无独立证据、暂不成立**，
待修复前重建给出真实的前侧后再判定。

**处置**：
1. 已委派的修复前重建（§17）**扩展为同时覆盖 B4**：用保留在 `impl/stage-p2a/bin/bishengir-compile` 的
   `2bf961bf…` 重跑 `b04_unaligned_store_cache4.mlir`，产出真实的前侧 V6/V7 措辞，落到
   `verify/work/p2a-acceptance/pre-repair-reconstruction/`，并**标注为重建**（含被取代二进制的绝对路径与 SHA-256）。
2. 已请 Verifier **明确说明** `first-V6/V7.txt` 走的是 (a) 还是 (b)；若是 (a)，该文件应改名或加标注，
   避免后续读者把它当作修复前见证。
3. 在 (2)(3) 结清前，**任何文档不得再引用**"B4 措辞与首次运行逐字节相同"。
4. 三处承载裁定的证据（B2 具名诊断、两个 dump-suppressed 探针、discriminator）**均不受影响**，P2-A 的
   裁定基础不变；**门禁仍然只挂在 Challenger 的设计确认上**。

未 push；未动 build 树；`hivm-poc-evidence/` 仍为本地独占。

---

## 2026-10-14 第 19 轮收口（§21）：Challenger 设计确认返回 —— 三项全部 **APPROVE**，A1 排除被确认为**正确**；我设的唯一悬挂门禁就此关闭

**Challenger 的裁定（逐字保留，它不写 `.md`，由我落盘）**：

> 1. **Yes.** V7 must reject an absent `l2_cache_mode`, any value other than exactly `4`, and any carrier whose type is not `i32`; absent and explicit `0` are both failures. `StoreOp` accepts `{0,1,2,4}`, and lowering can turn an absent carrier into zero, so no downstream stage restores cache4. `verifyStoreCacheMode` also checks value, target, and shape, but does not enforce the required cache4 carrier.
>
> 2. **Yes.** The §2.4 paragraph clarifies the byte-unchanged approved V7 row. The five later commits correct or measure option-surface and staging evidence within the approved envelope; they do not alter the Late cut or G1–G4 semantics. No additional design review round is required.
>
> 3. The repair brief is complete and appropriately narrow. I have no remaining design objection. The new staged build and re-verification are acceptance evidence owned by the Verifier/Lead; the contract and delta review itself is approved.
>
> **APPROVE — the requested design contingency is closed.** … "Present and nonzero" would admit modes `1` and `2`, which are legal general store modes but do not satisfy this frozen VADD contract. … **Excluding A1 from P2-A is correct.** Record it as NOT_RUN because the entry-only build deliberately stops after validation. Golden preflight acceptance followed by the stub's exit `1` proves validator acceptance; it does not prove successful binary compilation. A1 remains mandatory at P2-C/P2-D. This is independent approval of the contract, document delta, and repair source. **I have not independently inspected the new r1 execution logs**, so the reported 24/24 result remains Verifier/Lead evidence for adjudicating scoped P2-A acceptance.

**我据此下的判断与动作**：

0. **一处记录用词更正（先于裁定记下）。** Challenger 在其中一封信里称 §2.4 那段为 "uncommitted §2.4 32/1 paragraph"。
   **实测：它是已提交的** —— `docs/p2-architecture.md` 的 §2.4 两连词消歧段落在提交 `6aca6d4` 中，
   `git diff HEAD -- docs/p2-architecture.md` 为空。被批准的契约文本就是仓库里的那一份，不存在
   "工作区里未提交的段落"。这不影响它的裁定（它审的正是该段），但**记录不得让后来的读者去找一份不存在的未提交文本** ——
   与我同日更正 `verifyStoreCacheMode` 标签是同一条标准：不让结论靠一个不准确的措辞站住。

1. **唯一悬挂门禁关闭。** 我在 §19 设的条件是"P2-A 的接受以 Challenger 的设计确认为条件"。三项均为 Yes，
   且它另行确认 **A1 的排除正确**（这是我在 §19/§0.9a 自我更正过的那一条 —— 现在由独立方确认，
   而不是只由我自己认定）。**注意边界**：Challenger 明确声明它**没有**独立查看 r1 的执行日志，
   所以 24/24 的**执行**证据归 Verifier/Lead；它的批准是**契约与文档 delta** 的批准。两者不混为一谈。
2. **但我不立刻写"P2-A ACCEPTED"。** Challenger 在三封信里三次点出同一件**更强的廉价仪器**：
   用**新 staged 的 `bishengir-opt`** 跑**独立** late-input validator（无 IR dump）—— 期望 golden **rc=0**、
   b01/b02 **rc!=0 且具名 V7**。它把这条与"compile 路线配对探针"并列为推荐（后者我们已做）。
   我判断值得**先跑再判**，两个理由：
   (a) 我们现有的**正例是推出来的**：discriminator 里 golden 的 rc 是 **1**（来自 `:544` stub），
       "validator 接受了 golden"是由 `PassID` + After-not-`Failed` + 零验证行**推断**的；
       独立 opt 给出一个**没有 stub 挡路的直接 rc=0**，把推断变成直测。
   (b) 它正好**兑现 §0.4**：§0.4（"测试跑在新建并 staged 的 `bishengir-opt` 上，而非 P1 钉死的二进制"）
       自冻结起一直是 **NOT_RUN**。这一跑是它唯一的真实缺口。
   **先判后翻是不必要的风险**，而这一跑很便宜（单容器作业）。故：先跑，再裁定。
   已明确要求 Verifier：**仪器无效 ≠ P2-A 失败** —— 若该 pass 无法独立调用（需要合并管线上下文、
   或缺某个选项），原样报告并停止，那是**关于仪器的事实**，不得记为任何行的 FAIL，更不得由 golden 的
   非零返回推出"validator 拒绝 golden"。CLI 拼写以**实测 `--help`** 为准，不采信转述的
   `--hivm-verify-late-input='target=Ascend950PR_9579'`。
3. **一处措辞更正已落实（Challenger 提了两次，值得当真）**：我在 §0.7 写 "`verifyStoreCacheMode` is a
   shape predicate too" —— **不准确**。它同时检查 value / target / shape。相关的**结论**不变且更窄：
   **它和 `StoreOp::verify` 都不要求存在 `l2_cache_mode = 4`**，且 `HIVMToStandard.cpp:1425-1429`
   把"属性缺失"降为常量 `0`，故下游无补救。§0.7 已按此改写并留更正说明 —— **记录不得靠一个不准确的
   标签站住**，即使结论碰巧是对的。
4. **Implementor 的主动披露，我裁定：保留，不移动。** 它在 `impl/stage-p2a/` 内加了 `SUPERSEDED.md`
   （它自己标出这是唯一可能越界之处，并主动提出可移到 `impl/`）。**裁定：留在原处。** 理由：
   该目录的**四件 artifact 字节与 mtime 未变**、`SHA256SUMS.txt` 逐字节相同（`6d1d4a06…`），
   所以 D034 引用的字节仍可解析；而"一个描述被拒身份的 4/4 manifest 不带旗标"**正是陷阱本身**
   （`sha256sum -c` 会全 OK，读者会以为它仍是记录）。一个**旗标文件不是替换**，它不改变任何被引用的字节。
   我不要求移动，因为移动会把旗标与它标注的 bundle 分开，反而更容易漏读。
   **条件**：该旗标**不得**被任何 manifest 覆盖（现状就是如此：manifest 仍指 4 件，
   `stage-p2a/` 从自身 base 仍 4/4 OK），且引用该目录时**必须同时给出 base**（同 `sha256sum -c` 的既有教训）。
5. **它的过程自陈我接受**：把一处**承重**门控的收窄写进代码注释而没有上报，是流程错误；
   且其理由在实体上也错（D012 的 `StoreOp` 合法集是 `{0,1,2,4}`，0 在那里合法，路径上没有别的东西补上这个连词）。

**门禁**：**scoped P2-A 接受的裁定 pending 上面那条独立 validator 探针**（challenger 的设计条件已关闭，
这是我为"先测后判"自加的一道小门）。**P2-B 未开始**；未 push；未动 build 树；`hivm-poc-evidence/` 本地独占。

---

## 2026-10-14 第 20 轮（§22）：独立 validator 探针已跑 —— 逐项从盘上复核后**裁定 P2-A scoped ACCEPTED（D036）**，并已派发 P2-B

### 探针：我自己的复测（不采信报告）

目录 `verify/work/p2a-acceptance-r1/standalone-validator/`。**我实测的**：

| 项 | 我测到的 |
|---|---|
| 被消费的仪器 | `impl/stage-p2a-r1/bin/bishengir-opt`，**我重新哈希** = `81bc93d43f37db34f33ae44f3b67ba71e9efd6bfb8a979da3715439a2d9c8b03` —— 是 **staged opt**，不是 compile |
| rc | golden **0**；b01 **1**；b02 **1** |
| `--mlir-print-*` | 三个 cmd 文件里**一个都没有**（`grep -l` 无命中） |
| golden | `stderr` **0 字节**；`Late input validation` 计数 **0**；`note: see current operation` 计数 **0** |
| b01 | `stderr:1` = 具名 V7 **值**诊断（`l2_cache_mode = 0 but the Late contract requires exactly 4`） |
| b02 | `stderr:1` = 具名 V7 **载体缺失**诊断（`carries no l2_cache_mode attribute`） |
| 调用方式 | 走 `scripts/in-container.sh`（host 二进制不可执行，这点没被绕过） |

### 唯一可能把它变成"又一次 dump 文本假绿"的东西，我专门查了 —— 它是**失败诱发的**

b01/b02 的 stderr 里确实带 MLIR 的 `note: see current operation` 操作转储。关键在于它是**因果下游**：
它在 b01 里位于**第 4 行**、**紧跟第 1 行的 error 诊断之后**（是错误诊断的附注，不是 `--mlir-print-*` 请求的 dump）；
而**通过的 golden 的 stderr 是 0 字节，一条 note 都没有**。即 **note 只在 validator 失败时出现** ——
它既不可能把"实际通过"的输入伪装成拒绝，也不可能独立于失败存在。
所以这条探针是干净的：**正例的 rc=0 是构造性的**，工具里根本没有 `:544` stub 挡路。
原先"validator 接受了 golden"是从 `PassID` + After-not-`Failed` **推断**的，现在是**直测**。

### 裁定：**P2-A scoped ACCEPTED**（D036；完整裁决在 `docs/decisions.md`）

- **§0.4 的严格形式就此兑现**（"跑在新建并 staged 的 `bishengir-opt` 上，而非 P1 钉死的二进制"）：
  自冻结起一直 NOT_RUN，现在有了一次**真实执行**。**边界照旧**：A10–A12 仍 **NOT_RUN**，
  它们需要一个**能编译的正例**，这次探针不提供。
- 承载裁定的仍是 **B2**（具名 V7 诊断）+ 两个 dump-suppressed 探针 + discriminator + 修复前重建 ——
  后者把"翻转归因于修复"从论证变成**证明**：旧二进制仍 B2 失败、仍以错理由通过 B1。
- **B4 分项恢复**：不是我"补"的，而是**我先撤错了** —— 见证在 agent transcript 里（我搜了证据树就以为搜完了，
  同日我自己重测确认：record 870，`15:30:49.675Z`，`first-V6/V7.txt` 逐字命中）。
- **未被豁免的既有局限随裁定一起记**：**B1 的冻结判定不承载任何接受结论**（oracle 分不清"设计值行"与
  "dump 体行"，是**生产层修复 + dump-suppressed 探针**让 B1 重新有意义）；`_expect_catch` 对将来任何
  "键与 dump 文本撞车"的负例仍可能假绿。
- **明确没有声称的东西**：**没有二进制**（A1 在 P2-A 设计上 NOT_RUN，P2-C/P2-D **必须**且**预期** PASS）；
  A10–A12 与全部 C/D 行仍 NOT_RUN 并保留各自范围理由；**P2 不要求真实 NPU 数值正确性**（那是 P3 的 ACL Harness）。

### 下一步已执行：**P2-B 已派发**（goal §8 步骤 5）

派发要点（均引自本地 `docs/p2-goal-1013.md` §5，不凭记忆）：保守合法化尾；必须保留的 pass；
**禁止零次**清单（HFusion 调度/向量化、Bufferization/PlanMemory、GraphSyncSolver/InjectSync、自动 MultiBuffer、
自动 layout/schedule 重规划、broad AVE 优化、`buildLowerAVEPipelines`、重复 LLVM lowering）；
`ConvertHIVMToLLVM` **恰好一次**且唯一所有者是 HIVMC A5（§6.3 第 11 项 = C11）；
**用真实 MLIR instrumentation 而不是源码 grep 计数**；保护不变量清单；**终止 PIPE_ALL 必须入口作用域**；
"不要凭名字判断 SyncBlockLock finalization 可删"。另加两条我要求**明确回答**的：
(1) P2-B 结束时入口**到底产出什么**（IR？产物？仍在哪一行停住？）—— A1 不属于 P2-B，不得为凑 A1 造二进制；
(2) 若实际 IR 要求保留偏好清单之外的 pass，报出是哪一个、IR 的哪一点要求它。
暂存纪律照 P2-A：新目录、**follow-up 不 amend**、G1–G4 重做（含回写 mtime）、`build/bin` 复原到
`c6838fc0…`/`0761c6e6…` 并重跑 I1–I7（A2 的回归就比对这个值，所以复原是承重的）。

**下一步**：Implementor 实施 P2-B → Verifier 验收（步骤 5）。未 push；`hivm-poc-evidence/` 本地独占。

---

## 2026-10-14 第 21 轮（§23）：P2-A 的构建身份行已按实测填齐；P2-B 验收简报已派发；顺手跑掉覆盖缺口 4 的两个廉价检查，其中**第二个的判据字面写错了**

### 1. `docs/p2-binary-abi.md`：A2/A3/A5/A6 已填（commit `bdd775b`），并收窄了表头那句笼统的 NOT_RUN

表头原写 "no P2 compiler and no P2 binary exists yet"。写的时候是真的，**P2-A 构建之后就不再真**，
留着会让四条 A 行看起来"没得跑"，而它们其实跑过。现在改成显式边界：A2/A3/A5/A6 **已执行并记录**；
A1、A4、全部 B、全部 C、全部 D **仍 NOT_RUN**（它们要真二进制，而 P2-A 按设计不产）。
**A2 的值是 P2-A 范围内的，会被 P2-B 重建取代，不得读成 P2 最终身份。**

我今天**从盘上重测**（不是抄文档）的：
- **A3**：两个 stage 包各**以自己为 base** 跑 `sha256sum -c` → **4/4 OK**（在别处跑会对正确的包报 FAILED，
  这是既有教训，所以每次都写清 base）。
- **A5**：D031 的带外摘要 `SNAPSHOT-MANIFEST-DIGEST.txt` 重跑 → **47/47 OK、零 FAILED、零 warning**，
  且是**在两个 P2-A 构建和受护复原之后**测的；`INTEGRITY.json` 仍 `7f9b92bf…`，摘要自身仍 `4dfbc856…`，
  两个被钉的路径（`build/bin/bishengir-opt`、`build/lib/meta_op.aiv.c310.O2.bc`）**盘上仍是钉子里的值**。
  **没有为了迁就 P2 构建去改 D031 的钉子。**
- **A6**：I1–I7 复原**前** PASS（`integrity-pre-restore.log`）、**后** PASS（`integrity-post-restore.log`、
  `p1-integrity-gate-r1.log`）；G1–G4 有记录（`r1-guard-restore.txt`，`2026-10-13T15:49:04Z`），
  G2 的"恢复产物比其链接输入更旧"是时间戳静态证明；`build/bin` 两个身份我重测回到
  `c6838fc0…` / `0761c6e6…`。**该行不声称**恢复对 Ninja 安全 —— G2 是记录在案的缓解，
  §3.3 已写明 `ninja -n` 看不见重新链接。
- 顺手改正 A2 的一处**我自己写错的尺寸**：stage 里的 compile 是 **205,434,568 B**，
  不是同名 `build/bin` 文件的 205,293,424 B。同名不同字节 —— 这正是每处引用都必须带路径的原因。

### 2. Verifier 的 **P2-B 验收简报已派发**（msg `718b9500`）

含：**准备可以现在做，执行必须等 implementor 报出 P2-B 包已 stage**（一次只对一个 build 目录）；
P2-B 解锁的是**通行轨迹/契约保持**行，**A1 与全部 D 行在 P2-B 仍 NOT_RUN**，"还没有二进制"不是 P2-B 的失败；
证据必须来自**真实 MLIR instrumentation**，源码 grep 计数一律驳回；禁止零清单、必需顺序、
`ConvertHIVMToLLVM` **恰好一次且唯一属主是 HIVMC A5**；保护不变量**逐 stage 断言**；
**终止屏障检查必须入口作用域**（`b07` 负例已在）；**不凭名字判断 `SyncBlockLock` 可删**；
**报出命中的是哪一行**，因 SHA 不符或解析错误而失败的行记 **NOT_RUN**；
`run_p2_suite.py:1725` 硬编码日志目录的**保旧**纪律；**不得改 oracle / 事后放松断言**。

### 3. 覆盖缺口 4：我跑了那两个"不需要 run"的检查 —— **第二个的字面是错的**

按 `p2-verification.md` §A 缺口 4 的记录，把判据跑在 P2-A/r1 构建生成的 `.inc` 上：

- `ConfigUtils.cpp.inc`：`inputIRStage`/`inputIRStageFlag` **0 次**、`GEN_PASS_OPTION_TO_CONFIG` **0 次** —— 与预测一致。
- `PassOptions.cpp.inc`：**记录里写的那个字面**（`Pass::Option<std::string> inputIRStage` 在同一行）**0 次**。
  **这不是预测中的证伪** —— 生成器把声明发成**两行**：`:1192 Pass::Option<std::string>` / `:1193 inputIRStage{`，
  在 `#ifdef GEN_ALL_OPTION_REGISTRATION`（`:441`–`:1676`）块内。所以正确写法是"行首的 `inputIRStage{` 且在该块内"，
  而**设计前提在实测产物上是成立的**。**按原字面执行会得到一次假证伪** —— 判据自己说 0 就是证伪。
  与 `verifyStoreCacheMode` 措辞、`sha256sum -c` 的 base 同形：**判据的效力等于它的拼写**。
  已在记录处**就地补正**（补正只说准拼写，不放松判据）。
- 顺带记下**第一条检查的证据力上限**：`ConfigUtils.cpp.inc` 的 mtime 是 **`2026-10-09 14:55:24`**，
  **早于 P2-A 构建**（`PassOptions.cpp.inc` 是 `2026-10-13 23:06`，被重生成过）。
  所以"0 次"既可能是"生成器对无 `passGroup` 的 def 本就什么都不发"（预测的原因），
  也可能是"生成器没针对当前 `Options.td` 重跑"——**单看计数分辨不出**，与 §3.3 记的模板 bitcode 14:42 写入同一形状。
  该检查是**与前提一致**，不是前提的证明；重生成两个 `.inc` 才能定案，留给 P2-B 的构建，现在不声称。

**下一步**：等 Implementor 报 P2-B 包；Verifier 届时执行验收。未 push；`hivm-poc-evidence/` 本地独占。

### 3a. （§23 附；落笔晚于 3b，编号只为排序）**预先登记**：冻结套件 A11 的 IR-effect 半段**按现在的写法不可能 PASS**，而失败原因不是实现的缺陷

我在等 P2-B 的时候把 A10/A11/A12 读了一遍，发现一个**必须在结果出来之前就定好判据**的问题。
先记录，等 P2-B 的验收数字回来时按这份预先登记的判据判，而不是看到 FAIL 再回头找理由。

**已核实的事实（本机代码 + 冻结夹具，非推测）：**

1. `t_a11`（`run_p2_suite.py:744`）的 IR-effect 断言是
   `final_dump = out + "\n" + err`，然后 `re.findall(r"hivm\.hir\.\w+", final_dump)` 必须**为空**。
   `out`/`err` 是 `_instrumented_compile`（`:632`）的输出，而它传了
   `--mlir-print-ir-before-all` 与 `--mlir-print-ir-after-all`。
2. 冻结输入 `vadd_late_p1v2.mlir`（`d1023ac1…`，已重测）**本身含 15 处 `hivm.hir.*`**
   （`pointer_cast`×4、`wait_flag`×2、`store`×2、`set_flag`×2、`load`×2、`get_block_idx`×2、`pipe_barrier`×1）。
   第一个 `before-all` dump 就是**输入本身**。
3. 所以：**只要 dump 被捕获，A11 就必然在"输入自己的 IR"上失败**（报告成 "HIVM primitives not legalized"）；
   **若 dump 没被捕获**，则 `has_cf` 为假，A11 改为以 "scheduled-but-skipped" 失败。**两条路都不可能 PASS**，
   且**两条都不是实现的缺陷**。同一缺陷形状还有一处：A10（`:703`）/B1 的 E5 扫描读的也是 dump body。

**A11 的两半要分开说，否则会把能测的部分一起丢掉：**
- **顺序与 sync-lock 家族**那半段只用 PassID 流（`parse_pass_streams`），**是可测的**——
  Implementor 自己的 `diag.txt` 已给出 33 条 PassID、`missing=[] order_ok=True`、
  sync-lock 家族计数（`…infer-sync-block-lock-num-and-init-func` 2、`…mark-sync-block-lock-with-subblock` 1、
  `…lower-create-sync-block-lock` 2、`…insert-free-lock-var-before-return` 1、`…bind-sync-block-lock-arg` **0**）。
- **IR-effect** 那半段按现写法**不可测**。

**我预先定下的处置（不是现在动手改）：** 这属于**测试作用域缺陷**，**不是**放松断言的许可。
CLAUDE.md 写着"不得在失败后改 oracle 或放松断言"，所以**正确做法是让断言去测它自称在测的东西**——
即断言落在**最终** IR（最后一个 after-all dump / tail 的输出模块）上，而不是所有 dump 的拼接。
那是**把检查变准**，不是变弱；在改之前它会一直是一个"必然假红"的检查。
按 §8 第 8 步交给 Challenger 一并裁决，与既有覆盖缺口 (2)(3) 同批。

**一并查清的机制事实**（因为"trace 只有在覆盖被证明时才算证据"，§3.2 前置 1）：
- `bishengir::applyPassManagerCLOptions`（`lib/Pass/PassManagerOptions.cpp:84-115`）**只**用
  bishengir 自己的 `--bishengir-print-ir-before/after` 逐 pass 选择器来开启 IR 打印，
  **从不**读 `--mlir-print-ir-*-all`；两个选择器都没给时直接 `return`，什么都不开。
- 它的白名单 `kAllowedPassList = {hivm-inject-sync, hivm-inject-block-sync, hivm-graph-sync-solver}`
  （`:37`）**恰好就是 FORBIDDEN_PASSES 里的那三个** —— 那套机制是给这三个 pass 排障用的，
  **不是**取得覆盖 trace 的途径。
- Late 路线经 `bishengir::runPipeline`（`Utils.cpp:75`）→ `RetriablePassManager::runOnce`，
  后者同时施加 `mlir::applyPassManagerCLOptions` 与 `bishengir::applyPassManagerCLOptions`
  （`RetriablePassManager.cpp:106-107`）。所以 `--mlir-print-ir-after-all` 到底出不出的来，
  取决于**上游 MLIR** 在这份 LLVM/MLIR 版本里的 `applyPassManagerCLOptions` 行为 ——
  Implementor 正在量这一点（它自己的 `compile.err` 只有 33 条 PassID + pass-statistics + 收尾的 P2-B stub
  `[ERROR] Late mode: the bitcode identity gate and the HIVMC A5 lowering are not implemented in this build
  (P2-B tail only). Refusing to emit an artifact.`，**0 个 `IR Dump` header**）。
  **我不在它量完之前下结论。**

**下一步**：等 Implementor 报 P2-B 包；Verifier 届时执行验收。未 push；`hivm-poc-evidence/` 本地独占。

### 3b. （§23 附）我派发时引的锚点里有一个路径是错的、几个行号已经漂了 —— 已改文档并追补更正

派发之后我复量了锚点，发现两处会**真的导致错放日志**的问题，都已就地改正并追告 implementor（msg `ae6b29f2`）：

- **路径错**：我写 `hivmc/A5/HIVMCMainA5.cpp:944`；真路径是
  **`bishengir/lib/Tools/hivmc/A5/HIVMCMainA5.cpp`**（没有 `Tools/bishengir-compile/` 这一段）。行号 944 是对的。
- **行号漂移**：`p2-pass-trace.md` §A 的锚点是在 `80e29e6f2` 读的，而 P2-A 的 `ddc212e29` 往
  `regbase/Driver.cpp` 插了 Late 分支，于是 `inferLayoutOptimization` **68→79**、`inferMixedCV` **73→84**、
  `runRegBasePipeline` 调用点 **81→92**。`Utility.cpp:100`/`:61`、`BiShengIRCompileMain.cpp:344-346`、
  `PassPipeline.cpp:564` **未变**。
- 已在 `docs/p2-pass-trace.md` 增 **§A.0**，两栏并列（`80e29e6f2` 的值 vs HEAD `cfaa59677` 的实测值），
  并把规则写成 **按函数名锚定、不按行号** —— 一个被本阶段自己编辑的文件里的行号不是稳定标识符。
  把日志放在已过期的 `Driver.cpp:81` 上会放在空处，那一行就会以**错误的理由**测出"缺席"。
- 顺带把 `docs/p2-binary-abi.md` §0 里那行**已经过期**的表述改掉：它仍写 r1 包
  "ACCEPT contingent on the Challenger's design confirmation, so not yet closed"，
  而那条设计条件已由 D035 关闭、D036 已把它定为**被接受的 P2-A 记录产物**。留着一句"未关闭"是记录缺陷。

**下一步**：等 Implementor 报 P2-B 包；Verifier 届时执行验收。未 push；`hivm-poc-evidence/` 本地独占。

### 4. （§23 附）P2-B 范围补充：通行轨迹 A1–A4 需要的**入口边界日志在 HEAD 上根本不存在**

我在派发后量了 `HEAD`（`cfaa59677`）：`bishengir/lib/Tools/bishengir-compile` 下
**没有** `regbase/Utility.cpp:61`(`inferMixedCV`) / `:100`(`inferLayoutOptimization`) 的边界日志，
**没有** `regbase/BiShengIRCompileMain.cpp:345`(`runRegBasePipeline` 函数入口) 的边界日志；
唯一的 `[P2-Late]` 行是 `LatePipeline.cpp:251` 的 pinned-option 那条。
所以 **`docs/p2-pass-trace.md` §A 的 A1/A2/A2b/A3/A4 五项目前跑不了**，
而 §A 覆盖缺口 (2)(3)（"路由用边界日志断言"、"`inferLayoutOptimization`/`inferMixedCV` 零调用"）
正是靠它们才关得上。通行轨迹按构造对这两个函数是盲的 —— 它们是只做 `module.walk`、不加 pass 的普通函数。

已把这五处日志按 §A 已定死的锚点**并入 P2-B 范围**并追加派发给 implementor（msg `d8b58bd3`），
并加了两条约束：**只加 instrumentation，不改控制流/pass 构造/输出 IR**；
**以能被区分、绝不误认为错误的形式写到 stderr 单行**。
同时要求它在报告里明确回答 **P2-B 结束时入口到底产出什么**，并再次强调
**A1 不属于 P2-B，不得为让 A1 好看而造二进制**。

**下一步**：等 Implementor 报 P2-B 包；Verifier 届时执行验收。未 push；`hivm-poc-evidence/` 本地独占。

## 2026-10-14 第 22 轮（§24）：P2-B 落地 —— 我逐项从盘上复核；**四个冻结 oracle 缺陷已实测确认**（其中两个源头是我自己的文档），裁定 P2-B = 仅 step 3，并已派发 Verifier + Challenger

Implementor 报了 P2-B 包（`impl/P2B-REPORT.md` 23,768 B，00:45；`impl/p2b-step3.diff` 61 行；
新增 `impl/stage-p2b/`；轨迹 `impl/scratch/p2b-trace/`）。**我没有采信报告的任何一行**，下面是我自己量的。

### 1. 我独立复核通过的（这些成立）

- **改动面**：一文件、+36/−7。`git diff` 与 `impl/p2b-step3.diff` **逐字节相同**（我已 diff 过）。
  只改了 `regbase/LatePipeline.cpp`：`#include …/regbase/PassPipeline.h`，把 P2-A 的 fail-closed 块
  换成 `buildTail = [&](PassManager &pm){ buildLowerToLLVMPipeline(pm, config); }` +
  `bishengir::runPipeline(module, buildTail, config, "late-tail")`，其后是**新的、更窄的** fail-closed 块。
  **没有加任何 instrumentation 代码** —— goal §5 要的是真实 MLIR 机制，`runPipeline` →
  `RetriablePassManager::runOnce` 已经装好了（`[PassID]`、`applyPassManagerCLOptions`×2、per-pass verifier）。
- **阶段切口正确**：`buildBiShengHIRAVEToLLVMPipeline`（`regbase/PassPipeline.cpp:213-229`）我先读了 ——
  `getCompileHost()` 提前 return → `getEnableHIVMCompile()` 进 `hivmave::buildLowerAVEPipelines`
  → 最后才是 `getLowerToLLVM()` → `buildLowerToLLVMPipeline`。**直接调 tail builder 到不了
  `buildLowerAVEPipelines`**，这就是结构性保证，不是风格选择。
- **33 条 `[PassID]` 与源码对账无缺口**：19 个注册 tail pass + 4 条 `OpToOpPassAdaptor` + 1 个 validator
  = 33，其中 15 个 module 作用域动作、14 个 func 作用域动作。**注意：tail 是 19 个 pass 不是 20 个**
  （见 §24.4，那是我文档的缺陷）。SIMT/DPX 四连**确实没被调度**，因为 `--pure-simt` 被 §2.3 拒。
- **流与树**：`compile.out` 33 行、`compile.err` 34 行、`inline.out` 33 行、`inline.err` 2488 行；
  `compile.err` 里 `IR Dump` 头 **0 个**，`inline.err` 里 **58 个**，`tree/` 下 **58 个文件** ——
  这组不对称正是 `--mlir-print-ir-tree-dir` 把 dump 写进文件而非流里，两者互证。
- **产物真的没写**：两次运行都带 `-o …/kernel.o`，之后 `kernel.o`/`kernel-inline.o` **都不存在**，
  rc=1，终行是 `Late mode: the bitcode identity gate and the HIVMC A5 lowering are not implemented in
  this build (P2-B tail only). Refusing to emit an artifact. No output was written.`
- **`stage-p2b/`**：`cd impl/stage-p2b && sha256sum -c SHA256SUMS.txt` → **4/4 OK**，manifest 头部写明
  `BASE DIRECTORY = this file's own directory` 并列出被取代的旧身份（`stage-p2a-r1/` 的 `1a3f20f8…`、
  `stage-p2a/` 的 `2bf961bf…`）。两个模板 bitcode 与 P1 钉住的**逐字节相同**（O2 `5e7ccae2…`、
  O0 `0a34be87…`）—— 也就是说 **P2-B 没有满足 §3.3 的"从 P2 树重建模板并以新 SHA 作为 P2 自己的身份"**，
  那是 P2-C 的事，报告自己也这么写，是**负结论不是漂白**。
- **`build/bin` 已回到 D031 基线**：我实测 `bishengir-compile` = `c6838fc0a175ec6bb05214780a2a3e99ab478285620996dd5bea4c69f26aabe9`
  （205,293,424 B，mtime `10-13 13:46`）、`bishengir-opt` = `0761c6e662ee015e30425ee29e0c2314753aa356b21eb4119ff99747327a1977`
  （mtime `10-13 14:46`），`build/lib` 模板仍是 `5e7ccae2…`/`0a34be87…`，`build/install/lib` 仍是 D012 前的
  `44e4387b…`/`55ba44b9…`。报告 §9.2 说这条 restore "queued, next action" —— 报的时候它还没做，
  我读到的时候已经做完了。**已要它确认 restore 是它做的、并说明 mtime 是怎么保住的**，然后重跑 G1–G4 + I1–I7。

### 2. 我的裁定：**P2-B = 仅 step 3**；两位 manager 批准

Implementor 把 §3.1 的 step 4–5 边界提上来当阻塞问题问（§9.1），这是对的问法。裁定：

- **P2-B 只做 step 3（conservative tail）**。step 4–5（`addBitcodeAttrsToModule` + §3.3 的硬检查，
  然后恰好一次 `runHIVMCCompileA5`）**归 P2-C**，连同 §6.2/§6.3。理由三条：goal §5 把 P2-B 定为
  "保守的 Legalization 尾部"；§6.2/§6.3 本就是 P2-C 的交付物；它的替代方案（走
  `shouldOnlyRunHIVMPipeline()` 的 lower-then-stop）会把 `--only-run-hivm-pipeline` 这条
  **当前既不被拒也没被钉**的选项拖进 §2.3.1 的选项策略，为了一行把 P2-B 的接口面撑大 —— 而那一行
  P2-C 会正面地、带身份门地满足。**`convert-hivm-to-llvm = 1` 与 A12/C2 在 P2-B 保持 NOT_RUN**，
  这是诚实状态：tail 与 A5 的 pass 集**不相交**，此处的 0 是结构性的，造一个 1 就是让 A5 无门运行。
- **两个 manager（`late-preflight` / `late-tail`）批准**：与 ordinary 路线的形状一致，且让 P2-A 引用的
  证据保持逐字节稳定；E5 的"每个进入的 manager 边界"要求由"恰好两个"的枚举满足。

### 3. **四个冻结 oracle 缺陷** —— 我逐条实测确认（不是采信报告）

Implementor 报的 §7 四条我全部自己复量了。**这是验收工具自身的缺陷，不是 P2-B 的实现缺陷。**

- **(D-a) `SYNC_LOCK_FINALIZE_PASSES` 里混进了一个 `Prepare` 阶段的 pass。** 实测：
  `createBindSyncBlockLockArgPass()` 只在 `SyncBlockLockPipelinePhase::Prepare` 分支
  （`bishengir/lib/Dialect/HIVM/Pipelines/regbase/HIVMRegbasePipelines.cpp:547-549`），`Finalize` 分支
  恰好四个（`:550-556`）；整个 regbase 侧 `addSyncBlockLockFinalizePasses` 的唯一调用点是
  `regbase/PassPipeline.cpp:262`，**就是 P2-B 跑的那个 tail**。所以
  `hivm-bind-sync-block-lock-arg` 在忠实的 Late 运行里必然测到 0，A11 的 `missing_lock` 非空。
  **goal §5 的要求本身是满足的**：必需的 SyncBlockLock **finalization** 就是那四个 Finalize pass，4/4 在场。
- **(D-b) dump 头正则**一个都匹配不上。实测真实格式是
  `// -----// IR Dump After <name> (<arg>) //----- //`，而 `run_p2_suite.py:247` 的 `DUMP_RE` 要求
  `\)\s*-+//` —— **`//` 在破折号之前**，所以匹配 0。我在**两个独立二进制**上验了：P2-B 的 compile
  （58 个头 → 解析出 **0** 条）和 P1 钉住的 `bishengir-opt` `0761c6e6…`（1 个头 → **0** 条）。
  后果：**凡是靠 dump 解析的断言全部失效**，包括 A10 里 amendment v2 item 8 专门加进去的
  "逐 dump body 扫 `mix_mode`/`FilterPassesAttr` 首次引入"那条（0 条记录，永远不会触发）。
  顺带：`positive_control_filter_off` 在 0 条记录时给出的**理由**是
  "every paired pass changed the IR" —— 那是对空集合的**无根据因果断言**，不只是措辞问题。
  **这一条在我 admission 冻结 v2 时就已经存在，我没查出来。**
- **(D-c) A11 的 leftover 扫描把输入自己数进去了。** `t_a11`（`:744`）对 `out + "\n" + err` 扫
  `hivm\.hir\.\w+` 并要求为空，而**金样本 `vadd_late_p1v2.mlir`（`d1023ac1…`，我已重测）本身含 15 处**
  （`get_block_idx`×2、`load`×2、`pipe_barrier`×1、`pointer_cast`×4、`set_flag`×2、`store`×2、`wait_flag`×2）。
  于是：打印 dump 的运行必然在"输入自己的 IR"上红，不打印 dump 的运行则在 `has_cf` 上红。**两条路都不可能 PASS。**
  这条我已在 §23 item 3a **先于**报告的落地预先登记过，两边独立得到同一结论。
- **(D-d) C1 与 B1/B2 不能同真。** `t_c1`（`:1379`）要求 `b01_cache0.mlir` 与
  `b02_cache_carrier_deleted.mlir` **rc=0**；而 `_expect_catch`（`:955`）用 **rc≠0 + 具名诊断**
  就满足 B1/B2；D035 已确认的 V7 对两者都拒（rc=1）。所以 C1 与 B1/B2 **不可能同时 PASS**。
  C1 编码的是 **D035 之前**的期望 —— 这是"验收设计在先、契约后来被加固"造成的漂移，不是谁放松了断言。

### 4. 我自己的记录缺陷：`p2-architecture.md` §3.1 的表**错了两处**，并传播进了 sealed oracle

D-a 的源头是**我**：`docs/p2-architecture.md` §3.1 的 tail 表把 `BindSyncBlockLockArg` 列成第 7 行，
并写着 "Items 5–9 are the `SyncBlockLockPipelinePhase::Finalize` family"，`5–9` 的行内顺序
（Infer, Mark, **Bind**, Lower, Free）也是错的 —— 实测的调度顺序是
**Infer → Lower（均 `nest<func::FuncOp>`）→ Mark → Free（均 module）**。冻结套件的
`SYNC_LOCK_FINALIZE_PASSES` 正是从这张表抄的，于是缺陷被"密封"进了 oracle。

已做的更正（**是更正，不是事后改判据**）：
- 表**就地改正**为真实的 19 行、真实顺序（删 Bind、`lower-create-sync-block-lock` 提到 Infer 之后、
  Mark/Free 顺延），并加 dated **CORRECTION** 段说明改了什么、为什么、以及对 sealed oracle 的影响。
- `docs/p2-pass-trace.md` C1 的 "the 20 tail passes" 就地改为 "the 19 registered tail passes"，注明改动日期与原因。
- 同文件那段的路径注错（`hivm/Transforms/HIVMRegbasePipelines.cpp`）改为
  `regbase/HIVMRegbasePipelines.cpp`（真实路径含 `Dialect/HIVM/Pipelines/`）。
- `docs/status.md:1829`/`:1832` 那两处"恰好 20 个 Pass"是**历史条目**，不追改历史，由本节作更正记录。
- **sealed oracle 一个字节都没动**：`run_p2_suite.py` 现在仍是
  `0677be1a7ba217d7587c9c65436d93966ec5d40c03f5b8cc28c61989beea5621`，与 freeze of record 一致（我实测）。

**顺带两条校准事实**：`verify/logs/suite/` 里现存的那次运行（`2026-10-13T15:55:28Z`，PASS=24/FAIL=0/NOT_RUN=0）
是用 `--only` 跑的**子集** —— 结果里**根本没有 A1、A10–A12、C1–C5**，也就是说 **A11/C1 从未被执行过**，
所以"失败后不许改 oracle"这条规则**尚未被触发**，这是运行前的缺陷。另一条：Implementor 的
`diag_p2b_trace.py` 导入套件时在冻结目录里生成了 `verify/tests/scripts/__pycache__/`；sealed 文件字节未变
（我实测 sha 一致），**no seal was disturbed**，但已要求它以后用 `PYTHONDONTWRITEBYTECODE=1` 或把 runner 拷到 scratch。

### 5. 处置与派发（都不改 oracle）

1. **Implementor**（msg `342cef7d`）：裁定 P2-B=仅 step 3、两 manager 批准；**立即提交**（`cfaa59677`
   的**后续提交，绝不 amend**），消息描述 tail-only 状态；重跑 G1–G4 + I1–I7 并报数；确认 `build/bin`
   的 restore 是它做的、mtime 怎么保的。**不许碰 `verify/`，不许开 P2-C。**
2. **Verifier**（msg `7446a572`）：**先把 `verify/logs/suite/` 归档**（`run_p2_suite.py:1725` 硬编码该目录，
   再跑一次就会毁掉 15:55 那次），再绑定 `stage-p2b` 跑**未修改的**套件，A10–A12/C1–C5 全跑，
   逐行记 rc/stdout/stderr 并**原样引用**套件自己的 detail 串；**独立复现或推翻** D-a~D-d 四条
   （推翻和证实一样有价值）；独立复核报告 §0 的"rc=1 且 `-o` 无文件"；**自己读** `tree/` 给出 D1–D11
   的自测值（D11 必须 entry-scoped）；不得按名字判断 sync-lock finalize 可删。
3. **Challenger**（`review-2`，msg `f7870b42`，已唤醒）：五个问题 —— ①pass cut 是否正确/充分
   （直接调 tail builder 是否能触达 §5 禁止物）、②四条 oracle 缺陷**各自该怎么处置**（在冻结记录
   §5 的"correctness fix 且要声明会作废 implementor 契约并需复验"规则下）、③P2-B 的 pass 证据是否是
   真 instrumentation（含 `compile.err` 0 头 vs `inline.err` 58 头这个不对称能不能藏东西、
   以及"全流扫零"是否真的**强于**套件的 dump-body 扫零）、④两 manager 下 E5 的证据该是什么、
   ⑤`p2-binary-abi.md` D5 的 32768 B 推到 P2-C 是合法 NOT_RUN 还是把 goal 划给 P2 的行往后挪。
   要求逐问给 APPROVE/REVISE/BLOCK，并明确"要改什么才会批"；**它若发现第五个缺陷要直说**。

**为什么同时唤醒三个**：CLAUDE.md 的"最多两个**执行中**teammate"是针对实现工作负载的；此处
verifier 与 implementor 各自在跑，challenger 是**只读评审**且 §8 第 3/8 步都要求它 —— 这属于
"架构门禁"，不是把并行度放宽。若 Challenger 的裁决要求改 oracle，那会走冻结记录 §5 的声明式
correctness fix 流程，届时**P2-A 的契约需要重新确认**，这一点我会先问用户再动。

### 6. 未决 / 下一步

- 等 Implementor 的提交 + G/I 复测；等 Verifier 的 P2-B 验收与四条复现；等 Challenger 的五问裁决。
- 等 Challenger 回来后再决定：D-a~D-d 是走"correctness fix"（并因此重开 P2-A 契约）还是
  "另建一份**追加式**的补充检查、把冻结套件原样留着并把这几行记为**已披露的红**"。
  **在裁决之前，一个字节都不改。**
- P2-C 未开始（§8 第 6 步）；P2-D 未开始；`build/bin` 已在基线。

**下一步**：等三份回报；未 push；`hivm-poc-evidence/` 本地独占。
