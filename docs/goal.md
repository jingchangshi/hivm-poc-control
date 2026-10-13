# GOAL: TileLang -> Late HIVM/HIVMAVE -> Ascend950PR real kernel

## 1. Scope and completion criteria

Build a minimal real DeviceCodeGen E2E POC for the existing TileLang F32 SIMD vector addition test:

```
TileLang vector_add(524288, mode="simd")
  -> existing target=ascend lowering
  -> _prepare_device_codegen_mod
  -> new HIVM DeviceCodeGen
  -> HIVM/HIVMAVE Late MLIR
  -> new AscendNPU-IR Late Pipeline
  -> existing HIVMC A5 + Bisheng device compilation
  -> a device kernel binary
  -> existing TileLang TVM-FFI Ascend runtime
  -> ACL launch on Ascend950PR, numerical compare against A+B
```

Avoid AscendC source generation and PTO codegen as intermediate implementations of this new backend. Reuse existing host/runtime/binary module mechanisms wherever possible.

SUCCESS P3: independent manually authored Late IR to a device binary, ACL-launched on physical hardware with verified numerical correctness.
SUCCESS P4: TileLang runtime consumes prebuilt binary from the approved P3 run with correct launch metadata, dynamically allocated UB, and numerical correctness. This is **runtime E2E**, not DeviceCodeGen E2E.
SUCCESS P6: normal TileLang HIVM mode compiles the real Golden-compatible device TIR through the new DeviceCodeGen and the Late Pipeline, launches the resulting binary through TVM-FFI, checks numerical correctness on Ascend950PR without loading fixed prebuilt code. This is **CodeGen E2E**.

## 2. Immutable Golden input

Fixture origin: `examples.ascend.example_vecadd.vector_add(524288, mode="simd")`.
Generator: `/home/s00653124/workspace/tilelang/tests/freeze_golden_hivm.py`.
Boundary: `_prepare_device_codegen_mod(device_mod)` after `LowerIntrin`, `Simplify`, `HoistBroadcastValues`.
JSON uses `tvm.ir.save_json`; reload via `tvm.ir.load_json` and structural equality. The `ascend_reference.asc` is a separate replay of existing AscendC CodeGen, NOT an HIVM semantic oracle.

Expected canonical target directory: `tests/golden/hivm/vadd_f32_single_tile/` relative to the TileLang repo.
IMPORTANT: the five currently staged Golden fixture files (plus the staged generator script) are still in `tests/tests/golden/hivm/vadd_f32_single_tile/` because they were created before generator ROOT was corrected. Migrate without altering bytes or losing the staged content.

JSON SHA-256: `576fa7a37bcaf1748e387efbd26508652e2e4b18790fa21aea874a7b756c0fe5`.
TIR SHA-256: `4194733b8412484c9bb8caf74ba3285348345f92ad61684784fe8c3a79e12a18`.
Manifest `ub_offsets_f32` are ELEMENT offsets 0, 16384, 32768; BYTE offsets are 0, 65536, 131072.
Single-tile geometry: N=524288, 64 AIV cores, 8192 f32/core, 128 vector iterations x 64 f32, 196608-byte dynamic UB arena, buffer spacing 65536 bytes, 32768 bytes copied per DMA buffer, synchronization events MTE2_V id=0 and V_MTE3 id=0, A/B/C arguments of float32 global, grid=64.

The frozen JSON has the plain Ascend target tag. If new HIVM routing needs additional target keys, work on a derived in-memory copy and test that this metadata change does not change the semantics; never mutate the Golden input files.

Do not re-run the Golden generation script before moving and checking the existing staged artifacts. Do not re-apply `_prepare_device_codegen_mod` to an already prepared serialized Golden when replaying directly into DeviceCodeGen.

## 3. Actual repository sources are authoritative

- AscendNPU-IR local repository is current GitCode master at e878f349. Re-inspect the actual local source rather than treating previous private fork findings as authoritative.
- TileLang local HEAD is 0e687c17. Reuse target kind `ascend` with mutually exclusive backend predicates if source confirms this is the least invasive design. Do not introduce a brand-new TVM target kind without strong evidence.
- Reference repository tilelang-mlir-ascend is read-only; inspect its A5 codegen as ideas, not as a directly compatible implementation.

Known TileLang source seams at baseline (verify local source before editing):
- `tilelang/ascend/backend.py`: backend registration and device codegen selection.
- `tilelang/ascend/target.py`: target_is_plain_ascend vs target_is_pto. A HIVM selection must not collide with either.
- `tilelang/ascend/execution_backend.py`: existing `tvm_ffi` execution backend.
- `tilelang/engine/lower.py`: preparation pass boundary, `context.codegen_device`, `inspect_source` and host module import.
- `src/ascend/codegen/rt_mod_ascend.cc`: `AscendModuleCreate` and `ExtractFuncInfo`.
- `src/ascend/runtime/ascend_module.cc`: ACL binary loading, kernel lookup, host args, grid and dynamic UB launch attributes.

## 4. Late IR contract review (must finish before producing the pipeline)

