# POC status

> **Snapshot notice.** This is the simplified-technical-English rewrite of `docs/status.md`, taken
> BEFORE the user adjudicated the P0 gates on 2026-10-13 (D021). It therefore still says "P1
> advancement BLOCKED" and "P0 gate is NOT closed". Those statements were true when written and are
> now superseded: the gate is closed by user adjudication and P1 is authorized. For current state read
> `docs/status.md`. Kept as a wording reference, not as the record.
>
> **Further supersession (2026-10-13, after this snapshot).** Its "D012 and D013 stay open as hard
> gates" line is also out of date. The D013 discriminating experiment has since been executed and
> reported: the drop is confirmed, and D013's gate is closed. D012 remains a hard gate. Read
> `docs/status.md` for current state.

- Date: 2026-10-13
- Current stage: **P0 IN PROGRESS**. The user re-authorized P0 on 2026-10-13.
- **The P0 gate is NOT closed.** The independent Challenger delivered its verdict: the contract
  boundary is REVISE, and P1 advancement is BLOCKED. The pipeline cut is BLOCK for the existing
  flag, and REVISE toward a dedicated Late entry. These verdicts are REVISE/BLOCK, not APPROVE.
  D012 and D013 stay open as hard gates.
- This status supersedes the 2026-10-12 "P0 BLOCKED" handoff. Model-route safety refusals caused
  that block, not a compiler finding. The user has re-authorized P0.
- P1–P6: **NOT_RUN**. P7: OPTIONAL / NOT_RUN. This phase produced no production code. It ran no
  rebuild, no install and no physical NPU run.

## Repository ledger

| Repository | Branch | HEAD | Observation |
|---|---|---|---|
| TileLang | `feat/hivm-backend-poc` | `cc56643de04a49db6f50875e26d904b0c69e71a6` | Clean index and worktree. The Golden commit is present. |
| AscendNPU-IR | `feat/tilelang-hivm-late-pipeline` | `e878f34927fb5b77fd8c7257fda3a95a3560f2ec` | Clean index and worktree. The existing build directory is retained. |
| Reference `tilelang-mlir-ascend` | `main`, read only | `417dae9908ad2ea5ffb74294a7075b6cb64b4f2b` | Clean. No edits. |

## Task A — Golden freeze: **CONFIRMED COMPLETE, no action taken**

The five fixtures are already at the canonical `tests/golden/hivm/vadd_f32_single_tile/`. The old
`tests/tests/...` path does not exist. No migration or regeneration occurred, and none was needed.

| Fixture | SHA-256 |
|---|---|
| `golden_codegen_input.json` | `576fa7a37bcaf1748e387efbd26508652e2e4b18790fa21aea874a7b756c0fe5` (matches the contract) |
| `golden_codegen_input.tir` | `4194733b8412484c9bb8caf74ba3285348345f92ad61684784fe8c3a79e12a18` (matches the contract) |
| `ascend_reference.asc` | `c025961ada28868bf3c1d4af70d550c2742109a92a59d190aa61055047d776c8` |
| `device_mod.tir` | `0fc125b511c70be4c12e6f2dc830b6a134aa2a0bc8f8b07a2e8dd8e3922df3df` |
| `manifest.json` | `42dda67a5694530f8efa878f27cdbb4d7530dc93f0cdd72d0afef162b1c7b221` |

The Verifier ran an isolated script: **55/55 PASS, exit 0**. The script checks the two contract
hashes, the manifest invariants, the geometry cross-checks, a JSON round-trip with
`tvm.ir.load_json` and `assert_structural_equal`, the absence of the legacy path, and the
`__file__`-relative ROOT in `freeze_golden_hivm.py`. Log: `verify/verify_golden.log`. Script:
`verify/verify_golden.py`. The Verifier also re-hashed all five fixtures at the end of the session
(`golden-rehash-end-of-session.log`, 16:50:07Z). The hashes did not change, so the verification
mutated nothing. `summary.tsv` records the status of each item, including one deliberate
**NOT_RUN** (T1.10 provenance replay; the freeze script does not run in place).

## Task B — Branches: **CONFIRMED COMPLETE**

