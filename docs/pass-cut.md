# Current A5 call chain and Late pipeline cut

Date: 2026-10-12 (source); 2026-10-13 (regbase symbols + Challenger verdict). **Independent
Challenger verdict: BLOCK for the existing `--enable-direct-hivm-lowering` mechanism; REVISE for the
architecture** — a dedicated, mutually exclusive Late entry in the existing RegBase driver, no new
target kind, skipping HIR/HFusion, bufferization, planning, auto-sync, loop lowering, multibuffer and
`buildLowerAVEPipelines`, retaining validated legalization and exactly one `runHIVMCCompileA5`
(D020). **Gate state: the user adopted that architecture by adjudication on 2026-10-13 (D021, option
(b)) and authorized P1.** The verdict itself is unchanged; P2 pass instrumentation is still required
to prove the cut. Source authority: `/home/s00653124/workspace/AscendNPU-IR-1` HEAD
`e878f34927fb5b77fd8c7257fda3a95a3560f2ec`. This records live source call order, not the old private
repository or a skill's historical pipeline. No Late production entry exists yet; the proposed
minimal pass set is not experimentally proven — P2 pass instrumentation is still required.

## 1. Driver selection and ownership

`bishengir/tools/bishengir-compile/bishengir-compile.cpp:53-70` scans argv for target to select RegBase; `:147,184-186` dispatches to `regbase::runRegBaseCompile`. An A5 target attribute in input MLIR alone does not select the A5 driver. Future Late invocation must specify current local option `--target=Ascend950PR_9579`. A new stage selector is not implemented and its spelling is not approved in P0.

`bishengir/lib/Tools/bishengir-compile/regbase/Driver.cpp:68-81` currently infers layout optimization (and MixedCV on the Triton path) before invoking the RegBase pipeline. Future Late dispatch must occur before these upstream transformations. Forwarding inside that driver is commented, but the executable's `main` still takes the legacy route whenever `BISHENGIR_LEGACY_A5_REGBASE` is present (`bishengir/tools/bishengir-compile/bishengir-compile.cpp:105-119`); the implementation checks presence, not whether its value equals `1`. The proposed Late route concerns the merged native branch. Future execution evidence must record that branch explicitly.

Current normal HIR path, with option-dependent nested builders, is:

```text
runRegBaseCompile
  -> runRegBasePipeline
     -> RegBase buildBiShengHIRPipeline
        -> HFusion / conversion to HIVM / HIVM tensor optimizations
     -> buildFinalHIVMPipelines
        -> RegBase buildLowerHIVMPipelines
     -> RegBase buildBiShengHIRAVEToLLVMPipeline [separate driver call, SIMD flow]
        -> buildLowerAVEPipelines [enableHIVMCompile; optimizer guarded by RegBase && !enableDirectHIVMLowering]
        -> RegBase buildLowerToLLVMPipeline [getLowerToLLVM; legalization, NOT final LLVM]
     -> runHIVMCCompileA5
        -> runSIMDToLLVMCompile
           -> HIVMC buildBiShengHIRAVEToLLVMPipeline
              -> ConvertHIVMToLLVM and LLVM tail
        -> device translation / linking / Bisheng compile
```

Exact nested upstream builder sequence and flags are recorded in the source investigation appendix below. The two `buildBiShengHIRAVEToLLVMPipeline` names belong to different layers. Repeating their names in logs does not itself prove repeated lowering; inspect their namespace, file and actual pass sequence.

