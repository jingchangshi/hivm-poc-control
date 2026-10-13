# Test-first phase acceptance matrix

| Phase | Independent proof | What MUST NOT count |
|---|---|---|
| E0 | Recorded TileLang SIMD, NPU-IR compile, Triton NPU E2E in correct environments (already PASS) | Mere tool presence |
| P0 | Byte-for-byte Golden migration, immutable SHA, JSON structural roundtrip, manifest invariants, provenance and replay, reviewer approves contract | Rewriting Golden to match generated code |
| P1 | Late MLIR parses/verifies; input mapping and negative tests; ABI/UB/sync preserved with recorded source evidence | Text produced without parsing |
| P2 | P1 Late MLIR -> newly built real A5 binary; precise pass cut, no double lowering, regression checks and installed binary identity | Old installed compiler, output file existence only |
| P3 | Standalone ACL binary load/launch + independent numerical oracle on real Ascend950PR, all outputs correct | Compiler exit 0, tensor print only |
| P4 | Golden host path + PREBUILT binary via TileLang TVM-FFI, all numerical data correct; source and ABI metadata verified | Claim that TIR -> HIVM CodeGen was implemented |
| P5 | Direct replay Golden TIR -> NEW HIVM DeviceCodeGen -> parseable and semantically equivalent MLIR -> P2, unsupported ops rejected | Manually authored MLIR or prebuilt injection presented as generated |
| P6 | `tilelang.compile` HIVM selected, actual CodeGen invocation and new binary, NPU execution and independent full tensor equality; AscendC/PTO regression | CPU/AscendC/PTO fallback or loading frozen binary |
| P7 optional | Multi-tile behavior on N=2**21, independent numerical tests | Required for single-tile POC acceptance |

All phases use PASS / FAIL / BLOCKED / NOT_RUN. None may be silently omitted. Every PASS needs command, exit code, evidence and reviewer.
For P3/P4/P6 use nontrivial input patterns and check whole output; choose exact F32 equality when supported, otherwise define tolerance BEFORE implementation and justify it.
For P6 verify compiled binary freshness (hash/metadata/compile invocation) to exclude accidental P4 cache reuse.
For each performance claim record machine, device load, compiler/caches, sample count; performance optimization is not a POC acceptance gate.
