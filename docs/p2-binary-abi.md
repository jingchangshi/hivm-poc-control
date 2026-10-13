# P2 binary & ABI acceptance record

**Status: NOT_RUN — no P2 compiler and no P2 binary exists yet.** Every row below is **NOT_RUN** until
it carries a real command, the tool's **absolute path**, the Git SHA of the tree used, input and output
SHAs, the exit code, and the expected-vs-actual comparison. A prebuilt-ELF runtime run does not prove
CodeGen E2E, and a finished compiler or generated ELF does not prove numerical correctness — P2 requires
neither (real-NPU numerical correctness is P3), but it does require that each row be **executed** rather
than inferred. Source-inferred-only items may not be marked PASS.

## 0. Identity inventory (goal §9: 编译器/bitcode/binary 身份清单)

One index over every identity P2 must fix, so a reader (or P3) can see the whole chain in one place.
The **detail and the executed evidence live in the row cited**; this table deliberately duplicates no
values, because a second copy of a hash is a second thing that can drift.

| role | fixed by | recorded in | status |
|---|---|---|---|
| **compiler** — P1's `bishengir-opt` | D031 / `snapshot/INTEGRITY.json` I5 (`0761c6e6…`) | A1 (historical identity, **not** P2's expected SHA) | recorded |
| **compiler** — P2's own `bishengir-compile` | the P2 build from `80e29e6f27…` | A2, A3 (staged bundle), A5 (D031's pin unmodified) | NOT_RUN |
| **compiler** — restore integrity of the P1 pin | guarded restore, `p2-architecture.md` §3.3 G1–G4 | A6 | NOT_RUN |
| **bitcode** — C310 AIV template actually consumed | rebuilt from the P2 tree; path resolved from the **staged** bundle | B1, B2 (O0/O2), B3 (`/proc/self/exe`), B4 (`../lib`), B7 (no old/fallback template) | NOT_RUN |
| **bitcode** — the store ABI inside it | the template source `DMAUtils.h` at HEAD | B5 (the 4th i32 libcall argument), B6 (cache4 not cache0) | NOT_RUN |
| **bitcode** — hard failure when it is missing or wrong | the P2 entry, on the resolved path | B8 (N1–N4) | NOT_RUN |
| **binary** — the produced artifact | this P2 compile | D1–D6 (new binary, verbatim command, exit code, SHA-256, symbols, target info) | NOT_RUN |
| **binary** — hand-over to P3 | the P3 ACL harness inputs | D7 (binary, kernel symbol, HostArgs ABI, grid, dynamic UB, target info) | NOT_RUN |
| **linkage** — compiler ↔ bitcode ↔ binary | the identity sidecar the entry writes | A2/B1/D2 — one conversation: the sidecar must name the template the **staged** compiler actually resolved, not a build-tree path (see B8's binding note and `p2-verification.md` §0.1a) | NOT_RUN |

Two rules hold for every row: **the value is recorded for a file the run actually consumed**, and
**a hash copied from a document is not evidence** — it is re-measured at acceptance and the mismatch is
reported, never smoothed over.

**Superseded staged identities are retained, never overwritten** (rule added 2026-10-13, D034). A staged
bundle that has been bound and reported against by the Verifier keeps its hash in this inventory even
after it is replaced, so the identity a completed verification actually consumed stays reconstructible:

| superseded identity | what it was | bound and reported by | superseded by |
|---|---|---|---|
| `bishengir-compile` `2bf961bf80f99e7690f1def6bc7f6165bfe557404ebbd39fb81170f1ad2444c2` | the P2-A entry-only build (`ddc212e2…`), staged as `impl/stage-p2a/bin/bishengir-compile` | the Verifier's P2-A acceptance at `2026-10-13T15:24:48Z` (`p2-verification.md` §0.6) — **NOT ACCEPTED**, one real FAIL (`§0.7`, B2) | the repair-1 build `1a3f20f80d73908ce0b081608bb2af7132254261fd15f5d213e939d206df7c36`, staged as `impl/stage-p2a-r1/bin/bishengir-compile` (build-tree follow-up commit `cfaa5967…`, one file `regbase/LateInputVerifier.cpp` +45/−9) — **landed and re-verified: `p2-verification.md` §0.9, 24/24 PASS with B2 flipped FAIL→PASS on a named V7 diagnostic and both conjuncts proven by dump-suppressed probes; ACCEPT contingent on the Challenger's design confirmation, so not yet closed** |

Its companion `bishengir-opt` `179788df75f07ca19b301eef7d787992eed428c78787914c2e92256ed1beb0cb` is
superseded by `81bc93d43f37db34f33ae44f3b67ba71e9efd6bfb8a979da3715439a2d9c8b03` in the same
`stage-p2a-r1/` bundle.

**The templates were *not* regenerated — this row's contingency did not fire (measured 2026-10-13).** An
earlier version of this paragraph said that O2 `5e7ccae2…` is **also** the P1 historical pin, so *if* the
repair regenerated it the new value would have to be recorded as P2's own identity without displacing the
P1 pin's row. Measured on the r1 bundle, **both `lib/meta_op.aiv.c310.O2.bc` and `…O0.bc` are
byte-identical to the superseded bundle's** (`cd impl/stage-p2a-r1 && sha256sum -c SHA256SUMS.txt`
→ 4/4, with O2 still `5e7ccae2…` and O0 still `0a34be87…`). The repair is a `.cpp`-only change and the
incremental build log confirms it: one object file recompiled, two executables relinked, **no `.inc` and
no template bitcode rewritten**. So O2's "P1 historical pin" and "live P2-A-r1 identity" remain the same
bytes and the two claims do not need to be split here. Had they differed, the split would have been
mandatory — the two are different claims about possibly the same bytes, and this record must never
collapse them. Both bundles verify 4/4 **from their own bases**; the superseded one carries a
`SUPERSEDED.md` and is not the artifact of record.

