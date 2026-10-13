# Golden TIR → Late HIVM/AVE input contract

Date: 2026-10-13. Revision: P0 draft 4 (2026-10-13 re-authorized session). **Independent Challenger
verdict: REVISE, with P1 advancement BLOCKED** (D020). The boundary is source-supported, but the
contract is not accepted while D012 (cache4 carrier), D013 (terminal DMA completion), ABI,
pb32/VF preservation and execution evidence remain unresolved. No architecture is promoted to
ACCEPTED. **Gate state: the user closed the P0 gate by adjudication on 2026-10-13 (D021) and
authorized P1 — D012 → option (a) narrow store-side cache carrier extension, D013 → option 2
(discriminating experiment before P1 production work), cut → option (b) dedicated Late entry. This is
a user decision, not a Challenger APPROVE: the REVISE verdict above stands as the review record, and
D012/D013 remain hard gates until their closure conditions are met.** Scope: exactly immutable
`vadd_f32_single_tile`, no production implementation or complete hand-authored P1 kernel in P0. New in
this revision: locally-verified MLIR spellings (§3a), the D018 falsified-flag finding (§2 note), the
sharpened D009 caveat (the canonical template does not demonstrate the pb32 carrier), the D009
**silent-default mechanism** (§5), and the D012 **silent-drop proof** for the store cache attribute
(§4).

## 1. Authority and evidence boundary

TileLang HEAD `cc56643de04a49db6f50875e26d904b0c69e71a6`; AscendNPU-IR HEAD `e878f34927fb5b77fd8c7257fda3a95a3560f2ec`. Paths below are relative to the owning repository. Source facts support a design; they do not establish binary ABI compatibility or physical NPU correctness. Independent experiments and final verdict are recorded in `p0-verification.md`. Pipeline ownership and full pass sequence belong in `pass-cut.md`.

The user confirmed Golden migration and E0 are complete. JSON/TIR and all accepted fixture files are immutable. No regeneration or preparation replay is allowed to accommodate code generation output.

## 2. Accepted prepared TIR

Input is the saved IRModule from `tests/golden/hivm/vadd_f32_single_tile/golden_codegen_input.json`, not reconstructed text and not `device_mod.tir`.

| Property | Required value |
|---|---|
| JSON SHA-256 | `576fa7a37bcaf1748e387efbd26508652e2e4b18790fa21aea874a7b756c0fe5` |
| Prepared TIR SHA-256 | `4194733b8412484c9bb8caf74ba3285348345f92ad61684784fe8c3a79e12a18` |
| Boundary | After `_prepare_device_codegen_mod`: LowerIntrin → Simplify → HoistBroadcastValues |
| Kernel | `main_kernel(A,B,C)`, void, three float32 global handles in this order |
| Geometry | N=524288, grid x=64, 8192 elements/core, one tile/core |
| Vector work | 128 iterations, 64 f32 lanes each, full active f32 predicate, zeroing add |
| UB arena | 196608 bytes dynamic UB; spacing 65536 bytes |
| UB bases | A=0, B=65536, C=131072 bytes (TIR element offsets 0,16384,32768) |
| DMA | Two GM→UB reads, one UB→GM write, 32768 bytes each |
| Sync | set/wait MTE2→V event 0 after both reads; set/wait V→MTE3 event 0 after VF |
| Launch tags | `[blockIdx.x, tirx.use_dyn_shared_memory]`; dynamic UB tag last |

Sources: TileLang `golden_codegen_input.tir:7-43`, `manifest.json`; `tilelang/engine/lower.py:92-106`. JSON must deserialize and survive a structural roundtrip. Prepared Golden replay calls the selected DeviceCodegen directly, without applying `_prepare_device_codegen_mod` again. Ordinary compilation still follows the normal preparation path exactly once.