**Verified call sites (2026-10-13, read live):** `bishengir/lib/Tools/bishengir-compile/regbase/BiShengIRCompileMain.cpp` — `buildBiShengHIRPipeline` at `:409`, `buildFinalHIVMPipelines` at `:412`, `buildBiShengHIRAVEToLLVMPipeline` at `:435` (SIMD flow only), `runHIVMCCompileA5` at `:448`. Builder bodies: `buildFinalHIVMPipelines` `PassPipeline.cpp:326-337` (calls `hivm::regbase::buildLowerHIVMPipelines`); `buildBiShengHIRAVEToLLVMPipeline` `PassPipeline.cpp:213-230` (HACC→LLVM if `getCompileHost`; `buildLowerAVEPipelines` if `getEnableHIVMCompile`; `buildLowerToLLVMPipeline` if `getLowerToLLVM`). The whole SIMD sequence sits inside a `for` retry loop over `tryTimes` that increases HFusion buffer counts on failure — the Late route must NOT inherit this retry behavior (a failing Late input must fail loudly, not be re-scheduled).

**RegBase tail builder symbols (verified 2026-10-13, read live):** the RegBase entry is
`buildBiShengHIRPipeline(OpPassManager&, const BiShengIRCompileMainConfig&)` at
`bishengir/lib/Tools/bishengir-compile/regbase/PassPipeline.cpp:461`. Under
`if (config.getEnableHIVMCompile())` it calls, at `:501-502`:
`hivm::regbase::buildConvertToHIVMPipeline` (def `Dialect/HIVM/Pipelines/regbase/HIVMRegbasePipelines.cpp:723`)
and `hivm::regbase::buildHIVMTensorOptimizations` (def same file `:750`). **The RegBase/A5 path does
NOT call `buildOptimizeHIVMPipeline`** — grep over `regbase/PassPipeline.cpp` returns zero hits; that
builder is only in the generic driver (`Tools/bishengir-compile/PassPipeline.cpp:107`) and the
op-pipeline registration (`Dialect/HIVM/Pipelines/HIVMPipelines.cpp:592`). Do not cite the generic
symbol pair for the A5 route; the HIVM-tensor-optimization step is regbase-specific.

## 2. Proposed Late boundary

Proposed input: primitive `hivm`/`ave` plus func/arith/scf/memref, static GM entry, fixed UB pointer_cast bases, explicit VF and sync, target/device metadata and module AIV. Contract is `late-ir-contract.md`.

Proposed route:

```text
parse + verify Late input, explicit A5 CLI target
  -> validate narrow contract, reject mismatched/unsupported metadata
  -> append-device-spec normalization (retain until complete matching spec proven)
  -> RegBase legalization tail
  -> exactly one runHIVMCCompileA5
```

Bypass normal HIR, tensor optimization, final HIVM allocation/scheduling, and `buildLowerAVEPipelines` for this fully primitive Late input. Keep normal RegBase behavior unchanged for all existing inputs. Do not call ConvertHIVMToLLVM in the new front-end route and then invoke HIVMC's default SIMD-to-LLVM path again.

**CORRECTION (2026-10-13, D018):** an earlier revision of this section suggested the existing `--enable-direct-hivm-lowering` flag "may already express the needed bypass". That is **FALSIFIED**. The AVE guard reads `hivmave::HIVMAVEPipelineOptions::enableDirectHIVMLowering` (`HIVMAVE/Pipelines/HIVMAVEPipelines.cpp:39`), but that field has **no assignment anywhere** in `bishengir/`: `setupHIVMAVEPipelineOptions` (`regbase/PassPipeline.cpp:150-210`) is hand-written and never calls `config.getEnableDirectHIVMLowering()`. The generated `ConfigUtils.cpp.inc:159` sets the flag only inside `setupHIVMPipelineOptions` (`regbase/PassPipeline.cpp:103`), so only the HIVM guard (`HIVMRegbasePipelines.cpp:710`) is reachable from the compile driver. In the `bishengir-compile` device path the AVE field stays `false`, the guard is always true, and `buildLowerAVEPipelines` always runs its optimizer. Bypassing it therefore requires (a) a source change plumbing the config getter into `setupHIVMAVEPipelineOptions`, (b) a dedicated Late entry that does not call that builder, or (c) evidence the primitive Late input survives the AVE optimizer unchanged — an architecture decision, not an existing flag. Full trace: `lead/lead-source-trace.md` §16.

