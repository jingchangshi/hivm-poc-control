# P2 binary & ABI acceptance record

**Status: partially executed — scoped to what P2-A actually produced.**

Boundary, stated so no row's status is inferred from a neighbouring row's (revised 2026-10-14):

- **Executed: the build-identity rows (A2, A2-B, A3, A5, A6).** A P2 compiler *does* exist — the P2-A
  entry-only `bishengir-compile` and, since 2026-10-14, the **P2-B** `bishengir-compile` of the staged
  bundle `impl/stage-p2b-r1/` (`ea217b8d…`, D074) — and its absolute path, SHA-256, tree SHA, staged-bundle hashes and
  guarded-restore state are all measured below. This corrects the earlier blanket sentence in this
  header ("no P2 compiler … exists yet"), which was true when written and stopped being true at the
  P2-A build; leaving it would have made the A rows read as un-runnable when they had in fact run.
- **Still NOT_RUN, unchanged: A1, A4, all of B, all of C, all of D.** A4 needs the *runtime-resolved*
  `../lib` path from an executed compile; B/C/D need a real binary. **P2-A produces no binary by
  design** (the entry refuses after validation), so nothing in B/C/D can be measured yet, and the
  no-binary outcome is **not** a P2-A defect and **not** a P2 success — it is out of P2-A's scope.
- **The A-row values are P2-A-scoped and will be superseded, not silently overwritten.** P2-B rebuilds
  the compiler, so A2's identity will change; the superseded value moves to the retained-identity table
  in §0 the way the P2-A bundle's already did. **Do not read A2's recorded value as the final P2
  identity.**

Every row below is **NOT_RUN** until
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
| **compiler** — P2's own `bishengir-compile` | the P2 build from `80e29e6f27…` | A2, A3 (staged bundle), A5 (D031's pin unmodified) | **P2-B scope recorded** (`ea217b8d…` staged as `impl/stage-p2b-r1/`, from tree `cfaa59677` **with the four-file cut uncommitted** — source cut SHAs in A2). P2-A's `1a3f20f8…` retained in the superseded table above. NOT the P2-C identity: P2-C rebuilds the tail and regenerates the template |
| **compiler** — restore integrity of the P1 pin | guarded restore, `p2-architecture.md` §3.3 G1–G4 | A6 | **P2-A instance recorded** (I1–I7 PASS pre/post; G1–G4 logged; baseline re-measured `c6838fc0…`/`0761c6e6…`). Repeats at every later P2 build |
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
| `bishengir-compile` `2bf961bf80f99e7690f1def6bc7f6165bfe557404ebbd39fb81170f1ad2444c2` | the P2-A entry-only build (`ddc212e2…`), staged as `impl/stage-p2a/bin/bishengir-compile` | the Verifier's P2-A acceptance at `2026-10-13T15:24:48Z` (`p2-verification.md` §0.6) — **NOT ACCEPTED**, one real FAIL (`§0.7`, B2) | the repair-1 build `1a3f20f80d73908ce0b081608bb2af7132254261fd15f5d213e939d206df7c36`, staged as `impl/stage-p2a-r1/bin/bishengir-compile` (build-tree follow-up commit `cfaa5967…`, one file `regbase/LateInputVerifier.cpp` +45/−9) — **landed and re-verified: `p2-verification.md` §0.9, 24/24 PASS with B2 flipped FAIL→PASS on a named V7 diagnostic and both conjuncts proven by dump-suppressed probes. The design contingency was closed by the Challenger (D035) and the bundle is the accepted P2-A artifact of record (D036 — `docs/decisions.md`, scoped to entry + validator + staging identity; §0.4's strict form discharged by the standalone validator probe on the staged opt). Superseded at P2-B, not by a defect.** |
| `bishengir-compile` `1a3f20f80d73908ce0b081608bb2af7132254261fd15f5d213e939d206df7c36` `/` `bishengir-opt` `81bc93d43f37db34f33ae44f3b67ba71e9efd6bfb8a979da3715439a2d9c8b03` | the **P2-A repair-1** build (`cfaa5967…`), staged as `impl/stage-p2a-r1/` — the P2-A **artifact of record**, scoped ACCEPTED by D036 | the Verifier's P2-A re-verification (`p2-verification.md` §0.9, 24/24 PASS with B2 flipped FAIL→PASS on a named V7 diagnostic) and D035's design-contingency closure | the **P2-B** build `ea217b8d…` (`4c31167b…`), staged as `impl/stage-p2b-r1/` — superseded at P2-B **by scope, not by a defect** (D074). Retained on disk under `impl/` and pinned in `p2-bindings.json` as `superseded_p2_compiler_r1` |

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