Generator provenance is statically inspectable in `tests/freeze_golden_hivm.py`: output ROOT derives from `__file__` (`:10-13`); the exact SIMD example is lowered without binary compilation (`:17-24`); prepared module capture is `:27-28`; `tvm.ir.save_json/load_json/assert_structural_equal` is `:41-49`; separate AscendC source-only replay is `:51-69`. The generator contains direct fixture writes and is not safe as a read-only validation command. It was not executed in this P0 session. Independent verifier scripts load committed data without overwriting it.

The TIR's view declaration lengths are bookkeeping for pointer expressions, not permission to shrink the accepted UB arena or buffer spacing. Each DMA/vector footprint is 8192 f32, while each slot reserves 16384 f32.

Supported input forms are only those present in the saved module: PrimFunc/module attributes, block thread binding, one shared.dyn allocation, buffer/address expressions, constant arithmetic, one fixed serial loop, VECTOR/SIMD_VF regions, the two DMA intrinsics, explicit set/wait intrinsics, and SIMD pset/vld/vadd/vsts. Future codegen must reject unsupported calls, dtypes, dynamic shape, extra arguments, additional tiles, unexpected memory scopes and unsupported control flow with diagnostics. No generalized optimizer or multibuffer support is implied.

The typed pointers and dtype, noalias, readonly input indices `[0,1]`, empty nonrestrict list, entry calling convention and symbol fallback are input metadata obligations (`golden_codegen_input.tir:7-8`). Preserve their meaning or explicitly document a proven redundant annotation; no new aliasing/output-allocation assumption may be inferred from a missing textual `global_symbol`. Current FunctionInfo extraction falls back to the module GlobalVar name when that attribute is absent (`rt_mod_ascend.cc:79-86`). Contract validation must check the launch symbol `main_kernel` through the same rule.

## 3. Late module and function boundary

The emitted module is bufferized, explicitly addressed and explicitly synchronized. It enters after TileLang has selected layout, vector schedule, UB allocation and synchronization. It contains no HFusion/tensor computation, implicit allocations or scheduling requests.

Current source defines dialect namespaces `hivm` and `ave`; `HIVMAVE` is a source component/C++ name, not the textual namespace. Operations are `hivm.hir.*` and `ave.hir.*`. Exact syntax and verifier constraints must be established by current TableGen/IR and P1 parser tests, not names copied from the reference repository.

### 3a. Locally-verified MLIR spellings (Lead, read-only, 2026-10-13)

These spellings were read from the LIVE local checkout (not copied from `tilelang-mlir-ascend`), and
are the starting point for the P1 hand-authored input. Primary template:
`bishengir/test/Dialect/HIVM/arith-constant-op-to-hivm-regbase-broadcast.mlir`.

| Construct | Verified spelling | Source |
|---|---|---|
| Module core type | `module attributes {hivm.module_core_type = #hivm.module_core_type<AIV>}` | `test/Dialect/HIVM/infer-hivm-mem-scope.mlir:203`; enum AIC/AIV/MIX `HIVMAttrs.td:317-330` |
| Function core type | `attributes {hivm.func_core_type = #hivm.func_core_type<AIV>}` | template `:6` |
| Device entry | `attributes {hacc.entry, hacc.function_kind = #hacc.function_kind<DEVICE>, ...}` | template `:51` |
| UB base | `arith.constant 0 : i64` + `hivm.hir.pointer_cast(%c) : memref<..., #hivm.address_space<ub>>` | template `:12-14` |
| GM→UB | `hivm.hir.load ins(%arg : memref<...,#hivm.address_space<gm>>) outs(%ub : memref<...,#hivm.address_space<ub>>)` | template `:15` |
| Loop | `scf.for %i = %c0 to %cN step %c64 { ... }` | template `:16` |
| Vector load | `ave.hir.vload <NORM> %subview[%c0] : memref<...> into vector<64xf32>` | template `:19` |
| Predicate | `ave.hir.pge <ALL> : vector<64xi1>` | template `:20` |
| vadd | `ave.hir.vadd %a, %b, %mask : vector<64xf32>, vector<64xi1>` | template `:25` |
| Masked store | `ave.hir.masked_store <NORM_B32> %subview[%c0], %mask, %v : memref<...>, vector<64xi1>, vector<64xf32>` | template `:31` |
| UB→GM | `hivm.hir.store ins(%ub : ...<ub>) outs(%gm : ...<gm>)` | template `:33` |
| Sync | `hivm.hir.set_flag[<PIPE_MTE2>, <PIPE_V>, <EVENT_ID0>]` / `wait_flag[...]` | `test/python/dialects/regbase/set_wait.py` |