Both branches already existed and were clean. This phase made no commit, reset, clean, push or
remote change.

## Task B2 — Compiler binary identity: **RESOLVED (Verifier Task 2)**

Evidence: `verify/build-artifact-identity.json`, `build-vs-install-byte-diff.log`,
`single-byte-diff-context.log`, `path-tool-resolution.log`, `binary-pin-note.md`.

- The `build/bin/*` and `build/install/bin/*` files differ by **exactly one byte** — the trailing
  `:` of `DT_RUNPATH` (`$ORIGIN/../lib:` against `$ORIGIN/../lib`). They are the **same code**.
  This finding resolves the earlier "same size, different SHA ⇒ UNPROVEN" concern (D019).
- **A bare `PATH` inside the container does NOT resolve to the local build.** It resolves to the
  CANN 9.2 toolkit (`/data/pri/cann-9.2.0/.../tools/bishengir/bin/`, opt sha `248b6c09…`). Every
  compile probe that used a bare tool name tested a foreign binary. That set includes the Lead's
  first `--enable-direct-hivm-lowering` probe, which was therefore re-run against the explicit
  local path.
- `bishengir-toolkits/dev/bin/bishengir-compile` symlinks to `build/install/bin/bishengir-compile`.
- P1/P2 evidence must pin the **absolute path and the sha256** of the local binary. It must never
  use `command -v`.

## Task C/D — Source investigation: Lead trace complete; Challenger verdict DELIVERED

Lead evidence: `lead/lead-source-trace.md` (951 lines). Independent Challenger verdict:
**DELIVERED** (recorded in `docs/decisions.md` D020). The Challenger writes no `.md` file, by its
own instruction, so `review/` is empty by design. Summary: **the contract boundary is REVISE with
P1 advancement BLOCKED; the pipeline cut is BLOCK for the existing flag and REVISE toward a
dedicated Late entry; D018 → option (b); D012 → BLOCK for P1.** The Lead re-verified two new
Challenger source claims, and the verifier independently concurred
(`verify/challenger-claims-check.log`). First, `AppendDeviceSpec` is **not idempotent**
(`AppendDeviceSpec.cpp:142-151` warns and then overwrites unconditionally; the pass is
non-idempotent whenever a pre-existing spec differs). Second, the pge width default also exists on
the lowering path (`HIVMPgeOpLowering`, `HIVMAVEToAVEIntrin.cpp:1435-1440`). That lowering *stamps*
`mask_bit_width`, and it is a second silent-accept site for the Golden's derived-8 case. The
second claim independently confirms the §28 D009 mechanism. See `lead/lead-source-trace.md` §30.

Key established facts:

**Late-IR contract (high confidence):**

- The closest existing template is
  `bishengir/test/Dialect/HIVM/arith-constant-op-to-hivm-regbase-broadcast.mlir`. That file
  establishes the canonical idiom for a regbase AIV GM->UB / vload / vadd / masked_store / UB->GM
  kernel.
- `hivm.hir.pointer_cast(<i64 BYTE offset>)` matches Golden byte offsets 0 / 65536 / 131072.
- The sync maps exactly: `MTE2_V` → `hivm.set_flag[PIPE_MTE2, PIPE_V, EVENT_ID0]`; `V_MTE3` →
  `hivm.set_flag[PIPE_V, PIPE_MTE3, EVENT_ID0]`.
- Function AIV metadata `hivm.func_core_type = #hivm.func_core_type<AIV>` is required (D009).
- The ABI is confirmed: three bare GM pointers. The grid (64) and the dynamic UB (196608) arrive
  as launch attributes (`kAclLaunchKernelAttrDynUbufSize`), sourced from the
  `tirx.kernel_launch_params` tail.