**Challenger resolution (2026-10-13, D020):** option **(b)** is the correct architecture — a dedicated,
mutually exclusive Late entry in the existing RegBase driver that does not call
`buildLowerAVEPipelines`. Option (a) (plumb the getter) is only a narrow plumbing correction: it would
empty the AVE optimizer body but still leaves upstream bufferization, UB planning, auto-sync, loop
lowering and multibuffer, so it is not the cut. Option (c) (assume primitive IR survives the
optimizer) is **BLOCK** — compile success would not prove preservation of explicit UB addresses,
synchronization, VF/pb32 metadata, cache policy or ABI. The dedicated entry must skip HIR/HFusion,
bufferization, PlanMemory, auto-sync (GraphSyncSolver/InjectSync), loop lowering, multibuffer and the
AVE builder, retain only the validated legalization tail, and run exactly one `runHIVMCCompileA5`.
`buildLowerAVEPipelines` has exactly one call site (`regbase/PassPipeline.cpp:223`, inside
`buildBiShengHIRAVEToLLVMPipeline` under `if (getEnableHIVMCompile())`), so a Late entry that runs only
the `getLowerToLLVM()` legalization branch skips it structurally. **Caveat (Challenger, Lead-verified):**
`AppendDeviceSpec` is **not idempotent** — it warns "Overwriting the device spec…" and then overwrites
unconditionally (`AppendDeviceSpec.cpp:142-151`), so the Late entry must validate a complete matching
spec or deliberately choose overwrite behavior rather than inherit the pass blindly. This remains a
proposal: no execution trace proves the invocation counts yet (P2).

`runHIVMCCompileA5:972` invokes `runSIMDToLLVMCompile:876`; its pipeline builder in `bishengir/lib/Tools/hivmc/A5/PassPipelineA5.cpp:47-88` performs final LLVM conversion. `OnlyRunHIVMPipeline`/CPURunner modify lowerToLLVMFlag only after that SIMD conversion (`:975-981`), so those flags cannot be used as a mechanism to avoid a duplicate conversion already performed earlier.

## 3. Conservative existing legalization tail, exact order

Source: `bishengir/lib/Tools/bishengir-compile/regbase/PassPipeline.cpp:243-295`, `regbase::buildLowerToLLVMPipeline`. Sync helper expansion: `bishengir/lib/Dialect/HIVM/Pipelines/regbase/HIVMRegbasePipelines.cpp:545-560`.

| Order | Pass / expansion | Scope or condition |
|---|---|---|
| 1 | AnnotationLowering | module |
| 2 | AllocToAlloca | module |
| 3 | InsertInitAndFinishForDebug | nested func |
| 4 | MarkDisableLoad | module |
| 5 | InsertInferSyncBlockLockNumAndInitFunc | nested func, expanded sync helper |
| 6 | SyncBlockLockLowering | nested func, expanded sync helper |
| 7 | MarkSyncBlockLockWithSubblock | module, expanded sync helper |
| 8 | InsertFreeLockVarBeforeReturn | module, expanded sync helper |
| 9 | ConvertHIVMToStandard | module; DMA library-call lowering |
| 10 | ConvertHIVMAVEToStandard | module |
| 11 | FixCallUnknownLoc | nested func |
| 12 | ExpandStridedMetadata | module; materializes subview metadata |
| 13 | ConvertHIVMAVEToAVEIntrin | module; AVE primitive lowering and VF target attributes |
| 14 | HoistVstas | module |
| 15 | AdaptGPUKernel, HoistCallScalarToCaller | pureSIMT && useDPX only |
| 16 | DPXDivOptimization / SIMTInsertInitAndFinishForDebug | additional guarded options within pureSIMT && useDPX |
| 17 | ConvertAscendDPXToHIVMRegbaseIntrin | module; current existing tail inserts this also without DPX |
| 18 | DecomposeFRem | module |
| 19 | ConvertSCFToCF | module |
| 20 | LowerAffine | module |
| 21 | ArithExpandOps | module |