**D009 caveat (sharpened):** the canonical template writes `ave.hir.pge <ALL> : vector<64xi1>` with
NO explicit `functionType`/pb32 carrier. So the Golden b32 requirement is NOT demonstrated by the
template — its silent default is a RISK to be resolved, not a precedent to copy. Either the pb32
carrier must be added explicitly or the b32 default re-established for the `vector<64xi1>` shape;
P1 must test this directly (B5–B7).

Module target must represent `Ascend950PR_9579` with the current HACC target/device-spec attributes. Keep the existing append-device-spec normalization until experiments prove a complete preprovided spec can replace it. Module must explicitly carry `hivm.module_core_type = #hivm.module_core_type<AIV>`; entry's function core attribute alone does not satisfy HIVMC's module-level dispatcher. Entry is a DEVICE, entry kernel, AIV function with `hacc.function_kind`, `hacc.entry` and HIVM core metadata. Precise attribute syntax is governed by current local definitions and the reviewed mapping appendix. Challenger source trace: `runDeviceBiShengLIRCompile:474` reads module core type; `translateDeviceKernelToLLVM:380-389` has an unreachable path for a non-SIMT module missing it.

Proposed entry signature uses three static `memref<524288xf32, #hivm.address_space<gm>>` arguments, in A/B/C order, with no returned values. Static contiguous shape and identity layout are required for current bare-pointer device conversion. It must lower to exactly three GM pointers, not memref descriptors, grid arguments, workspace or hidden tiling arguments. Final LLVM signature and ELF metadata are mandatory P2 checks; source support alone is not acceptance.

VF is an explicit function marked `hivm.vector_function`, invoked with the same marker on its call. The current LLVM translation uses the marker on both callee and call to establish SIMD VF attributes. Keep it internal to the kernel module; it is not a second host-launchable kernel. VF passes may narrow induction variable types; the 128 iterations, 64 lanes, addresses and synchronization ordering must remain semantically identical.

Sources investigated by Challenger: NPU-IR `HIVMToLLVMIRTranslation.cpp:175-217`; `HIVMAVEToAVEIntrin.cpp:3371-3474,3491-3514`. Complete repository-relative paths and marker syntax will be fixed in the reviewed appendix.

## 4. Address and access semantics

For logical AIV block b in [0,63], element j in [0,8191]:

```text
GM element = b * 8192 + j
C[GM element] = f32_add(A[GM element], B[GM element])
UB byte addresses: A 0 + 4*j; B 65536 + 4*j; C 131072 + 4*j
```

`hivm.hir.get_block_idx` returns the physical block index as i64 (`HIVMOps.td:38-45`); use an explicit width/index conversion for subview arithmetic. GM subview offset is b*8192 in elements, size8192, stride1. Each of 64 blocks owns a disjoint 32768-byte GM range; total elements 64*8192=524288. Test nonzero b and final b=63, not just b=0.

UB `hivm.hir.pointer_cast` receives one i64 byte address and static memref metadata, explicitly in `#hivm.address_space<ub>`. Its lowering performs direct inttoptr, without multiplying by element size. Constants are therefore 0,65536,131072, never 0,16384,32768. Address zero is a compiler-accepted UB representation; validity on the physical device remains P3 acceptance.

Use separate 8192-element views rooted at each fixed base for accessed payload, while preserving 65536-byte slot spacing and 196608-byte arena. Accessed half-open byte intervals are [0,32768), [65536,98304), [131072,163840); reserved slots are [0,65536), [65536,131072), [131072,196608). No buffer address planning, packing or arena reduction is allowed in the Late front-end path.