The challenger must inspect live code and record:
- Actual supported HIVM/HIVMAVE/AVE dialect op names, syntax, and verifier requirements.
- Mapping of blockIdx.x and GM pointer offsets / subviews into valid MLIR.
- Correct meaning of UB `pointer_cast`, base address 0, GM-to-UB and UB-to-GM DMA, exact byte vs element units.
- Explicit MTE2_V and V_MTE3 flag/wait placement, loop order, VF and vector lane lowering.
- The compiler ABI: kernel symbol, AIV topology, ACL arguments A/B/C, dynamic UB allocation and grid=64.
- The precise stage cut into existing A5 HIVMC/Bisheng path. Determine if runHIVMCCompileA5 or subordinate functions perform SIMD-to-LLVM lowering, and avoid running it twice.
- Passes that must be bypassed because TileLang already decided UB allocation, loop structure, sync, and scheduling. Identify indispensable normalizers/legalizers that remain.

Document exact source file paths and call graph. Use before/after IR dumps and discriminating tests, not assumptions.

## 5. Phases and hard gates

P0: Safely migrate and freeze Golden; create branches preserving current staged changes; verify JSON and TIR hashes, serialization, manifest, output path and reproducible script without overwriting fixture; create test/acceptance matrix and Late IR contract draft; obtain Challenger review. Never re-run Golden blindly.

P1: Hand-author valid minimal Late HIVM/HIVMAVE MLIR for exactly the frozen VADD, with parser/verifier tests, deliberate invalid-op/ABI-negative tests, and source/semantic trace for each input operation. No NPU-IR production code yet unless necessary to register an explicit test-only mode. Verify no loss of synchronization and UB spacing.

P2: Implement narrow AscendNPU-IR Late Pipeline entry and scoped pass sequence. Add command-line stage selection (exact spelling to be chosen after source examination). Keep normal RegBase behavior unchanged. Compile P1 MLIR into real A5 binary through existing HIVMC/Bisheng. Inspect pass dumps to prove conversion is not duplicated. Confirm the installed compiler used by test scripts is the new binary. Add MLIR regression tests and run relevant check-bishengir subset/full as reasonable.

P3: Independent ACL C++/Python harness loads the P2 binary and launches on real Ascend950PR. Use independent deterministic A/B data, several input patterns including nonzero block indices, compare all C against trusted elementwise A+B, verify grid=64, UB allocation=196608, no OOB and no race symptoms. The test harness must not use the TileLang new backend. Hardware numerical correctness, not compilation alone, gates P4.

P4: Add explicit HIVM backend selection and test-only PREBUILT-BINARY injection through existing TileLang Ascend TVM-FFI runtime. Generate/extract FunctionInfo/launch params from correct IR, propagate host args, grid and dynamic UB, preserve `inspect_source` contract. Run actual `tilelang.compile` host path + NPU using the frozen P3 binary. Keep this injection scoped and visibly named; it is NOT the final codegen. Run existing AscendC and PTO selection/regressions (if available). Challenger approves ABI before P5.

P5: Implement minimal real TIR -> HIVM/HIVMAVE MLIR DeviceCodeGen, only supporting the frozen Golden's op whitelist. Do not copy the entire tilelang-mlir-ascend codegen. Reuse current target/backend registry. Use exact Golden replay for deterministic CodeGen unit tests, verify semantic IR structure/op counts/UB/sync/ABI, and compile via P2 Late Pipeline. Unsupported operations fail with explicit diagnostics. Remove fake prebuilt behavior from production flow; retain a clearly isolated test fixture when useful.

P6: Run normal TileLang HIVM-mode compile and launch without reading manually authored MLIR or prebuilt binary. Verify real compilation event in logs and generated new artifact. Check output on Ascend950PR against independent torch addition, compare all elements, exercise multi-core indexing and nontrivial inputs. Run regression for existing AscendC/PTO and current Triton paths; record PASS or BLOCKED for every acceptance criterion. Complete handoff with code commits, commands, logs, diagrams/notes, limitations, and future work.

P7 (optional, only after P6): Test N=2**21 with multiple tiles per core and asynchronous/multi-buffer behavior. Do not compromise the single-tile POC to add broad support prematurely.

## 6. Task and resource isolation

- Use the existing host checkout branches to retain NPU-IR incremental CMake/Ninja build paths; don't move NPU-IR into a fresh worktree that makes existing `build` unusable.
- TileLang branch: `feat/hivm-backend-poc`; NPU-IR branch: `feat/tilelang-hivm-late-pipeline`.
- First inspect current status; preserve user-staged files and never reset/clean.
- Implementor edits production. Verifier owns separate independent tests/evidence. Challenger performs read-only review.
- At most one Ninja build/install process per build directory and one NPU acceptance run at a time.
- Everything builds/runs inside Docker `s00653124_tilelang`, using scripts/in-container.sh; keep `/home` paths consistent.
- Stage and commit phase-scoped changes only after independent tests; no automatic push.
- Keep all decisions and status in control directory because Agent Team session state is ephemeral.

## 7. Evidence and escalation rules

For each phase record: repository and HEAD, changed files, exact command, conda environment, expected vs actual, exit code, numerical assertion, compiler IR/binary metadata, PASS/FAIL/BLOCKED/NOT_RUN, reviewer verdict and next tasks.
Do not call a failure PASS after changing the test or switching to old compiler / AscendC / PTO / CPU fallback.
For hard compiler bugs: stop after two focused failures, ask challenger for competing hypotheses and the smallest discriminating test. Ask the user only for architecture-changing choices, missing credentials/access or environmental decisions that cannot be determined by experiment.

The lead must maintain a separate phase table and write `docs/status.md` before context compaction, shutdown, or any user checkpoint. Do not plan a single unbounded P0-P6 run.