**D009 (predicate granularity) — MANDATORY carrier; omission is SILENT at the Golden's geometry:**
Golden `pset(32,"PAT_ALL")` is a **b32** predicate. The frozen `ascend_reference.asc` emits
`asc_create_mask_b32(PAT_ALL)`. The carrier is `functionType = #ave.func_dist_type<pb32>` on the
pge and masked_store ops. The Lead re-derived the mechanism from source this session:
`AVEPgePattern` (`AVENormalizeOps.cpp:587-631`) reads `functionType` with `getBitWidthFromAttr`.
**If `functionType` is absent, the pass derives the width as `VL_BITS/numElems` = 2048/numElems
and accepts only 8/16/32. Any other value calls `report_fatal_error`.** The Golden mask is
`boolx256`, so 2048/256 = 8 is a **legal** derived width. An author who omits the carrier gets
**silent `intr.hivm.pge.b8 {mask_bit_width = 8}` wrong-code, not a diagnostic**. A 32-element mask
would derive 64 and abort (PG-03, rc 134). The Golden will not fail loudly. P1 must therefore
assert the emitted `mask_bit_width = 32` directly. "The compile succeeded" is not evidence. See
`lead/lead-source-trace.md` §28.

**D012 (cache4) — carrier EXHAUSTED and SILENT-DROP PROVEN, hard P1 gate:** `hivm.hir.store` has
NO cache operand (`HIVMDMAOps.td:157`). The `HIVM_EvictionPolicyAttr` is on `LoadOp` only (MTE2),
and the raw `l2_cache_mode` I32Attr is on `ND2NZOp` only (also MTE2). The HIVM dialect has **no
MTE3 store-side cache carrier**. The store libcall lowering (`HIVMToStandard.cpp:1020-1032`)
appends only `atomicKind`. The RegBase C310 template hardcodes `l2_cache_ctl=0`. Yet the frozen
AscendC reference forwards cache4 faithfully as `asc_store_l2_cache_mode(4)`. **Probe SD-03 now
proves the attribute is silently discarded rather than rejected:** `hivm.hir.store ...
{l2_cache_ctl = 4 : i64}` parses cleanly, but `-convert-hivm-to-std` emits a libcall byte-identical
to the cache0 case (SD-01) apart from the function name — still `%c0_i32`. A Late-IR author could
write cache4, see a clean compile, and get cache0. Cache4 is real and must not be silently
dropped. Carrier table: `lead/lead-source-trace.md` §17.

**D013 (terminal DMA completion) — refined, still a P1–P3 gate:** the Late tail ends at arith
expand ops. The Lead found no explicit return barrier in the MLIR tail. **However**, the frozen,
E0-accepted `ascend_reference.asc` itself contains **no terminal barrier either** (the last
statement is the store). The AscendC path therefore shares the question, so the finding de-risks
the Late route but does not close it. Closure needs a P2/P3 binary-level comparison of the
terminal completion structure. See `lead/lead-source-trace.md` §18.

**Pipeline cut point — hypothesis, NOT accepted:** `runHIVMCCompileA5` does the real HIVM->LLVM
conversion, through `runSIMDToLLVMCompile` and the HIVMC A5 `buildLowerToLLVMPipeline`. A name
collision exists between the regbase and the HIVMC-A5 `buildBiShengHIRAVEToLLVMPipeline`. The Late
route must skip the HIR/HFusion scheduling and the final HIVM allocation and scheduling stages,
and it must run exactly one HIVM->LLVM conversion. **The earlier hope that an existing
`--enable-direct-hivm-lowering` flag could express the needed bypass is FALSIFIED (D018):** that
flag never reaches the AVE optimizer guard in the `bishengir-compile` device path, because
`setupHIVMAVEPipelineOptions` does not populate it. `buildLowerAVEPipelines` therefore always runs.
Bypassing it needs an explicit route. See `lead/lead-source-trace.md` §16.

## Blockers and open design questions

1. **D012 cache4** — the carrier search is exhausted. No MTE3 store-side cache field exists in the
   HIVM dialect. Before P1, the team needs a reviewed narrow extension, an accepted equivalence
   argument, or an alternative store op.
2. **D013 terminal DMA completion** — unresolved. The E0-accepted AscendC reference has no
   terminal barrier either, so the AscendC path shares the question. A P2/P3 binary-level check
   must close it.
3. **D009 mask width** — the representation is identified (`pb32`). No one has exercised it on a
   one-op input yet.