DMA is contiguous burst count1 and 32768 bytes. Existing RegBase DMA lowering derives burst length from size0*sizeof(element), so memref size8192 f32 establishes the correct payload. GM offsets use element units; pointer_cast uses byte units; launch UB uses bytes. These distinct units require independent tests.

The immutable DMA call operands are explicitly part of the input whitelist (`golden_codegen_input.tir:19,22,43`; TileLang `src/ascend/op/builtin.h:188-192,208-215`; codegen `:1383-1432`):

| Call | Golden operands after dst/src, in TIR order | Preservation obligation |
|---|---|---|
| GM→UB A/B | sid0, burst_num1, burst_len32768, left_pad0, right_pad0, data_select0, load_cache0, src_stride32768, dst_stride32768 | Same unpadded one-burst access and cache selection; document any scalar field not represented in candidate MLIR |
| UB→GM C | sid0, burst_num1, burst_len32768, store_cache4, dst_stride32768, src_stride32768 | Same one-burst output and cache4; current high-level store mapping unresolved |

The two stride fields are explicit byte-valued Golden operands. With a single burst, a representation may encode equivalent one-burst access without using a stride field, but the mapping must explain this rather than silently treating constants as unrelated metadata. `sid` is not passed to the current AscendC C APIs (`codegen_ascend.cc:1406,1424`); preserve this documented boundary instead of inventing a hidden device argument. Padding/data-select/cache values require their own supported mapping checks.

**Unresolved exact store mapping / hard gate:** Golden UB→GM intrinsic argument at zero-based index5 (the sixth argument) is `l2_cache_ctl=4`, not a stride or an ignorable constant. TileLang `src/ascend/op/builtin.h:211` describes it; `src/ascend/codegen/codegen_ascend.cc:1424-1431` forwards it as `asc_store_l2_cache_mode(4)`. Current NPU-IR `hivm.hir.store` lowering exposes atomicKind but no cache mode (`regbase/HIVMToStandard.cpp:1262-1275`), and the `__DAV_C310__` DMA template path sets l2_cache_ctl=0 (`RegBase/DMAUtils.h:268-289`). Matching bytes/addresses does not establish exact DMA equivalence.

**The single-pass probe has now been run and the result is worse than "unmapped" (SD-01/SD-03, 2026-10-13):** setting `{l2_cache_ctl = 4 : i64}` on `hivm.hir.store` **parses without error**, but `-convert-hivm-to-std` output is byte-identical to the cache0 case apart from the function name — the emitted libcall `@store_ubuf_to_gm_2d_float(..., %c0_i32)` still receives `0`. The attribute is **silently discarded, not rejected**. A Late-IR author could write cache4, observe a clean compile, and ship cache0. Target/template macro applicability and the linked intrinsic cache value remain separate checks. P1 may not claim a faithful Golden kernel until an existing low-level representation preserves cache4, an equivalence proof is accepted, or a reviewed narrow extension is approved. Do not drop cache4 or alter Golden.

Sources investigated by Challenger: `HIVMToLLVM.cpp:232-244`; `RegBase/DMAUtils.h:488-502,539-550`; exact owning paths are in the reviewed mapping appendix. Negative tests must detect a factor-four UB error, swapped A/B/C, overlapping slots, changed arena or payload, missing block indexing, and wrong subview layout.

The per-core and per-iteration coverage oracle is independent of emitted syntax: b=0 maps GM elements0–8191; b=1 maps8192–16383; b=63 maps516096–524287. Vector iteration i accesses j=64*i through64*i+63; i=127 ends at8191. Payload32768 bytes fits within each65536-byte slot, leaving32768 reserved bytes per slot. The final payload ends at UB byte163840, but the accepted reserved arena remains196608; shrinking it based only on accesses violates the launch contract.

## 5. Vector and synchronization semantics

The two loads execute before `set_flag[MTE2,V,0]` and matching wait. The VF then performs exactly the Golden's full predicate setup and 128 successive 64-element vload(A), vload(B), f32 vadd, vstore(C). After the VF call, set/wait V→MTE3 event0 precedes the one output DMA.