This conservative tail is the initial reuse candidate, not a claim that every pass is indispensable for F32 VADD. Sync-lock helpers are not the normal GraphSyncSolver/InjectSync passes, but their no-op behavior on the exact Late whitelist must still be verified. The candidate route excludes pure SIMT/DPX/debug/sanitizer features by contract.

## 4. Semantic core and safe bypass candidates

Source-supported dependency core for the narrow primitive input:

1. AppendDeviceSpec unless the input already carries a complete validated target spec.
2. ConvertHIVMToStandard for supported HIVM DMA operations (subject to unresolved store cache4 mapping).
3. ExpandStridedMetadata for GM subviews and subsequent pointer construction.
4. ConvertHIVMAVEToAVEIntrin for load/add/store/predicate, induction-variable handling and VF attributes.
5. ConvertSCFToCF for loop control.
6. HIVMC's default SIMD LLVM tail exactly once: ConvertHIVMToLLVM → LowerAffine → ConvertIndexToLLVM → ArithToLLVM → ReconcileUnrealizedCasts → AppendUsePrintDebugData, under the non-Triton/non-debug/non-sanitizer configuration.

Candidate removals from the conservative front-end tail: AnnotationLowering if no annotations; AllocToAlloca if no alloc; debug helpers if no debug ops; sync-lock helpers if no locks; ConvertHIVMAVEToStandard if no i64 AVE/illegal division; FixCallUnknownLoc if no affected locations; HoistVstas optimization if unneeded; DPX conversion if no DPX and no relevant address-space effects; FRem/ArithExpand if absent. These are input-based hypotheses, not a proven minimal pass set. Remove only after identical independent assertions on the full P1 kernel and before/after dumps; do not simplify based on pass names alone.

Mandatory bypassed upstream capabilities: HFusion scheduling/conversion, tensor bufferization, UB/workspace memory planning, automatic scope/core partition and blockification, automatic synchronization, multibuffering, VF fusion/vectorization, vector layout inference/normalization and broad loop/reduction optimization. TileLang already fixed those decisions. `buildLowerAVEPipelines` includes conversions and optimization/scheduling that are not required merely because the dialect is AVE; primitive fully specified input can use the legalizer directly, subject to P1 verification.

## 5. Actual LLVM conversion and ABI

Registered conversion spellings are in `bishengir/include/bishengir/Conversion/Passes.td`: `convert-hivm-to-std:128`, `convert-hivmave-to-std:388`, `convert-hivmave-to-ave-intrin:425`, `convert-hivm-to-llvm:443`.

Standalone ConvertHIVMToLLVM defaults `use-bare-ptr-memref-call-conv=false` (`Passes.td:450-452`). HIVMC wrapper instead passes `config.tryBarePtrCallConvForStaticShape()=true`. Any standalone ABI discriminator must explicitly use `use-bare-ptr-memref-call-conv=true`; otherwise it tests a different calling convention. Static identity GM memrefs must become exactly three device pointer parameters. Vector function internal arguments are not host device-entry arguments.

HIVMC device path requires module-level AIV core metadata. `runDeviceBiShengLIRCompile:474` dispatches from module_core_type; `translateDeviceKernelToLLVM:380-389` has an unreachable branch if non-SIMT input lacks it. Function-level AIV alone is insufficient. Record exact symbol/core/device-target and LLVM types in P2 dumps before claiming ABI compatibility.

## 6. Known blockers and discriminating experiments

**Store cache policy:** Golden forwards l2_cache_ctl4. Current high-level hir.store has no cache operand; relevant RegBase DMA template path sets0. A single-pass existing-store probe can establish the library-call boundary lacks cache selection, but final template/target applicability and actual linked instruction require separate inspection. Do not label this mapping exact or numerical equality proven. Existing low-level representation, accepted equivalence, or a narrow reviewed extension is required before P1 proceeds.