**Structural precondition for B3/B4, measured from disk 2026-10-14 — structural only, execution still
NOT_RUN.** `find <stage> -type l` returns **nothing** for either bundle: `bin/bishengir-compile`,
`bin/bishengir-opt` and both `lib/*.bc` are **regular files** (`stat -c %y … f`). That matters because
`getLibDirFromExecutable` (`BiShengIRCompileMain.cpp:63-76`) does `real_path` (which resolves symlinks),
strips the filename, appends `../lib` — so a **symlinked** `bin/` entry would silently resolve back to the
build tree and use `build/lib`, the opposite of what staging is for. With no symlinks, `real_path` of the
staged binary is the staged path itself, so `../lib` is `<stage>/lib` **by construction**. Second half:
`<stage>/lib/` holds **exactly** the two P2-tree templates (`meta_op.aiv.c310.O2.bc` 31,206,796 B and
`…O0.bc` 27,695,180 B) and **not** the pre-D012 `44e4387b…` (31,179,496 B, 2026-10-09) that
`build/install/lib/` carries — so B7's "no old bitcode" is satisfiable from this layout. **What this does
not establish, and must not be read as establishing:** that the compiler *chose* this path at run time
(that is B3's `/proc/self/exe` observation), that O2 was *selected* over O0 (B2), or that the linked
kernel *consumed* the template (B1/B5/B6). Those remain NOT_RUN rows and require an executed compile.
The layout only removes one failure mode; it is not evidence of the selection. (Incidental, harmless and
recorded so it is not mistaken for drift: the superseded bundle's `lib/*.bc` kept the build-time mtime
`13:46` because it was staged with `cp -p`, while the r1 bundle's are `23:45` from a plain copy — the two
bundles' SHAs match, so the content is identical and the mtime difference carries no meaning.)

## A. Build identity (goal §6.1)

| # | field | value | status |
|---|---|---|---|
| A1 | P1 `bishengir-opt` SHA (historical identity, **not** P2's expected SHA) | `0761c6e662ee015e30425ee29e0c2314753aa356b21eb4119ff99747327a1977` | recorded |
| A2 | P2's own `bishengir-compile` identity: absolute path + SHA-256 + Git SHA of the tree | **P2-A scope:** path `/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline/impl/stage-p2a-r1/bin/bishengir-compile`; SHA-256 `1a3f20f80d73908ce0b081608bb2af7132254261fd15f5d213e939d206df7c36`; tree `feat/tilelang-hivm-late-pipeline` @ `cfaa59677` (parent `ddc212e29`), `git status --porcelain` **empty**; staged file 205,434,568 B (distinct from the 205,293,424 B `build/bin` pre-P2/restored binary of the same name — same name, different bytes, which is exactly why every citation here carries its path). Predecessor `2bf961bf…` @ `ddc212e2…` retained in `impl/stage-p2a/` (see §0). **Moved to §0's superseded table at P2-B (D074); this row now carries the P2-B identity.** | **recorded (P2-B scope)** |
| A2-B | P2's `bishengir-compile` identity at **P2-B** (the artifact of record, D074): absolute path + SHA-256 + tree SHA + **the uncommitted source cut it was built from** | path `/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline/impl/stage-p2b-r1/bin/bishengir-compile`; SHA-256 `ea217b8d423b2e463e8c9d3953fcf037fbf4446f0a3caa0cb21b107c85a09787`; staged file **205,436,664 B**, mtime `2026-10-14 01:00:24 +0800`; tree `feat/tilelang-hivm-late-pipeline` @ `cfaa59677`, `git status --porcelain` **not empty — four files, +82/−7** (this is the cut under test, not drift): `regbase/BiShengIRCompileMain.cpp` `324fc3a5ad164b863ff533e771cfaf188483b21b9f980c75417fa40ed721a9d2` (+16), `regbase/LatePipeline.cpp` `5cc21d7bce6ff119d99743d9174cbddd2e356635d38651b74eb51a8a71ec277c` (+54/−3), `regbase/Utility.cpp` `8ecbfc00dc2f98204bcf99b1a791d9b857f5c82d80d02bcf451497d5f8dba4ae` (+12), `hivmc/A5/HIVMCMainA5.cpp` `4b0894276de41fdeb87b060e4be2971697223db2d955cbd824355e6ce6caf340` (+7). **The four source mtimes (`00:52:44–00:53:10 +0800`) all precede the link (`01:00:24 +0800`), so the staged binary is that cut and not a stale predecessor** — measured 2026-10-14T20:55Z. **Open, and stated rather than smoothed:** the cut is uncommitted, so the source identity above is a working-tree measurement; committing it unrewritten is the implementor's outstanding deliverable and this row must be re-measured after that commit. | **recorded (P2-B scope)** — the commit half **OPEN** |
| A3 | staged bundle: `<stage>/bin/bishengir-compile` and `<stage>/bin/bishengir-opt` SHAs | **`stage-p2a-r1/`:** compile `1a3f20f8…`, opt `81bc93d43f37db34f33ae44f3b67ba71e9efd6bfb8a979da3715439a2d9c8b03`. **`stage-p2a/` (superseded):** compile `2bf961bf…`, opt `179788df75f07ca19b301eef7d787992eed428c78787914c2e92256ed1beb0cb`. **`stage-p2b-r1/` (D074 artifact of record):** compile `ea217b8d423b2e463e8c9d3953fcf037fbf4446f0a3caa0cb21b107c85a09787`, opt `4c31167bfad5243ed1bcf24efc3ccd4af65fb7cbcf5ff9468f3b2a90fa6fbc38`, O2 `5e7ccae20f4e2a1887d1bdd87f787ea97f2732f7564a67013e737a163482fb42`, O0 `0a34be8759a082d2b446e5369455d20c26d432eea19a5ca516c0e189ddafc258`; **`sha256sum -c SHA256SUMS.txt` → 4/4 OK run from that directory as its own base**, measured 2026-10-14T20:55Z; `[P2-BOUNDARY]` marker sites **6** in the compiler and **3** in the opt (D054/D061; the routing pair matches 5 of them and the sixth, `a5-compile`, has no reader — D075). `stage-p2b/` (`47e02604…`, **0** marker sites) is superseded. Both P2-A bundles re-measured from disk 2026-10-14 and both `sha256sum -c SHA256SUMS.txt` **4/4 OK run from each directory as its own base** (`cd <stage> && …`; a check run from elsewhere reports FAILED on a correct bundle — the standing rule). | **recorded (P2-A scope)** |
| A4 | staged `lib/` template SHA and the `../lib` path actually resolved at runtime | — | **NOT_RUN** — needs an *executed* compile that resolves `../lib`; the staged SHAs alone (`5e7ccae2…` O2 / `0a34be87…` O0) are recorded under A3 but are **not** this row. See §0's no-symlink structural note. |
| A5 | D031's old compiler pin unmodified | **Measured 2026-10-14 (UTC 16:36), after both P2-A builds and the guarded restore — re-run from the digest as its own base:** `sha256sum -c` on `p1v2-20261013-d025/lead/SNAPSHOT-MANIFEST-DIGEST.txt` → **47/47 OK, zero FAILED, zero warnings** (paths in that file are absolute, so the base is not a variable here — stated because it usually is). The digest's own identity is unchanged: `4dfbc856658b87034c6f2562e9aaf15863ec0c148ba1b13faa54c5eb438690a8`, and it pins `snapshot/INTEGRITY.json` = `7f9b92bf9766f008bcf23ab6d34f791a300f260b218584d2d3b2d088c32d4807`, **re-measured equal**. That file's `compiler.sha256` still names `0761c6e6…` and `compiler.linked_bitcode.sha256` still names `5e7ccae2…`, and **both paths on disk still hash to those values** (`build/bin/bishengir-opt`, `build/lib/meta_op.aiv.c310.O2.bc`). D031's pin was **not** edited to fit a P2 build. | **recorded (P2-A scope)** |
| A6 | `snapshot/INTEGRITY.json` I5 status before and after each P2 build, with the guarded restore of `p2-architecture.md` §3.3 step 4 (G1–G4) | **P2-A instance, from the implementor's records I read on disk:** I1–I7 **PASS before** the restore (`impl/scratch/integrity-pre-restore.log`) and **PASS after** (`integrity-post-restore.log`, and `p1-integrity-gate-r1.log` → `PASS: all integrity invariants hold (I1-I7)`, incl. `store-carries-l2_cache_mode=4`). Guards, per `impl/scratch/r1-guard-restore.txt` (generated `2026-10-13T15:49:04Z`): **G1** both targets rebuilt (3 build logs); **G2** restored outputs dated **older** than their link inputs — compile `13:46:21 +0800` and opt `14:46:23 +0800` vs `libBiShengIRRegBaseCompileLib.a` `23:45:15` and `…bishengir-compile.cpp.o` `23:07:56`; **G3** staged hashes + the staged opt registering the new pass (`lead/p2a-admission/help-staged-opt.txt`); **G4** the sealed `bin/`+`lib/` bundle. **Both restored identities re-measured by me 2026-10-14:** `build/bin/bishengir-compile` = `c6838fc0a175ec6bb05214780a2a3e99ab478285620996dd5bea4c69f26aabe9`, `build/bin/bishengir-opt` = `0761c6e6…` — baseline restored. The preserved copies verify 3/3 from their own base (`lead/preserved-p1-compiler/SHA256SUMS.txt`). **What this row does not claim:** that the *restore* is safe against Ninja — G2 is the recorded mitigation, and §3.3 step 4 documents that `ninja -n` cannot see the relink. | **recorded (P2-A scope + P2-B bundle)** |

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
(`third-party/llvm-project/clang/include/clang/Basic/DiagnosticFrontendKinds.td:47-48`) — and its two sites sit in the ccec **frontend**
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

**Citation base, declared 2026-10-14 after the Lead audited every path in §B and §C against the tree.**
The compiler-tree citations in this file are written relative to `bishengir/` — so
`include/bishengir/Dialect/HACC/Targets/NPUTargetSpec.td:224` is
`<tree>/bishengir/include/bishengir/Dialect/HACC/Targets/NPUTargetSpec.td:224` and
`lib/Template/lib/RegBase/Vector/include/RegBase/DMAUtils.h` is
`<tree>/bishengir/lib/Template/lib/RegBase/Vector/include/RegBase/DMAUtils.h`. The `clang/…`
citation is the exception and is now written in full, because it resolves from the repo root
through `third-party/llvm-project/clang/include/`. **Audit result:** every substantive claim and
line number in §B and §C re-verified on 2026-10-14 — `resolveTemplateBitcodeOptLevel` at
`BiShengIRCompileMain.cpp:104`; `getExtraDeviceBCPaths` at `HIVMCMainA5.cpp:110`;
`-cce-link-aicore-ll-module` at `:186`; `override-module` at `:97` and `:227`, the latter inside
the `shouldSaveLinkedIR()` command echo; the C310/m300 arms of `DMAUtils.h` at `:268`/`:295` with
the `:278-279` narrowing comment, `copy_ubuf_to_gm_align_v2` at `:284`/`:288` and
`copy_ubuf_to_gm_align` (no `_v2`) at `:308`/`:312`; `let UbSize = 2031616; // bits = 248KB` at
`NPUTargetSpec.td:224`; the numeral trap `#dlti.dl_entry<"UB_SIZE", 196608 : i32>` at
`HACCAttrs.td:309`; and the ub-overflow diagnostic at `RegBase/plan-memory-vf.mlir:453`. Three
path spellings were wrong as written and are corrected above; **no claim, value or assertion was
changed.** One imprecision left standing deliberately rather than silently tightened:
`bitcodePathList.empty()` occurs at `HIVMCMainA5.cpp:180`, **`:209` and `:767`**, so "the only hard
failure" in the paragraph above is precise only for the A5 link path; the argument it supports (an
extra bitcode makes the list non-empty, so `empty()` cannot trigger) is unaffected. Also note the
line numbers are those of the **P2 working tree** (`HIVMCMainA5.cpp` carries the uncommitted +7-line
P2 cut), so they are valid for the P2 commit and not for the pre-P2 tree.

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
| C9 | dynamic UB = 196608 kept as the **launch contract**, not mis-written as the device capacity (`UB_SIZE` = 2031616 **bits** is a different quantity). Units confirmed in-tree: `include/bishengir/Dialect/HACC/Targets/NPUTargetSpec.td:224` `let UbSize = 2031616; // bits = 248KB, reserve 8KB for compiler`, and PlanMemory's own diagnostic states bits — `bishengir/test/Dialect/HIVM/RegBase/plan-memory-vf.mlir:453` `expected-error … {{ub overflow, requires 2097152 bits while 2031616 bits available!}}`. So 2031616 bits = 253952 B = 248 KB capacity, versus the kernel's 196608 **bytes** arena. **Numeral trap:** `bishengir/include/bishengir/Dialect/HACC/IR/HACCAttrs.td:309`'s doc example writes `#dlti.dl_entry<"UB_SIZE", 196608 : i32>` — the very numeral this kernel uses for its arena — so a check that matches numerals instead of quantities would "confirm" C9 while being wrong | NOT_RUN |
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