Textual AVE op/mask names and lane encoding must be taken from current ODS and the reviewed appendix. In particular, `vector<256xi1>` alone does not imply 32-bit predicate granularity. Golden pset(32,PAT_ALL) requires an explicit `functionType = #ave.func_dist_type<pb32>` on the candidate `ave.hir.pge <ALL>` op. Expected lowering is PgeB32 with full f32 lanes. Independent PG-01/02 confirmed explicit pb32 versus absent-type mask256 lowers to b32 versus b8. This is not a universal fallback: PG-03 with absent-type mask32 aborts in AVENormalizeOps (exit134), while PG-04 with explicit pb32 succeeds. Preserve the original failed expectation. These probes ran normalization plus conversion; direct primitive conversion without the broad optimizer remains a full-input NOT_RUN obligation. Keep zeroing and full f32 lane behavior.

**Why the carrier is mandatory, not stylistic (sharpened 2026-10-13).** Re-derived from source: `AVEPgePattern` (`AVENormalizeOps.cpp:587-631`) reads `functionType` via `getBitWidthFromAttr` (`HIVMAVE/Utils/Utils.cpp:864-884`); when the attribute is absent it derives the width as `VL_BITS/numElems` = 2048/numElems (`HIVM/Utils/Utils.h:567,577-578`) and accepts only 8/16/32, otherwise `report_fatal_error` (`:625`). The Golden mask is `boolx256`, so 2048/256 = **8** — a *legal* derived width. Omitting the carrier therefore does **not** fail the build; it silently emits `intr.hivm.pge.b8 {mask_bit_width = 8}` where the Golden's `asc_create_mask_b32(PAT_ALL)` demands b32. The Golden sits on the one mask length where this defect is silent (a 32-element mask derives 64 and aborts, as PG-03 shows). Consequence: the P1 checks (B5–B7) must assert the emitted `mask_bit_width = 32` / `pge.b32` directly; "the compile succeeded" is not evidence, and the canonical template's carrier-free `vector<64xi1>` form (which would silently derive 32) must not be copied. Source: `lead/lead-source-trace.md` §28.

Required invariants are data dependency and access order, address ranges, loop iteration coverage, full mask and exact event/pipe IDs. Legalization can change types and expand operations without changing these invariants. Do not invoke GraphSyncSolver/InjectSync or re-vectorization on already explicit input.

The Golden has no explicit final MTE3 drain. The terminal completion barrier in the normal pipeline is a **GraphSyncSolver artifact, not a legalizer artifact**: `CodeGenerator::insertBarrierAllBeforeReturn` creates a `PIPE_ALL` `hivm.pipe_barrier` immediately before the return of a **device-entry** function (`GraphSyncSolver/SyncSolverCodeGen.cpp:923-933`), and it is called only inside `if (options.isIntraCoreMode())` (`:975-982`), which is true for the hardcoded `INTRA_CORE_SYNC` the pass builds (`GraphSyncSolver.cpp:76`; `Utility.h:150-153`). `hacc::utils::isDeviceEntry` requires the `hacc.entry` attribute (`HACC/IR/HACCInterfaces.cpp:151-154`, `HACC/IR/HACCAttrs.td:75`), and the pass returns early for `hivm.vector_function` (`GraphSyncSolver.cpp:70-72`, attr `HIVM_VFAttr`, `HIVMAttrs.td:1315`). `InjectSync` inserts the same terminal barrier in `BARRIERALL` mode (`InjectSync/InjectSync.cpp:50-67`, matched on `func::ReturnOp` at `:56`) **and** in `NORMAL` mode via `SyncAnalyzer::InsertLastPipeAll` (`InjectSync/SyncAnalysis.cpp:103-119`, called from `Plan()` at `:53-54`; `insertBarAllAtLast` defaults to true at `SyncAnalysis.h:56`), both gated on `isDeviceEntry`. So the terminal barrier has three emitters, and the Late cut skips all of them. The regbase A5 stage picks GSS when `enableHIVMGraphSyncSolver && !enableHIVMInjectBarrierAllSync` (`HIVM/Pipelines/regbase/HIVMRegbasePipelines.cpp:115-131`), and in `bishengir-compile` those default **true / false** (`Tools/bishengir-compile/Options.td:202-206`, `:576-579`), so GSS is the normal source; the CCE-level `--cce-auto-sync=bar-all` injection is off by default (`Tools/hivmc/Config.h:534-536,996`). Upstream `ReturnOpLowering` replaces `func.return` with `LLVM::ReturnOp` and adds no barrier (`third-party/llvm-project/mlir/lib/Conversion/FuncToLLVM/FuncToLLVM.cpp:678-719`). The Late cut skips GSS and InjectSync, so it drops that epilogue and the conversion does not restore it. **Correction (2026-10-13):** the frozen `ascend_reference.asc` is AscendC **source** (1848 bytes of `__simd_vf__`/`__global__` C++), not compiler output, so its lack of an explicit terminal barrier says nothing about what the AscendC compiler emits; the earlier "the AscendC path shares the question, so the device layer likely supplies it" inference is weaker than first recorded. A pass-level discriminating experiment is frozen in `lead/d013-experiment-frozen-20261013.md`. Whether device/runtime return completion suffices remains a P3 hardware question. Any required explicit compiler-completion epilogue needs independent review and must be documented separately from the immutable four Golden set/wait operations; it must not silently rewrite the oracle.