**Terminal DMA completion:** the terminal barrier is a GraphSyncSolver artifact. `insertBarrierAllBeforeReturn` (`GraphSyncSolver/SyncSolverCodeGen.cpp:923-933`) creates a `PIPE_ALL` barrier before the return of a device-entry function, gated on `isIntraCoreMode()` (`:975-982`) — always true for the hardcoded `INTRA_CORE_SYNC` (`GraphSyncSolver.cpp:76`) — and on the `hacc.entry` attribute (`HACC/IR/HACCInterfaces.cpp:151-154`). `InjectSync` adds it in `BARRIERALL` mode (`InjectSync.cpp:50-67`) **and** in `NORMAL` mode via `SyncAnalyzer::InsertLastPipeAll` (`InjectSync/SyncAnalysis.cpp:103-119`, called from `Plan()` at `:53-54`; `insertBarAllAtLast` defaults true at `SyncAnalysis.h:56`), both gated on `isDeviceEntry`. The A5 regbase stage selects GSS by default (`HIVMRegbasePipelines.cpp:115-131`; `bishengir-compile/Options.td:202-206,576-579`). The Late cut skips both passes, so it drops the barrier, and `ReturnOpLowering` does not restore it (`third-party/llvm-project/mlir/lib/Conversion/FuncToLLVM/FuncToLLVM.cpp:678-719`). **Correction:** `ascend_reference.asc` is AscendC source, not compiled output, so its lack of a terminal barrier is not evidence about emitted code. **Result (2026-10-13): the frozen pass-level discriminating experiment ran and reported — the drop is CONFIRMED** (`lead/d013-result-20261013.md`; §5 row `HAS / NO / NO`). **P1 obligation:** the Late IR tail must carry terminal completion explicitly, or justify the drop with independent device-layer evidence. Device/runtime return completion is carried forward as an explicit P3 obligation. If a compiler epilogue is required, review it separately and preserve all four Golden flag/wait operations.

**Predicate granularity:** pge mask256 without functionType defaults to 8-bit. Freeze expected PgeB32 with explicit pb32 vs PgeB8 without; execute same primitive op via current AVEIntrin pass. This is a P0 discriminating experiment, not a full P1 kernel.

**No double lowering:** P2 pass dumps must count exactly one execution of the actual ConvertHIVMToLLVM pass, zero forbidden allocation/sync-inference/scheduling passes and verify legalizer order. Source trace is not an execution count. Count pass invocations by pass identity/scope in instrumentation, not raw text occurrences or both before/after dump headers. Keeping existing sync-lock legalizers is distinct from rerunning GraphSyncSolver/InjectSync on explicit synchronization.

A full Late input that already contains LLVM functions cannot be passed blindly into the proposed pre-HIVMC route; its validator must reject a mismatched stage. A lower-level already-LLVM entry would require a separately reviewed API and contract, not an option inferred from existing post-SIMD flags.

Commands/logs/results for P0 probes are in `p0-verification.md` and the independent evidence directory. Compiler build, full kernel lowering, actual linked binary and all physical NPU tests are NOT_RUN in P0.

## 7. Reproducible source checks

The following commands are read-only and are the minimum reproducible source trace for this document. They use absolute repository roots so a changed working directory cannot silently select another checkout. Their output is evidence of source text only; it does not prove pass invocation or binary behavior.