4. **Pipeline cut** — `enableDirectHIVMLowering` cannot bypass the AVE optimizer in the compile
   driver (D018, falsified hypothesis). **The Challenger verdict (D020) is BLOCK for the flag
   mechanism and REVISE toward a dedicated, mutually exclusive Late entry in the existing RegBase
   driver.** That entry adds no new target kind. It skips HIR/HFusion, bufferization, planning,
   auto-sync, loop lowering, multibuffer and `buildLowerAVEPipelines`. It retains the validated
   legalization and runs exactly one `runHIVMCCompileA5`. The team must add explicit dispatch and
   contract validation, plus P2 pass instrumentation.
5. **Runtime predicate exclusivity (D004)** — confirmed. `target_is_plain_ascend` is defined by the
   *absence* of `pto`, so an `hivm` key would make it match too, and `_resolve_backend_module`
   would raise. The team must redefine the plain predicate to exclude `hivm` (see
   `lead-source-trace.md` §19, §19a).
6. **AppendDeviceSpec is not idempotent** (Challenger claim, Lead re-verified) — the pass warns and
   then overwrites the device spec unconditionally (`AppendDeviceSpec.cpp:142-151`). The Late route
   must validate a complete matching spec or deliberately choose overwrite behavior. A silent
   overwrite could change the target spec out from under a hand-authored Late module.

## Next action

The independent Challenger verdict is in (D020). Per the user's instruction, P0 ends here for
review: **the Lead reports the consolidated open questions, the acceptance results and the
next-phase plan to the user.** No P1 production work starts until the user adjudicates the open
gates — in particular D012 (cache4 carrier and equivalence) and D013 (terminal completion), and
the choice of a dedicated Late entry (Challenger option (b)). The verifier has applied the Lead's
C1 correction and both tightenings to `p0-acceptance-criteria.md`.

## Lead artifacts this session (2026-10-13)

- `lead/lead-source-trace.md` — §28 (D009 silent-default mechanism, 2048/numElems) and §29 (D012
  silent-drop proof, SD-01 against SD-03) added; the file is now 951 lines.
- `lead/lead-review-of-acceptance-matrix.md` — the Lead review of the verifier matrix: ACCEPT with
  the C1 source-citation correction and two tightenings. C1: the A5 route uses the regbase
  `buildConvertToHIVMPipeline` and `buildHIVMTensorOptimizations`, NOT the generic
  `buildOptimizeHIVMPipeline`. The two tightenings: B4 (D009) and the D/§F PATH rule. The verifier
  applied all three (matrix Rev 2).
- Docs updated: `late-ir-contract.md` → draft 4 (D009 mechanism in §5, D012 silent-drop in §4);
  `pass-cut.md` §1 (verified regbase tail symbols); `decisions.md` D009 and D012 sharpened, **D020
  (Challenger verdict) recorded**; `p0-verification.md` §6.5 verdict, §6.6 gate status.
- The Lead received the independent Challenger verdict by SendMessage and recorded it (D020).
  `review/` is empty by the Challenger's own no-`.md`-artifact instruction, so the verdict lives in
  `decisions.md`.

## Phase table

| Area | Outcome | Limit |
|---|---|---|
| Golden freeze and hashes | PASS (independent, 55/55; end-of-session re-hash unchanged) | No one regenerated the Golden |
| Branches preserved | PASS | No commits or pushes |
| Compiler binary identity | PASS (Verifier Task 2) | No rebuild or install |
| TileLang backend and runtime trace | PASS (Lead source) | Routing not implemented |
| A5 compile-chain trace | PASS (Lead source; Challenger independently anchored) | No execution trace |
| Late-IR contract | DRAFT 4, evidence-backed | Challenger verdict **REVISE / P1 BLOCKED** |
| Pass cut point | HYPOTHESIS; flag route FALSIFIED (D018) | Challenger: BLOCK for flag, REVISE for dedicated Late entry; P2 must prove it with IR dumps |
| P0 acceptance matrix | DRAFT Rev 2, Lead-reviewed and verifier-revised (`p0-acceptance-criteria.md`) | The Challenger has not reviewed this file |
| Independent Challenger verdict | **DELIVERED** (D020) | REVISE/BLOCK, not APPROVE |
| P1–P6 | NOT_RUN | No authority in this phase |