## 6. Existing TileLang backend and runtime compatibility

Use TVM target kind `ascend` and a distinct predicate-selected HIVM BackendModule. Current plain Ascend predicate only excludes PTO; a new HIVM predicate alone would overlap. Future routing must make plain Ascend/PTO/HIVM exclusive and reject combined pto+hivm selection. The Golden retains its original target; any routing metadata is applied only to an in-memory derivative, with semantic equality checked independently.

Sources: `src/ascend/target.cc:36-52`; `tilelang/ascend/target.py:33-40`; `tilelang/ascend/backend.py:55-78`; `tilelang/backend/module.py:238-255,300-307`. Predicates and CLI/API names for the future feature are proposed, not currently implemented.

Proposed selection truth table (future implementation acceptance, not current results):

| Target kind | PTO key | HIVM key | Required backend |
|---|---|---|---|
| ascend | absent | absent | existing plain Ascend |
| ascend | present | absent | existing PTO |
| ascend | absent | present | new HIVM |
| ascend | present | present | reject with an ambiguity diagnostic |
| any other | either | either | no Ascend-family predicate matches |

Current `DeviceCodegen` callbacks resolve `target.build.tilelang_ascend` / `_without_compile` and their PTO equivalents (`tilelang/ascend/codegen.py:8-18`). HIVM must declare its own callbacks in its manifest; engine dispatch stays through the resolved context. Explicitly reuse `pipeline.ascend_pipeline` for ordinary frontend lowering, while direct prepared-Golden replay bypasses both that pipeline and device preparation. The current full Ascend sequence is in `tilelang/ascend/pipeline.py:36-187`: layout, scheduling/sync, tile-op lowering, flattening, host/device split, UB merge, launch lowering already occur upstream of the Golden boundary.

If replay needs an in-memory target derivative, independently compare parameters, body, return type, buffer mappings, launch tags, thread extents and all attributes except the explicitly reviewed routing target change. Preserve the target's existing architecture/feature attributes and demonstrate the derivative does not rerun lowering. Comparing the whole module for structural equality after deliberately changing its target is not an appropriate assertion; compare a copy with only that reviewed target change normalized. Recheck original fixture hashes after replay. This is a future test plan, NOT_RUN in P0.

Reuse existing Ascend pipeline, STANDARD_HOST_CODEGENS and tvm_ffi compatibility when independently verified. `ExecutionBackendSpec` must preserve eager device compile, host codegen and the declared callee-allocated output capability. No new Runtime or launcher is required by current source evidence. Device codegen produces new binary Bytes with format `aibin`, FunctionInfo and inspectable Late MLIR.