```sh
NPU=/home/s00653124/workspace/AscendNPU-IR-1
TL=/home/s00653124/workspace/tilelang
rg -n 'hasRegBaseTarget|BISHENGIR_LEGACY_A5_REGBASE|runRegBaseCompile|parseSourceFile' "$NPU/bishengir/tools/bishengir-compile/bishengir-compile.cpp"
rg -n 'inferLayoutOptimization|runRegBasePipeline|runHIVMCCompileA5' "$NPU/bishengir/lib/Tools/bishengir-compile/regbase/Driver.cpp" "$NPU/bishengir/lib/Tools/bishengir-compile/regbase/BiShengIRCompileMain.cpp"
rg -n 'createConvertHIVMToStandardPass|createConvertHIVMAVEToStandardPass|createExpandStridedMetadataPass|createConvertHIVMAVEToAVEIntrinPass|createConvertSCFToCFPass' "$NPU/bishengir/lib/Tools/bishengir-compile/regbase/PassPipeline.cpp"
rg -n 'createConvertHIVMToLLVMPass|createLowerAffinePass|createConvertIndexToLLVMPass|createArithToLLVMConversionPass|createReconcileUnrealizedCastsPass' "$NPU/bishengir/lib/Tools/hivmc/A5/PassPipelineA5.cpp"
rg -n 'isAscend950|meta_op.aiv.c310|dav-c310-vec|cce-link-aicore-ll-module' "$NPU/bishengir/lib/Tools/bishengir-compile/regbase/BiShengIRCompileMain.cpp" "$NPU/bishengir/lib/Tools/hivmc/A5/HIVMCMainA5.cpp"
rg -n 'hacc.entry|function_kind|module_core_type|vector_function|functionType' "$NPU/bishengir/include/bishengir/Dialect/HACC/IR/HACCAttrs.td" "$NPU/bishengir/include/bishengir/Dialect/HIVM/IR/HIVMAttrs.td" "$NPU/bishengir/include/bishengir/Dialect/HIVMAVE/IR/HIVMAVEAttrs.td"
rg -n 'VFPgeOp|VFLoadOp|VFMaskedStoreOp|AVE_BinaryOp' "$NPU/bishengir/include/bishengir/Dialect/HIVMAVE/IR/HIVMAVEOps.td"
rg -n 'data_select_bit|l2_cache_ctl|_mlir_ciface_(load|store)_' "$NPU/bishengir/lib/Template/lib/RegBase/Vector/include/RegBase/DMAUtils.h"
rg -n 'VectorCoreCount|Ascend950PR_9579' "$NPU/bishengir/include/bishengir/Dialect/HACC/Targets/NPUTargetSpec.td"
```

For a pass execution trace, use the approved compiler's own pass instrumentation on a frozen P1 input and save full argv, executable hash, stdout/stderr, and pass names. Counting source occurrences or repeated builder names is insufficient. No such full Late trace exists in P0.

## 8. Source investigation appendix

Lead audited these anchors directly at the recorded HEAD. This appendix is source evidence; current independent Challenger approval and full-kernel execution remain BLOCKED/NOT_RUN respectively.