## A. Build identity (goal §6.1)

| # | field | value | status |
|---|---|---|---|
| A1 | P1 `bishengir-opt` SHA (historical identity, **not** P2's expected SHA) | `0761c6e662ee015e30425ee29e0c2314753aa356b21eb4119ff99747327a1977` | recorded |
| A2 | P2's own `bishengir-compile` identity: absolute path + SHA-256 + Git SHA of the tree | — | NOT_RUN |
| A3 | staged bundle: `<stage>/bin/bishengir-compile` and `<stage>/bin/bishengir-opt` SHAs | — | NOT_RUN |
| A4 | staged `lib/` template SHA and the `../lib` path actually resolved at runtime | — | NOT_RUN |
| A5 | D031's old compiler pin unmodified | — | NOT_RUN |
| A6 | `snapshot/INTEGRITY.json` I5 status before and after each P2 build, with the guarded restore of `p2-architecture.md` §3.3 step 4 (G1–G4) | — | NOT_RUN |

## B. C310 bitcode identity (goal §6.2)

The chain P1 already proved exists in the template:
`l2_cache_mode → zext → shl 60 → descriptor → MOV.UB.TO.OUT.ALIGN.V2.DV`.
P2 must show the **new kernel's** chain actually consumed that template — not merely that some
standalone template has the right dataflow.

| # | check | status |
|---|---|---|
| B1 | selected C310 AIV bitcode **absolute path** and **SHA-256** | NOT_RUN |
| B2 | O0/O2 selection recorded (`resolveTemplateBitcodeOptLevel`, `regbase/BiShengIRCompileMain.cpp:104`) | NOT_RUN |
| B3 | `bishengir-compile`'s **actual** executable path (via `/proc/self/exe` → `real_path`) | NOT_RUN |
| B4 | the template **actually** selected under `../lib` | NOT_RUN |
| B5 | the **fourth i32 Store libcall argument** in the emitted LLVM IR matches the template ABI — expected value frozen from the template source below | NOT_RUN |
| B6 | `cache4` is **not** replaced by cache0 anywhere in the link/optimize chain the final kernel uses | NOT_RUN |
| B7 | no old bitcode, cached object, or fallback template is used — **two distinct C310 O2 templates exist in-tree with different SHAs**, and the second is pre-D012 (`p2-architecture.md` §3.3) | NOT_RUN |
| B8 | **missing required C310 bitcode in Late mode fails hard** — expected: a diagnostic naming the missing required template and **no binary**. This is **four separate negatives**, not one combined case (`p2-architecture.md` §3.3): N1 required template absent; N2 a template present but **not the recorded identity** (this is the row that catches the pre-D012 `44e4387b…`); N3 `--link-aicore-bitcode` supplied; N4 absent *and* an extra supplied — rejected with no binary and **without prescribing which diagnostic fires**. Today's behaviour (warning at `HIVMCMainA5.cpp:112-114` plus a binary) is a **hypothesis predicted from source, NOT_RUN until executed**, and is recorded as a **finding about the existing route**, not fixed for the non-Late route. An unexecuted successful-link outcome stays NOT_RUN, never PASS. | NOT_RUN |

**A correction carried into this file: `-Wno-override-module` is not a duplicate-symbol suppression.**
An earlier draft of `p2-architecture.md` §3.3 claimed the flag let a user-supplied bitcode define the
same symbols as the required template. Measured, the flag is `DiagGroup<"override-module">` whose only
diagnostic is *"overriding the module target triple with %0"*
(`clang/Basic/DiagnosticFrontendKinds.td:47-48`) — and its two sites sit in the ccec **frontend**
argument list (`HIVMCMainA5.cpp:97`, next to `-cce-bitcode-is-aicore`) and in the `--save-linked-ir`
command echo (`:227`). The substitution vector is fully explained without it, by the list-based link
path (`getExtraDeviceBCPaths()` seeds the list at `:110`; the meta-op path is appended only when the
core type is in `bitcodePaths`, warning otherwise at `:112-114`; `bitcodePathList.empty()` at `:180` is
the only hard failure and cannot trigger once an extra is supplied; every path is linked as
`-cce-link-aicore-ll-module <BCPath>` at `:185-188`). **A negative asserting the flag's absence would
therefore prove nothing** — it would pass on a build that still substituted a template.

**B-row preconditions, measured 2026-10-13 (`p2-architecture.md` §3.3).** Two facts bound what B1–B7
can claim, and both must be settled by *execution*, not by path:

1. `build/lib/meta_op.aiv.c310.O2.bc` = `5e7ccae20f4e2a18…` (31,206,796 B, 13:46) is the P1-pinned
   P2-tree template; `build/install/lib/meta_op.aiv.c310.O2.bc` = `44e4387baea74c96…` (31,179,496 B,
   **2026-10-09** 15:01) is a **pre-D012** template that a bundle staged from `build/install/{bin,lib}`
   would resolve to silently through `getLibDirFromExecutable`'s real-executable-relative `../lib`.
2. The pinned P2-tree template predates the committed template source: `RegBase/Vector/include/RegBase/DMAUtils.h`
   was written at 14:42, the per-file template bitcodes at 13:37:55 and the merged one at 13:46:45, and
   no `RegBase/Vector/*.bc` was regenerated after 14:00. So B1's SHA-256 must be recorded for a template
   **rebuilt from the P2 tree**, and B4 must show `../lib` resolving to *that* path.

**B5's expected value, frozen from the template source before any implementation.** The store ABI is
defined by `lib/Template/lib/RegBase/Vector/include/RegBase/DMAUtils.h`, which the D012 commit edits.
Measured at HEAD:

| | C310 arm (`#if defined(__DAV_C310__)`, `:268-294`) | non-C310 / m300 arm (`:295-319`) |
|---|---|---|
| core function | `store_ubuf_to_gm_intrin_core(ub_ptr, ub_offset, gm_ptr, gm_offset, burst_cnt, burst_len, src_gap, dst_gap, uint32_t l2_cache_mode)` — **9 params** | same **minus** `l2_cache_mode` — **8 params** |
| emitted intrinsic | `copy_ubuf_to_gm_align_v2` (`:284`, `:288`) — note the **`_v2`**, matching P1's `MOV.UB.TO.OUT.ALIGN.V2.DV` | `copy_ubuf_to_gm_align` (`:308`, `:312`) — **no** `_v2` |
| positional args | `(gm_ptr+off, ub_ptr+off, 0, burst_cnt, burst_len, l2_cache_ctl, dst_stride, src_stride)` | `(gm_ptr+off, ub_ptr+off, 0, burst_cnt, burst_len, src_align_gap, dst_gap)` |
| the policy | `uint8_t l2_cache_ctl = static_cast<uint8_t>(l2_cache_mode);` — comment `:278-279`: *"Narrowed to the descriptor field width only here; the policy is 32-bit through the whole public/forwarding ABI."* | absent |

So the goal §6.2 phrase "第四个 i32 Store libcall 参数" resolves to **`l2_cache_ctl`**: in
`(ptr, ptr, i32=0, i32=burst_cnt, i32=burst_len, i32=l2_cache_ctl, …)` it is the **4th i32-typed**
argument. Two cautions, both to be settled on the **emitted LLVM IR**, not on this table:

- The two arms are **not** "the same call plus one argument": the 7th/8th arguments differ in both
  name and value (`dst_stride, src_stride` vs `src_align_gap, dst_gap`), so a diff that only checks the
  argument count would pass on a wrong call.
- `l2_cache_ctl` is also the spelling the frozen artifact's stale note calls a *discardable op
  attribute* (see `p2-architecture.md` §2.4). In the **template** it is the legitimate narrowed local
  variable. The names collide; the meanings do not.

**C4's expected value, same source:** the non-C310 arm keeps the **8-parameter** core and the
**non-`_v2`** intrinsic with no cache argument — that is what "非 C310 路径保留旧 ABI" means
concretely, and it must be shown on a real non-C310 compile, not inferred from this table.

## C. LLVM and Kernel ABI — the 11 enumerated checks (goal §6.3)

Each is checked **from the real compile output**, not from the source that produces it.

| # | check | status |
|---|---|---|
| C1 | device entry corresponds to **three GM pointer parameters** | NOT_RUN |
| C2 | Kernel symbol agrees with the information **FunctionInfo** requires | NOT_RUN |
| C3 | AIV core type and Ascend950PR target correct | NOT_RUN |
| C4 | **no additional device-entry parameters** are produced | NOT_RUN — "device entry" here is the three-GM-pointer kernel entry (C1), distinct from the non-C310 store ABI of goal §7 C, whose expected value is frozen below |
| C5 | `blockIdx` acquisition and GM offsets match the Golden | NOT_RUN |
| C6 | pb32 lowering result **stays b32** | NOT_RUN |
| C7 | DMA load/store addresses and ranges as expected | NOT_RUN |
| C8 | `set_flag`/`wait_flag` and the **terminal barrier** are neither wrongly deleted nor reordered | NOT_RUN |
| C9 | dynamic UB = 196608 kept as the **launch contract**, not mis-written as the device capacity (`UB_SIZE` = 2031616 **bits** is a different quantity). Units confirmed in-tree: `include/bishengir/Dialect/HACC/Targets/NPUTargetSpec.td:224` `let UbSize = 2031616; // bits = 248KB, reserve 8KB for compiler`, and PlanMemory's own diagnostic states bits — `bishengir/test/regbase/Dialect/HIVM/RegBase/plan-memory-vf.mlir:453` `expected-error … {{ub overflow, requires 2097152 bits while 2031616 bits available!}}`. So 2031616 bits = 253952 B = 248 KB capacity, versus the kernel's 196608 **bytes** arena. **Numeral trap:** `HACCAttrs.td:309`'s doc example writes `#dlti.dl_entry<"UB_SIZE", 196608 : i32>` — the very numeral this kernel uses for its arena — so a check that matches numerals instead of quantities would "confirm" C9 while being wrong | NOT_RUN |
| C10 | no unlegalized HIVM/HIVMAVE ops and no unrealized casts remain | NOT_RUN |
| C11 | the real `ConvertHIVMToLLVM` ran **exactly once** (cross-ref `docs/p2-pass-trace.md` C2) | NOT_RUN |

C8 inherits the P1 contract gate's **semantic** requirement: the terminal barrier check is
**entry-scoped**. The artifact declares a second function (`@main_kernel_simd_vf_0`), and a barrier
moved into that helper must not satisfy it. The negative is
`verify/tests/negatives/b07_pipe_all_moved_into_vf.mlir`.

**C9's assertion boundary is narrower than "kept as the launch contract" suggests, and that must be
stated or the row will be read as requiring something AscendNPU-IR does not do.** `dyn_shared_memory_buf`
has **zero** consumers anywhere in the AscendNPU-IR tree (grep over `*.cpp`/`*.h`/`*.td`/`*.py`: no
hits). It is a TileLang/TVM-side PrimFunc attribute: written by
`tilelang/src/transform/lower_device_kernel_launch.cc:305`, read by
`tilelang/tilelang/jit/adapter/wrapper.py:488-489`, and consumed at runtime by
`tilelang/src/ascend/runtime/ascend_module.cc:482-489`, which sets
`kAclLaunchKernelAttrDynUbufSize`. So within P2 the row can assert exactly two things, both on the
**emitted artifact**: the func attribute is still present on the entry with value `196608`, and the
device-spec `UB_SIZE` entry is still `2031616` (bits). The launch-contract consumption is a
TileLang-runtime and P3 concern; a P2 check that hunts for a compiler-side reader will find none and
must not read that absence as a failure. The frozen artifact records this too, including its own
correction of an earlier claim (`vadd_late.mlir:38-46`: "an earlier note here claimed the arena 'has no
carrier in the Late IR'. That was WRONG — it conflated 'no consumer in AscendNPU-IR' (true) with 'no IR
carrier' (false)").

## D. Final artifact authenticity (goal §7 D)

| # | requirement | status |
|---|---|---|
| D1 | a **new, real** A5 AIV kernel binary is produced | NOT_RUN |
| D2 | reproducible command recorded verbatim | NOT_RUN |
| D3 | exit code recorded | NOT_RUN |
| D4 | binary SHA-256 recorded | NOT_RUN |
| D5 | symbols recorded | NOT_RUN |
| D6 | target information recorded | NOT_RUN |
| D7 | the P3 ACL harness inputs the binary must hand over are identified: binary, kernel symbol, HostArgs ABI, grid, dynamic UB, target info | NOT_RUN |

## E. Standing prohibitions for this file

- No credentials, host environment secrets, or account tokens appear in any recorded command or log.
- No large raw binary is committed to a public repository; the binary and its logs live under
  `/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline/`, which is **local-only**.
- Exit code 0 is not acceptance; every row states expected vs actual.