`ExtractFuncInfo` in `src/ascend/codegen/rt_mod_ascend.cc:53-89` is a private anonymous-namespace helper, not an exported API. Future integration needs narrow shared extraction or equivalently tested metadata extraction. `AscendModuleCreate` exists in C++ and as global `tl.ascend.ModuleCreate` (`src/ascend/runtime/ascend_module.cc:534-562`).

The runtime handles ACL binary load, kernel symbol lookup, per-device lifetime and HostArgs packing (`ascend_module.cc:84-217,404-423,518-531`). On the recorded 64-bit platform three device handles pack at offsets0/8/16, total24 bytes. Grid64 and UB196608 are additional host packed-function values consumed as launch configuration, not device-entry arguments. `LaunchParamConfig` reads launch tags after the three device args; the runtime passes dynamic UB in launch attr ID2 and calls aclrtLaunchKernelWithHostArgs (`ascend_module.cc:452-493`; TVM `src/runtime/thread_storage_scope.h:250-309`). Raw HostArgs packing loops over exactly `FunctionInfo.arg_types`, excluding launch extras (`ascend_module.cc:278-291`). Runtime structures/constants are local definitions (`:313-332`); matching CANN SDK structure identity and generated binary metadata remain P3/P4 gates. Reuse the existing Torch NPU stream exchange and `runtime.Executable` adapter (`tilelang/jit/adapter/tvm_ffi.py:40-45,78-81,134-153`).

Native ownership already sits under `src/ascend/CMakeLists.txt:22-42`: with USE_ASCEND enabled it includes `src/ascend/codegen/*.cc` and `runtime/*.cc`. New backend-owned production files in that directory would use this existing source selection after a reviewed incremental rebuild; this is a source integration point, not a P0 build. No additional runtime library is justified by this investigation.

Default inspect_source currently returns source_map[`asc`] only. To preserve engine behavior without a runtime change, store Late MLIR under that default-compatible key and optionally explicit `mlir`. No AscendC intermediate is implied. Binary serialization does not retain source_map, so fresh and cached source inspection need separate assertions; do not claim cached MLIR inspection works (`ascend_module.cc:379-402,542-555`; `tilelang/engine/lower.py:190-195`).

P6 compilation freshness must include the actual new DeviceCodegen callback and actual compiler subprocess, in addition to output identity. Existing `global_func_device_codegen` invokes `run_codegen_with_instrumentation` (`tilelang/backend/device_codegen.py:17-28`; `tilelang/instrumentation/session.py:251-261`), providing a source-supported observation point. A codegen event alone can still return a cache hit, so it does not prove a new compiler invocation. Current AscendBinaryCache key uses TileLang version, source hash, target/arch/format and options (`tilelang/cache/ascend_binary_cache.py:22-42`); any future HIVM cache must additionally distinguish the approved Late route/compiler identity and the explicit P4 test fixture. This is a future design/test obligation, NOT_RUN in P0.

## 7. Acceptance obligations and unknowns

P0 requires immutable Golden checks, live source contract and call chain, independent tests designed before production and final Challenger verdict. P1 must establish exact parseable syntax and negative diagnostics for the complete kernel. P2 must show one LLVM lowering, exact entry signature, symbol/core/UB metadata and a fresh real binary. P3/P4/P6 must prove all-element F32 correctness on physical NPU through their separate paths.

Unproved until their designated gates: physical UB address0 validity; AIV topology/block index behavior for every block; predicate and register encoding after LLVM; terminal DMA completion; generated ELF compatibility with the existing ACL runtime; absence of extra ABI params; binary cache/source inspection behavior for the new backend. No compiler-only probe or source fact closes a hardware gate.

Test plan and outcomes are maintained independently in `p0-verification.md`; a minimized legalizer list is a candidate until full P1 input and pass deletion experiments justify it. This contract deliberately retains the conservative source-supported tail at P0.