| Stage / condition | Current source anchor | Observation |
|---|---|---|
| Driver flow | `bishengir/lib/Tools/bishengir-compile/regbase/BiShengIRCompileMain.cpp:53-59,345-448` | Mixed → runMixedPipelines; PureSimt → TTIR builder; ordinary SIMD → HIR then final HIVM; SIMD AVE/legalizer is a separate driver call at432-436. Module cloned per attempt; normal retry loop is not a validated Late route. |
| Final HIVM builder | `bishengir/lib/Tools/bishengir-compile/regbase/PassPipeline.cpp:326-337` | enableHIVMCompile guard; Mixed optionally delayed HFusion vectorization; calls regbase buildLowerHIVMPipelines. Does not call AVE/legalizer builder. |
| AVE/legalizer builder | Same file `:213-229` | compileHost runs HACC-to-LLVM and returns; enableHIVMCompile invokes AVE builder; getLowerToLLVM invokes conservative legalization. |
| AVE optimizer guard | `bishengir/lib/Dialect/HIVMAVE/Pipelines/HIVMAVEPipelines.cpp:37-89` | Entire optimizer under RegBase && !enableDirectHIVMLowering. Includes VectorToHIVMAVE, ArithToHIVMAVE, reduction/loop/layout transforms and AVENormalizeOps. **The guard's flag is unreachable from the compile driver (D018):** `setupHIVMAVEPipelineOptions` never populates `hivmAVEPipelineOptions.enableDirectHIVMLowering`, so the guard is always true in the `bishengir-compile` path and this optimizer always runs. Bypassing this builder requires an explicit route, not the existing flag. Bypassing still requires primitive input proven suitable for the conversion pass; PG probes ran standalone normalization and do not prove bypass safety. |
| Config transfer | `bishengir/lib/Tools/bishengir-compile/regbase/BiShengIRCompileMain.cpp:281-339` | HIVMCFromBiShengIRConfig copies target/output and Triton/Mixed/SIMT/debug/sanitizer/bitcode/FMA/warp/shared-memory/stack options; sets autoVectorizeV2(false), deterministicComputing(true), useDPX(true), onlyRunHIVMPipeline(false). Late route must constrain options rather than blindly inherit broad capabilities. |
| Template attributes | Same file `:61-126,440-448` | Relative to actual compiler executable, ../lib; canonical existing files only; Ascend950 AIV selects meta_op.aiv.c310.<resolved optimization>.bc. Then save temps/config transfer/HIVMC. Missing file is skipped by addIfExists and must become an explicit future acceptance failure. |
| Final LLVM ownership | `bishengir/lib/Tools/hivmc/A5/PassPipelineA5.cpp:47-88`; `HIVMCMainA5.cpp:876-880,944-1025` | shouldLowerToLLVM guard; static bare-pointer option supplied to ConvertHIVMToLLVM. Default non-Mixed/non-SIMT branch runs SIMD conversion before onlyRunHIVMPipeline/CPURunner change flags. |
| Conversion legality | `bishengir/lib/Conversion/HIVMToLLVM/HIVMToLLVM.cpp:97-147,232-244,368-386` | Partial conversion; unknown ops dynamically legal. Raw UB inttoptr; function/memref/control-flow patterns. Independent final-op whitelist is still required. |
| Translation | `bishengir/lib/Tools/hivmc/A5/HIVMCMainA5.cpp:374-411,465-560` | Module AIC/AIV classification required; removes module core/HACC target for LLVM translation. MIX splitting only when selected; translate, save LLVM IR, lowerDeviceToBinary, optional relocation. |
| Device compilation | Same file `:61-233,883-943` | Ascend950 → dav-c310 with AIV -vec suffix; --cce-aicore-only, default -O2, -cce-bitcode-is-aicore; RegBase adds -mllvm --cce-vf-auto-sync=global; bitcode via -cce-link-aicore-ll-module; parameter-size from countDeviceArgSizeInByte. These flags do not prove terminal MTE3 completion. |
| Linking/relocation | Same file `:239-299,300` and `:552-556` | ld.lld-based linker/relocation helpers; relocation conditional. Exact output loaded by ACL must be captured at P2/P3. |
| CMake executable | `bishengir/tools/bishengir-compile/CMakeLists.txt:17-37` | bishengir-compile links BiShengIRRegBaseCompileLib and collected private/conversion/translation libraries. |
| CMake RegBase | `bishengir/lib/Tools/bishengir-compile/regbase/CMakeLists.txt:25-53` | Compiles PassPipeline.cpp/BiShengIRCompileMain.cpp/Driver.cpp/Utility.cpp; links translation/conversion/dialects/extensions and MLIRHIVMAVEPipelines. |
| CMake HIVMC | `bishengir/lib/Tools/hivmc/CMakeLists.txt:1-19`; `A5/CMakeLists.txt:1-8` | BiShengIRHIVMCLib links A3/A5; BiShengIRHIVMCLibA5 compiles HIVMCMainA5.cpp and PassPipelineA5.cpp. |

Exact conservative order remains section3; candidate minimum remains section4. No source audit establishes actual invocation counts, safe deletion or binary correctness. Remaining work: complete typed mapping appendix, selected DMA symbol/bitcode trace for9579, completion discriminator, independent final review. The coordination checklist is `/home/s00653124/workspace/hivm-poc-evidence/p0-20261012-late-contract/p0-handoff-checklist.md`; no separate handoff document exists.
