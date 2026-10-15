# P2 binary & ABI acceptance record

**Status: partially executed — scoped to what P2-A actually produced.**

Boundary, stated so no row's status is inferred from a neighbouring row's (revised 2026-10-14):

- **Executed: the build-identity rows (A2, A2-B, A3, A5, A6).** A P2 compiler *does* exist — the P2-A
  entry-only `bishengir-compile` and, since 2026-10-14, the **P2-B** `bishengir-compile` of the staged
  bundle `impl/stage-p2b-r1/` (`ea217b8d…`, D074) — and its absolute path, SHA-256, tree SHA, staged-bundle hashes and
  guarded-restore state are all measured below. This corrects the earlier blanket sentence in this
  header ("no P2 compiler … exists yet"), which was true when written and stopped being true at the
  P2-A build; leaving it would have made the A rows read as un-runnable when they had in fact run.
- **Still NOT_RUN, unchanged: A4, all of B, all of C, all of D.** A4 needs the *runtime-resolved*
  `../lib` path from an executed compile; B/C/D need a real binary **and a frozen, independently owned
  test that consumes it**. **P2-A produced no binary by design** (the entry refuses after validation),
  so nothing in B/C/D could be measured at P2-A, and that no-binary outcome was **not** a P2-A defect
  and **not** a P2 success — it was out of P2-A's scope. Since P2-B a real binary does exist (the
  staged bundle's `bishengir-compile`), so the B/C/D rows now read NOT_RUN **for want of an executed
  frozen consumer, not for want of a binary** — do not read this list as "there is nothing to test
  against". The implementor's four-arm smoke (`impl/scratch/p2c-smoke/`, run of record
  `late-20261014T005329Z`) is engineering scaffolding and is **not** acceptance evidence for any row.
  *(A1 left this list at this edit: its row has read `recorded` since P2-A — re-measured 2026-10-14 at
  `lead/preserved-p1-compiler/bishengir-opt.0761c6e6`, 214,509,632 B — so the header list had gone
  stale and contradicted its own table. Corrected here rather than left standing.)*
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
values, because a second copy of a hash is a second thing that can drift. The single exception is the
**template split** below (P2-C), where *two* identities must be read as a pair and the pairing itself is
the fact; those six values live there and nowhere else in this document, so each still has exactly one
home.

| role | fixed by | recorded in | status |
|---|---|---|---|
| **compiler** — P1's `bishengir-opt` | D031 / `snapshot/INTEGRITY.json` I5 (`0761c6e6…`) | A1 (historical identity, **not** P2's expected SHA) | recorded |
| **compiler** — P2's own `bishengir-compile` | the P2 build from `80e29e6f27…` plus the P2-C tail cut | A2, A3 (staged bundle), A5 (D031's pin unmodified) | **P2-C scope recorded (current):** `812c5cb1d29cb3b8879593ea77d08ebdae274cc943d41523c7c4bab739cbf455`, 205,469,200 B, staged as `impl/stage-p2c-r1/`, built from commit B `4495ab29b` on `feat/tilelang-hivm-late-pipeline` (the five-file cut; source pin = the v3 snapshot `P2C-SOURCE-SNAPSHOT-v3-committed.txt`). Supersedes **by scope, not by a defect** the P2-B identity `ea217b8d…` (`impl/stage-p2b-r1/`, tree `cfaa59677` **with the four-file cut uncommitted** — source cut SHAs in A2), which is retained on disk but is **not** entered in the superseded table above: that table's membership rule is "bound and reported against by the Verifier", and no Verifier binding for P2-B exists (`p2-verification.md` carries no P2-B row). P2-A's `1a3f20f8…` stays in that table. **P2-C is the identity that regenerates the templates — see the split below before quoting any `lib/*.bc` hash.** |
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

**The template split — it did NOT fire at P2-A, and it DID fire at P2-C. Two rows, never collapsed.**

*(a) Scoped to P2-A; measured 2026-10-13, still true of those two bundles.* An earlier version of this
paragraph said that O2 `5e7ccae2…` is **also** the P1 historical pin, so *if* the repair regenerated it
the new value would have to be recorded as P2's own identity without displacing the P1 pin's row.
Measured on the r1 bundle, **both `lib/meta_op.aiv.c310.O2.bc` and `…O0.bc` are byte-identical to the
superseded bundle's** (`cd impl/stage-p2a-r1 && sha256sum -c SHA256SUMS.txt` → 4/4, with O2 still
`5e7ccae2…` and O0 still `0a34be87…`). The repair is a `.cpp`-only change and the incremental build log
confirms it: one object file recompiled, two executables relinked, **no `.inc` and no template bitcode
rewritten**. So *for the P2-A bundles* O2's "P1 historical pin" and "live P2-A-r1 identity" were the same
bytes and the two claims did not need splitting there. Both bundles verify 4/4 **from their own bases**;
the superseded one carries a `SUPERSEDED.md` and is not the artifact of record.

*(b) The P2-C split — the contingency fired; measured 2026-10-14.* P2-C rebuilds the tail, so its staged
bundle carries templates of its **own**, and they are **not** the P1 pins. Both rows are true at once,
about different bytes:

| claim | file | sha256 | bytes |
|---|---|---|---|
| **P1 historical pin** (preserve as bytes — D031/D088) | `AscendNPU-IR-1/build/lib/meta_op.aiv.c310.O2.bc`; preserved copy `lead/preserved-p1-compiler/meta_op.aiv.c310.O2.bc.5e7ccae2` | `5e7ccae20f4e2a1887d1bdd87f787ea97f2732f7564a67013e737a163482fb42` | 31,206,796 |
| **P1 historical pin**, O0 | `AscendNPU-IR-1/build/lib/meta_op.aiv.c310.O0.bc` | `0a34be8759a082d2b446e5369455d20c26d432eea19a5ca516c0e189ddafc258` | 27,695,180 |
| **P1 historical pin**, linked | `AscendNPU-IR-1/build/lib/meta_op.aiv.c310.O2.bc.linked.bc` | `1b356045e047cb31cf9306d5c0fd387622ceb64c6bc127f57bcc637f4ad5486e` | 31,180,028 |
| **P2-C's own template** (this build's identity) | `impl/stage-p2c-r1/lib/meta_op.aiv.c310.O2.bc` | `3807eb3d907236b17855db57a475ea120c37a74905c92bb4b971b415da7d6f79` | 31,206,992 |
| **P2-C's own template**, O0 | `impl/stage-p2c-r1/lib/meta_op.aiv.c310.O0.bc` | `b8c68dcb1ef506464c2a27d83178772386e9cf65a834b9a8c9a71c8a41f27078` | 27,695,840 |
| **P2-C's own template**, linked | `impl/stage-p2c-r1/lib/meta_op.aiv.c310.O2.bc.linked.bc` | `cbd56bd926f847f485f958d7358579f476bb0f41f345a6074fdcf5f2c0ae84c4` | 31,180,672 |

All six re-measured from disk 2026-10-14 (the three P2-C values also verify 4/4 by
`sha256sum -c SHA256SUMS.txt` **from the bundle's own base**). Three consequences the collapsed
single-row form would have hidden:

1. **The P1 pin has no rebuild path (D091), so byte preservation is the only way to keep it.** The pin
   was compiled from a *pre-commit working-tree state* of one header,
   `bishengir/lib/Template/lib/RegBase/Vector/include/RegBase/DMAUtils.h`, that is no longer on disk:
   no committed revision reproduces it (the P1-era object build window 2026-10-13 13:37:55–13:46:02
   precedes that header's worktree mtime 14:42:53, which precedes the only commit touching it since,
   `80e29e6f2` at 19:47:10). Therefore it must **never** be "verified" by rebuilding it — a rebuild
   yields the P2-C row, and reading that as a pin mismatch would be a **false FAIL**.
2. **The delta is real, measured, and is debug metadata only — a withdrawn attribution is recorded here
   so it is not re-derived.** +196 B (O2) / +660 B (O0) / +644 B (linked), all of it the **debug
   line/column/scope-line tables of that one header**; the instruction streams diff to **0 lines** and
   the 1,677 `llvm.hivm.store.vfsimt.info` descriptor calls are byte-identical
   (`impl/scratch/p2c-template-probe/TEMPLATE-DIFFERENTIAL-PROBE.txt`). An earlier note attributing the
   delta to `80e29e6f2`'s cache-mode change is **withdrawn** — the pin already carries
   `i32 noundef %l2_cache_mode` in the same 39 define-lines, and that attribution would have
   false-FAILed a correct emitter. *Disclosed limits, not smoothed:* the comparison is over **re-emitted**
   IR (both sides passed through the frontend's own pipeline, so a difference it normalises away on both
   sides would be invisible), and the **O0 differential is NOT_RUN** (the re-emitter dies on both O0
   files) — §3/§5 of the probe are measured for **O2 only**, which is the level the Late route resolves.
3. **A Late-mode positive row must consume the P2-C row from a bundle that carries it.** The build
   tree's `build/lib` still holds the **P1 pin** by design (D090), so the P2-C identity gate fails
   closed there: a positive row aimed at `build/bin` (or at a symlinked `bin/` that re-points `../lib`
   at the build tree) is a **false FAIL waiting to be written**.

**Structural precondition for B3/B4, measured from disk 2026-10-14 — structural only, execution still
NOT_RUN.** *(The paragraph below is scoped to the two **P2-A** bundles, which is what was on disk when
it was written; the P2-C sub-paragraph that follows supersedes nothing in it but does not inherit its
file list.)* `find <stage> -type l` returns **nothing** for either P2-A bundle: `bin/bishengir-compile`,
`bin/bishengir-opt` and both `lib/*.bc` are **regular files** (`stat -c %y … f`). That matters because
`getLibDirFromExecutable` (`BiShengIRCompileMain.cpp:63-76`) does `real_path` (which resolves symlinks),
strips the filename, appends `../lib` — so a **symlinked** `bin/` entry would silently resolve back to the
build tree and use `build/lib`, the opposite of what staging is for. With no symlinks, `real_path` of the
staged binary is the staged path itself, so `../lib` is `<stage>/lib` **by construction**. Second half:
each P2-A `<stage>/lib/` holds **exactly** the two templates of that era (`meta_op.aiv.c310.O2.bc`
31,206,796 B and `…O0.bc` 27,695,180 B — the **P1-pin** bytes, see the split above) and **not** the
pre-D012 `44e4387b…` (31,179,496 B, 2026-10-09) that `build/install/lib/` carries — so B7's "no old
bitcode" is satisfiable from this layout. **What this does not establish, and must not be read as
establishing:** that the compiler *chose* this path at run time (that is B3's `/proc/self/exe`
observation), that O2 was *selected* over O0 (B2), or that the linked kernel *consumed* the template
(B1/B5/B6). Those remain NOT_RUN rows and require an executed compile. The layout only removes one
failure mode; it is not evidence of the selection. (Incidental, harmless and recorded so it is not
mistaken for drift: the superseded bundle's `lib/*.bc` kept the build-time mtime `13:46` because it was
staged with `cp -p`, while the r1 bundle's are `23:45` from a plain copy — the two bundles' SHAs match,
so the content is identical and the mtime difference carries no meaning.)

**The same structural measurement on the P2-C bundle, and one difference that matters (measured
2026-10-14).** `find impl/stage-p2c-r1 -type l` returns **nothing**, so the B3/B4 construction argument
above holds unchanged for P2-C. Three facts, however, are *not* inherited from the P2-A bundles and must
be read from the bundle itself:

1. **`bin/` holds only `bishengir-compile`** (`812c5cb1…`, 205,469,200 B). There is **no
   `bin/bishengir-opt`** in the P2-C bundle, unlike the P2-A/P2-B bundles. A row (or a reader) looking
   for the compiler pair here will not find it; `bishengir-opt` is governed by A1 and still lives at
   `lead/preserved-p1-compiler/bishengir-opt.0761c6e6` (214,509,632 B) and in `build/bin`.
2. **`lib/` holds three files, not two**: the O2 and O0 templates of the P2-C row, **plus**
   `meta_op.aiv.c310.O2.bc.linked.bc` (`cbd56bd9…`, 31,180,672 B) — a member the P2-A bundles did not
   carry. So "`lib/` holds exactly the two templates" is a P2-A-scoped sentence; for P2-C the correct
   statement is "the three AIV-C310 artefacts of the P2-C row, and nothing else" (verified: the
   directory listing is exactly those three files).
3. **The staged compiler *was* a hard link and no longer is — the hazard was found and fixed while this
   section was being written; both states are recorded, with the fix's receipt.** *Before* (measured
   09:17–09:23): `stat -c '%i %h'` gave inode `161332346` with **link count 7**, the other six being
   `bin/bishengir-compile` under `impl/scratch/p2c-smoke/{run-20261014T005154Z,
   late-20261014T005231Z, late-20261014T005329Z}/{C,D}/`, and a `find
   /home/s00653124/workspace/hivm-poc-evidence /home/s00653124/workspace/AscendNPU-IR-1/build -inum
   161332346` returned **exactly those seven** — so the artifact of record was **not** hard-linked to the
   build tree (it was never the restored `c6838fc0…`); the sharing was confined to the evidence root's
   own smoke scratch. *After* (measured by me at 09:24, receipted independently by the implementor's
   `impl/scratch/p2c-build/s5-break-bundle-bin-hardlink.{sh,log}` at 09:23:53–59 and
   `s5-bundle-bin-identity.txt`): the file was replaced by a verified copy — **link count 1, inode
   `161332350`**, content **unchanged** at `812c5cb1…` / 205,469,200 B — and the bundle's own
   `sha256sum -c SHA256SUMS.txt` reads **4/4 OK** from its base. The six scratch copies still share the
   old inode `161332346` among themselves; that is scratch, not evidence of record. Two consequences
   stated plainly: (i) a hard link never broke the `../lib` argument — `real_path` resolves *symlinks*
   only, so the staged path stayed the staged path and `../lib` was `<stage>/lib` throughout, and the
   arms' own `lib/` directories are separate; (ii) but the seven paths were **one inode**, so an
   in-place write (or a `chmod`) through *any* of them would have changed the artifact of record for all
   of them — which is why the fix was made at the **sealed** end rather than by copying the arms.
   **D079 warning, taken from this paragraph's own history:** its first draft stated link count 7 as a
   *current* fact and was voided by that legitimate fix within four minutes. Read both states, and
   re-measure `stat -c '%h %i %F'` rather than quoting either number as the present one.
   *A note on how this was found, because it changes how identities are read here:* the `sha256sum` of an
   unrelated `lib` member (`meta_op.mix.aiv.c310.O2.bc.linked.bc`) came back through tool output with a
   **masker placeholder inside the hex string**, part of the hash replaced in flight. The same file
   re-hashed with the digest grouped two characters at a time printed the untouched 64-hex value — so the
   file is intact and only the *rendering* was altered. **Identity values are therefore never sealed from
   a single ungrouped reading**: re-measure through a rendering that defeats the masker, exactly as the
   six values in the split table were.

## A. Build identity (goal §6.1)

| # | field | value | status |
|---|---|---|---|
| A1 | P1 `bishengir-opt` SHA (historical identity, **not** P2's expected SHA) | `0761c6e662ee015e30425ee29e0c2314753aa356b21eb4119ff99747327a1977` | recorded |
| A2 | P2's own `bishengir-compile` identity: absolute path + SHA-256 + Git SHA of the tree | **P2-A scope:** path `/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline/impl/stage-p2a-r1/bin/bishengir-compile`; SHA-256 `1a3f20f80d73908ce0b081608bb2af7132254261fd15f5d213e939d206df7c36`; tree `feat/tilelang-hivm-late-pipeline` @ `cfaa59677` (parent `ddc212e29`), `git status --porcelain` **empty**; staged file 205,434,568 B (distinct from the 205,293,424 B `build/bin` pre-P2/restored binary of the same name — same name, different bytes, which is exactly why every citation here carries its path). Predecessor `2bf961bf…` @ `ddc212e2…` retained in `impl/stage-p2a/` (see §0). **Moved to §0's superseded table at P2-B (D074); this row now carries the P2-B identity.** | **recorded (P2-B scope)** |
| A2-B | P2's `bishengir-compile` identity at **P2-B** (the artifact of record, D074): absolute path + SHA-256 + tree SHA + **the uncommitted source cut it was built from** | path `/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline/impl/stage-p2b-r1/bin/bishengir-compile`; SHA-256 `ea217b8d423b2e463e8c9d3953fcf037fbf4446f0a3caa0cb21b107c85a09787`; staged file **205,436,664 B**, mtime `2026-10-14 01:00:24 +0800`; tree `feat/tilelang-hivm-late-pipeline` @ `cfaa59677`, `git status --porcelain` **not empty — four files, +82/−7** (this is the cut under test, not drift): `regbase/BiShengIRCompileMain.cpp` `324fc3a5ad164b863ff533e771cfaf188483b21b9f980c75417fa40ed721a9d2` (+16), `regbase/LatePipeline.cpp` `5cc21d7bce6ff119d99743d9174cbddd2e356635d38651b74eb51a8a71ec277c` (+54/−3), `regbase/Utility.cpp` `8ecbfc00dc2f98204bcf99b1a791d9b857f5c82d80d02bcf451497d5f8dba4ae` (+12), `hivmc/A5/HIVMCMainA5.cpp` `4b0894276de41fdeb87b060e4be2971697223db2d955cbd824355e6ce6caf340` (+7). **Provenance, stated at the strength measured — the claim was NARROWED on 2026-10-14 by the Challenger's ruling 1(a), and the artifact did not change.** The four source mtimes (`00:52:44–00:53:10 +0800`) all precede the link (`01:00:24 +0800`), and that alone does **not** establish that each translation unit was recompiled: mtime order is consistent with a stale object or an incremental relink. **The stronger evidence is per-edge and was measured in the build's own log** (`impl/scratch/p2b-r1-build.log`, sha256 `42ac33eb29bca1d397d5d2fdf46cebb7d72fecd8f3994eac04d5e0e0f264514b`, 2479 lines, mtime = link time, tail `NINJA_RC=0`): `Utility.cpp.o` built at edge `[1326/1334]`, `LatePipeline.cpp.o` `[1327/1334]`, `BiShengIRCompileMain.cpp.o` `[1328/1334]`, then `libBiShengIRRegBaseCompileLib.a` relinked `[1329/1334]`, `HIVMCMainA5.cpp.o` `[1330/1334]`, `libBiShengIRHIVMCLibA5.a` `[1331/1334]`, `bin/bishengir-compile` `[1332/1334]`. The same build rebuilt or relinked **243/243** of the paths the D041 host-build incident had dirtied (**0** unaccounted), so no host-compiled object survived into the link. **What is still NOT claimed, deliberately:** the log cannot show the compiler did fresh work rather than take a **ccache** hit (a hit is invisible in ninja output), so this row's claim is *exact bytes + per-edge build provenance*, never "proven freshly compiled". A cache hit is immaterial to content identity anyway, since ccache is keyed on the preprocessed source. The related host-run 0.63 s burst anomaly is D041's **open Q8** and concerns the host artifacts, all of which this build replaced. **Source-snapshot manifest:** `lead/scratch-p2b-source-snapshot-20261014/` (base HEAD, the four file hashes, the diff hashed as bytes `89d647430cfcb69a1be11e831225c6f0c2cf87b1e0cd034365b0fe1c94d9dd89`, diffstat `4 files changed, 82 insertions(+), 7 deletions(-)`, the compiler SHA, the build command/provenance, and a no-post-build-change predicate; `sha256sum -c SHA256SUMS.txt` 3/3 OK from its own base). **The cut stays uncommitted by decision, not by omission** — the Challenger ruled (1(c)) that a commit is **not required** and does not itself invalidate the provenance; **content/snapshot identity is the pin**, and manifest plus `[1]–[3]` void the whole row if the worktree changes. D041's restore-`build/bin`-after-the-last-build sequencing is why the tree is still dirty. | **recorded (P2-B scope)** — uncommitted **by decision** (ruling 1(c)), pinned by the snapshot manifest |
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

## F. Append-only errata (record-not-repair)

This section is **appended**; nothing above it is rewritten. Each item names the exact bytes it
corrects and the measurement that corrects them, so a reader who has already read the body can see
which sentence is superseded without re-reading the whole file.

**E-1 — §3 item 2's headline folds three rows into one claim; only the O2 row is measured
(corrects `docs/p2-binary-abi.md:113`).** The sentence *"+196 B (O2) / +660 B (O0) / +644 B
(linked), **all of it** the debug line/column/scope-line tables of that one header"* asserts a
property of all three deltas while the same subsection's disclosed limit (`:121-123`) withdraws two
of them. Corrected status, row by row:

| row | delta | status |
|---|---|---|
| O2 (`meta_op.aiv.c310.O2.bc`) | +196 B | **MEASURED** — re-emitted IR comparison, instruction streams diff to 0 lines, 1,677 `llvm.hivm.store.vfsimt.info` descriptor calls byte-identical |
| O0 (`meta_op.aiv.c310.O0.bc`) | +660 B | **NOT_RUN** — the re-emitter dies on **both** O0 files (`fatal error: error in backend: 64-bit code requested on a subtarget that doesn't support it!`); no measurement of the delta's kind exists |
| linked (`…O2.bc.linked.bc`) | +644 B | **NOT_RUN** — no row in this phase reads the linked pair |

So `all of it` is not established. The *conclusion* the item exists to record — that the delta is
**not** the cache-mode change, i.e. that the withdrawn attribution to `80e29e6f2` stays withdrawn —
is unaffected: it rests on the pin already carrying `i32 noundef %l2_cache_mode` in the same 39
define-lines, which is measured independently of any of the three size rows (and re-measured at
104 consumption sites in both templates; see `docs/decisions.md` D101(b)). Only the **size rows**
are demoted.
*Evidence:* `impl/scratch/p2c-template-probe/TEMPLATE-DIFFERENTIAL-PROBE.txt` §7.1 (the O0
re-emission failure); the probe's §3/§5 (O2-only); `docs/p2-binary-abi.md:121-123` (its own caveat);
`docs/decisions.md` D101. *Source of the correction:* the step-8 Challenger `review-2`, reproduced
by the Lead from the probe text and the doc's own caveat (`lead/r9-review/STEP8-VERDICT-INTERIM.txt`
V-5, `STEP8-VERDICT-FINAL.txt`).

**E-2 — this file is content-identified elsewhere, and this append changes that identity
(disclosure, not a defect).** `verify/p2c-freeze/P2C-TEST-FREEZE.txt:1366` and eight later freeze
revisions record this file as `41473 B / 8d384b28…`, under a heading that says the rows are recorded
*"so a later reader can tell whether the row mappings above still describe the documents they were
read from."* Those rows are a **point-in-time census, not a live guard** (no script in
`verify/p2c-freeze/` re-hashes this file at run time), so this append does not break a running check
— but it does mean every one of those rows now describes a **superseded generation** of this file.
A reader comparing the freeze's recorded hash to the file on disk will see a mismatch; that mismatch
is **this errata section and nothing else**. The pre-append bytes are preserved at
`lead/r9-review/pre-errata-snapshot/p2-binary-abi.md.pre-errata` (`41473 B / 8d384b28…`) and this
append's own boundary is provable by prefix hash (`head -c 41473` of the new file = `8d384b28…`).

**E-3 — §3 item 2's "104 consumption sites" is not a measured count; the measured count is 416,
and the claim it supports is unaffected.**

Appended 2026-10-14T19:57Z (`date -u` immediately before this write).

`E-1`'s replacement text (the paragraph ending at `:378`) reads "…and re-measured at **104**
consumption sites in both templates; see `docs/decisions.md` D101(b)". That literal does not
reproduce. Measured on the probe's own re-emitted IR, with the predicate stated so it can be re-run:

    grep -c 'zext i32 %l2_cache_mode to i64'  dis15/p1.o2.ll   ->  416
    grep -c 'zext i32 %l2_cache_mode to i64'  dis15/p2.o2.ll   ->  416

The chain those `zext`s feed (`shl i64 <that>, 60`, matched SSA-to-SSA) is also **416** in each file,
as are the `MOV.UB.TO.OUT.ALIGN.V2.DV` descriptor calls that consume the result (**1331** each) and
the `llvm.hivm.store.vfsimt.info` calls (**1677** each). **No predicate I could construct over either
file yields 104.** The parameter appears in **39** `define` blocks (4 chains each; 39 x 4 = 416), so
104 is neither the chain count nor the block count. `416 / 4 = 104` exactly, and the literal most
plausibly entered the Lead's notes as a unit slip from that division. The probe document does **not**
carry it: `TEMPLATE-DIFFERENTIAL-PROBE.txt:74` states the **39** define-lines, which is the figure
that does reproduce.

**Provenance and reach of the wrong literal.** It originated in the Lead's own working notes for the
step-9 closeout and propagated, by my hand, into four record sites: this file, `docs/decisions.md`
D101(b), and `docs/status.md` 追加 222 (SS5, the R1-boundary paragraph) and 追加 225 (SS2(a)). It is
corrected by appending here and by `docs/decisions.md` D111; the two `status.md` sites are corrected
by 追加 230. The frozen suite and the Challenger's sealed verdict never carried it — a search over
`verify/p2c-freeze/` and `lead/r9-review/STEP8-VERDICT-FINAL.txt` returns 0 hits.

**The claim it supports is unaffected.** What item 2 needs is that the P1 pin **already carries** the
4-parameter store and the same cache-mode dataflow — that is what keeps the withdrawn `80e29e6f2`
attribution withdrawn. That rests on the **byte-identical** store signature at the **same line
743742** in both `.ll` files and on `instr.diff` = **0 bytes**, not on any count. The count was
decoration; the identity is the evidence. So this is a **literal** correction, not a conclusion
change, and the conclusion stands: the O2 size delta is a debug-table delta, not a cache-mode change.

*Basis:* `impl/scratch/p2c-template-probe/dis15/{p1,p2}.o2.ll` (1,666,906,685 / 1,666,906,814 B), the
commands quoted above, and `impl/scratch/p2c-template-probe/TEMPLATE-DIFFERENTIAL-PROBE.txt:74`.

## 0bis. Final P2 identity inventory — appended at step 9 (2026-10-15)

*Append-only. Every row above this heading is preserved verbatim, including the six `NOT_RUN`
rows in §0's index table and the superseded-identity table; this section does not edit them. It
adds the **superseding bindings** review-6 asked for: the executed evidence, the exact paths, the
source cuts, the tool/input/template/output hashes, the commands, the outcomes and the residuals.*

**Governing rule (unchanged): the value is recorded for a file the run actually consumed, and a
hash copied from a document is not evidence.** Every hash below was computed from the file on disk
at step 9 (`python3 -I` + `hashlib`), not transcribed. Re-measure before quoting.

Path prefixes: `$EV` = `/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline`,
`$IR` = `/home/s00653124/workspace/AscendNPU-IR-1`, `$TL` = `/home/s00653124/workspace/tilelang`,
`$CL` = `/home/s00653124/p2-closeout`.

### 0bis.1 The executed chain, end to end

| role | identity (path = sha256 (size)) |
|---|---|
| compiler (staged, the run actually executed) | `$EV/impl/stage-p2c-r1/bin/bishengir-compile` = `812c5cb1d29cb3b8879593ea77d08ebdae274cc943d41523c7c4bab739cbf455` (205,469,200 B) |
| template actually resolved + consumed (O2) | `$EV/impl/stage-p2c-r1/lib/meta_op.aiv.c310.O2.bc` = `3807eb3d907236b17855db57a475ea120c37a74905c92bb4b971b415da7d6f79` (31,206,992 B) |
| template sibling (present, NOT selected) O0 | `$EV/impl/stage-p2c-r1/lib/meta_op.aiv.c310.O0.bc` = `b8c68dcb1ef506464c2a27d83178772386e9cf65a834b9a8c9a71c8a41f27078` (27,695,840 B) |
| link-edge output (linked template) | `$EV/impl/stage-p2c-r1/lib/meta_op.aiv.c310.O2.bc.linked.bc` = `cbd56bd926f847f485f958d7358579f476bb0f41f345a6074fdcf5f2c0ae84c4` (31,180,672 B) |
| bundle manifest (sealed) | `$EV/impl/stage-p2c-r1/SHA256SUMS.txt` = `6aa3c1ef0b818418e641f7713a0e0a3be991b609ad787c968279abff81819cf4` (377 B) |
| bundle manifest (pre-rebuild; 1 FAILED by design) | `$EV/impl/stage-p2c-r1/SHA256SUMS.pre-rebuild.txt` = `53d3d8f1800e0b8eda5743d7012116fd686395ccbfc7011e335830ce4a2cca0c` (377 B) |
| bundle manifest README | `$EV/impl/stage-p2c-r1/SHA256SUMS.README.txt` = `9c2e111f9a9d4b8a0a28748252549582346fc575484efb6100abd477c6c4f27f` (6,407 B) |
| bundle template-identity note | `$EV/impl/stage-p2c-r1/TEMPLATE-IDENTITY.txt` = `83241567a025c85406f01871615b43acac3e48a06ba5fbb70a9fd6eb0dc8c3c6` (6,785 B) |
| external anchor: B7 regeneration | `$CL-SHA256SUMS.regen.final.txt` = `6aa3c1ef0b818418e641f7713a0e0a3be991b609ad787c968279abff81819cf4` (377 B) |
| external anchor: staged-bin identity | `$EV/impl/scratch/p2c-build/s5-bundle-bin-identity.txt` = `74f1215dc083958a091efd1e0ec7c6439f41efd79e2d6587063760746fa19ac3` (4,376 B) |
| INPUT fixture (Late HIVM/AVE MLIR) | `$EV/verify/tests/negatives/vadd_late_p1v2.mlir` = `d1023ac1fdfd22c8d7f2aed5c71241fdd83039744b383e43b19255d10a31f36b` (18,659 B) |
| OUTPUT binary (the produced artifact) | `$EV/verify/work/run/A1/kernel.o` = `2c4247c3b2ce11f79ad15415eb06d4ce1d5523a0c19c91ad3743bc08e7b8c4c9` (4,448 B) |
| OUTPUT identity sidecar | `$EV/verify/work/run/A1/kernel.o.identity.json` = `8df4dcd688e6006c6c120fabd6e294446e2eec4edd8c3811974a80361737fc62` (880 B) |
| compile stderr (nine [P2-C-GATE] lines) | `$EV/verify/logs/suite-v3/20261014T172238Z-190190/A1.stderr` = `7f43c384b2f82d5f3c434c71399ee20da40d3c185b2ac6e5c6c51c561eb4cd8c` (1,816 B) |
| compile argv (verbatim) | `$EV/verify/logs/suite-v3/20261014T172238Z-190190/A1.cmd.txt` = `043a87d66f922cea0109bdcce5f59b1ffab9c5e498641cb80ea52b951d291f39` (449 B) |
| C2 compile stderr (311,326 B / 2,970 lines) | `$EV/verify/logs/suite-v3/20261014T172238Z-190190/C2.stderr` = `fac68713b4cc0898fdfae2367bf4113afe478dcc66fc468777fcdf18dde0de63` (311,326 B) |
| run-of-record summary | `$EV/verify/logs/suite-v3/20261014T172238Z-190190/suite-summary.json` = `243973ffd53116d0de086506a8f1c9648d7323c04bcedac89701d9069e1d67f5` (20,030 B) |
| Golden codegen input (immutable fixture) | `$TL/tests/golden/hivm/vadd_f32_single_tile/golden_codegen_input.tir` = `4194733b8412484c9bb8caf74ba3285348345f92ad61684784fe8c3a79e12a18` (3,338 B) |
| runtime: ascend_module.cc | `$TL/src/ascend/runtime/ascend_module.cc` = `9e89adb0853ab8985b944484feef81cc039d4741d61b339e8b5412d772413e77` (17,632 B) |
| runtime: ascend_device_api.cc | `$TL/src/ascend/runtime/ascend_device_api.cc` = `9196ae245ccb2091b3d8e9b5d1adfe722ea31c5b8c8d3dc85f18c071132ae654` (2,995 B) |
| runtime: rt_mod_ascend.cc (producer) | `$TL/src/ascend/codegen/rt_mod_ascend.cc` = `08bf2b8a47675dfd60d909898adc0229f3e24cc4a77b1165cae0d03ea953bf22` (5,127 B) |
| runtime: metadata.h (tag constants) | `$TL/3rdparty/tvm/src/runtime/metadata.h` = `5fb7b9b5ee2ef6ea545936593dd773f5d7add2ab3402cb48716aac06b5cb1e71` (6,475 B) |
| runtime: thread_storage_scope.h (grid path) | `$TL/3rdparty/tvm/src/runtime/thread_storage_scope.h` = `a82e291a1cb30155b5322a6e3554cd13009be116c8c0ac1c193f0d55751d93b6` (12,083 B) |
| HACCToLLVM.cpp (hacc.block_dim reader) | `$IR/bishengir/lib/Conversion/HACCToLLVM/HACCToLLVM.cpp` = `7c320ef54cd832e106dce394143d938c1e551fc3cfd18700b045ebbb7cbaf6ee` (28,316 B) |
| A5 PassPipelineA5.cpp (the route's pass list) | `$IR/bishengir/lib/Tools/hivmc/A5/PassPipelineA5.cpp` = `7bd25d32bb72d6822c4701fceabfc5d314c4c3f7893abe77527cf42e778637bd` (19,793 B) |
| review-6 verdict | `$CL/review-6-verdict.txt` = `0248aab4abb2e4d9a036ede611a02722aa88d05e5198e5d530d6fa65e10533e6` (9,406 B) |
| review-5 verdict | `$CL/review-5-verdict.txt` = `444a3cf0cd352a9191787e511e7c7424b705a5f9ace0bd78d060b086bd06a186` (5,168 B) |
| FunctionInfo consistency report | `$EV/lead/functioninfo-consistency-check/FUNCTIONINFO-CONSISTENCY-CHECK.txt` = `9d7603a304e4f6d217ec536aaa0e7594f7d83276caf44605ba81dba047a8d2dd` (51,699 B) |
| FunctionInfo field pin | `$EV/lead/functioninfo-field-pin/FUNCTIONINFO-FIELD-PIN.txt` = `da05a4716e0871b4af08a0397699892b4e4ff2809e9ce760b0132e74624c5f25` (10,834 B) |
| R1 linked-consumption investigation (FINDING 4) | `$EV/lead/r9-review/R1-LINKED-CONSUMPTION-INVESTIGATION.txt` = `374544aa41b6fad0461c31aa52a8c3b8edbbf087886e8ddc80b234191ef0b58b` (19,149 B) |
| R5 closure note (carries the :99 mislabel) | `$EV/verify/logs/step8-r2r6-20261014T2000Z/R5-CLOSURE.txt` = `0d4bcb3ce36ff7bae5268e04e9ab12cdd6fa8b0a81e56386294b2cbe94cc8a84` (12,765 B) |
| frozen test suite of record | `$EV/verify/p2c-freeze/P2C-TEST-FREEZE.r9.txt` = `1e3afd3d9eb44746fcac34996d2b88658753d27bd42556716a667a35537c433b` (213,553 B) |

### 0bis.2 What the two manifests and the four anchors bind

- **`sha256sum -c SHA256SUMS.txt` run from `$EV/impl/stage-p2c-r1/` as its own base → 4/4 OK, rc=0**
  (re-measured at step 9). Every bundle entry has link count 1 and a distinct inode
  (161332341–161332351), so no staged artifact is reachable through a second name.
  *A check run from any other directory reports FAILED on a correct bundle — the standing rule.*
- **`SHA256SUMS.pre-rebuild.txt` → 1 FAILED + 3 OK, rc=1 by design.** The FAILED is
  `bin/bishengir-compile`, i.e. the manifest correctly records the pre-rebuild binary that the
  P2-C build replaced. This is a *recorded* pre-state, not a corruption.
- **The B7 regeneration is byte-identical to the sealed manifest**: `$CL-SHA256SUMS.regen.final.txt`
  and `$EV/impl/stage-p2c-r1/SHA256SUMS.txt` both hash to `6aa3c1ef0b818418e641f7713a0e0a3be991b609ad787c968279abff81819cf4` (377 B), `cmp` clean.
  Regenerating last changed no row — which is what B7 requires and all it can show.
- **The four external anchors**, re-measured at step 9 and unchanged:
  `TEMPLATE-IDENTITY.txt`, `SHA256SUMS.README.txt`, `SHA256SUMS.txt` (the three in the bundle) and
  `$EV/impl/scratch/p2c-build/s5-bundle-bin-identity.txt` (outside it, so the bundle cannot
  self-certify). Their values are the four `anchor` rows above.

### 0bis.3 The staged compiler is NOT HEAD — stated because review-6 required it

| | identity |
|---|---|
| **staged compiler actually executed** (the run of record) | `812c5cb1d29cb3b8879593ea77d08ebdae274cc943d41523c7c4bab739cbf455` (205,469,200 B), staged at `$EV/impl/stage-p2c-r1/bin/`, built from commit **`4495ab29b`** on `feat/tilelang-hivm-late-pipeline` (the five-file cut) |
| **HEAD of the production tree at step 9** | `82f3ad088ab3cea0e81fa6627e0487a93e5317ef`, `git status --porcelain` **empty** |
| **`4495ab29b..HEAD` delta** | **2 files, 295 insertions, 0 deletions — both are test files**: `bishengir/test/Conversion/HIVMToStandard/RegBase/store-l2-cache-mode-lowering.mlir` (121 lines) and `…/store-l2-cache-mode.mlir` (174 lines). **Production logic: zero change.** |

So the binary that produced `kernel.o` predates HEAD by two **test-only** commits. That is why the
executed identity is the staged one and why the two are recorded as separate rows rather than
collapsed. A reader must not infer that HEAD's tree, or a later opt build, is what ran.

### 0bis.4 The eleven §6.3 checks — executed status, and the one that was NOT_RUN

All eleven are now carried by executed evidence. Source for items 1/3/4/5/7/8/9/10 is the **final
region** of `C2.stderr` (`fac68713`, 311,326 B) — the span from the line-2869 header
`IR Dump After AppendUsePrintDebugData` to EOF. **It must not be called "After ConvertHIVMToLLVM":**
that header is at line **1259** and heads an intermediate region still holding **18**
`unrealized_conversion_cast`; the final region holds **0**. Item 2 (FunctionInfo consistency) left
`NOT_RUN` via the Verifier's independent execution — `$EV/lead/functioninfo-consistency-check/`,
report `9d7603a3` (51,699 B, 862 lines):
`name` **CONSISTENT**, `arg_types` **CONSISTENT** (delimited to count 3 == 3 and packed size
24 == 24), `launch_param_tags` **NOT_OBSERVABLE**, `arg_extra_tags` **NOT_OBSERVABLE**. The two
`NOT_OBSERVABLE` marks are deliberate: **no FunctionInfo object exists in the artifact at all**
(generation is P4), so "expected list" against "field absent" is an equality between two absent
operands and may be read as neither MATCH nor MISMATCH.

### 0bis.5 Residuals carried forward (not smoothed over)

- **§6.2 item 5 is measured on all three sides as an ABI/encoding observation only.** The compiler
  emits the constant `4` in the store libcall's 4th i32 operand; the template's own IR consumes that
  parameter as a cache mode (455 occurrences in each template). The **stronger** observation — that
  the linked kernel's machine-code descriptor word carries `4` in bits 60–63 — is **NOT
  established**: this environment has no AIV disassembler that decodes the linked body. This is an
  ABI fact, **never a numerical-correctness claim**.
- **The grid-carrier divergence is load-bearing.** The artifact carries grid as `hacc.block_dim = 64`
  (a function attribute); the runtime reads grid from `launch_param_tags` + packed positional args.
  Over the named 1,179-line runtime universe, `hacc.` has **0** hits; in the compile output,
  `tirx.kernel_launch_params` has **0**. Neither side spells the other's carrier. The only reader of
  `hacc.block_dim` is `HACCToLLVM.cpp:609`, a **host-stub** mechanism whose pass does not appear
  among this run's 35 dump headers. Correct statement: **no consumer on the Late/A5 route.**
- **`hacc.block_dim` is not verifier-checked** — P3's grid input (64) rests on the Golden and the
  attribute, not on an executed runtime read (see the P3 handoff note's N3 caveat).
- **F.3 is an executed failure, not a NOT_RUN.** The ordinary RegBase SIMD compile fails to link on
  the same undefined symbol in both arms and produces identical prelink objects. **No passing
  ordinary-SIMD ELF regression may be reported.**
- **Store multiplicity is a nonblocking coverage limitation.** The positive checks do not prove the
  absence of additional stores; no whole-function side-effect-completeness claim is approved.
- **No complete r4 code-object equality is cited** (a standing fact-limit, obligation B9).
- **C8 (the second positive evidence compile) remains UNSPENT.** Its sole candidate — the D087
  template-swap differential — is non-discriminating by construction: the instruction-stream diff
  between the two templates is **0 bytes**, the store entry sits at the same line with a
  byte-identical signature already carrying `l2_cache_mode`, and swapping changes zero instructions.

### 0bis.6 The suite's own row verdicts, and which runs supersede them

Run of record: `$EV/verify/logs/suite-v3/20261014T172238Z-190190/`, summary `243973ff`
(20,030 B), **PASS 37 / FAIL 6 / BLOCKED 0 / NOT_RUN 3** over 46 rows. FAIL = **A11, A12, A13,
A15, C1, R1**; NOT_RUN = **B12a, B12b, C3**. **Later reruns supersede row verdicts by design:**
v4 (`20261014T203225Z-1411318`) → 7 PASS / 1 FAIL (A13) / 38 NOT_RUN; v5
(`20261014T224213Z-2013872`) → 3 PASS (A13, A15, R1) / 43 NOT_RUN; v6
(`20261014T234654Z-2356091`) → 46 NOT_RUN (VOID per D123). So A11/A12/C1/R1's run-of-record FAIL
is **superseded** by a later rerun that selected them — not a contradiction to be averaged.
B12a/B12b read `NOT_RUN` only because the suite's own stale `stub_terminal` canary matched the
P2-C route's terminal line; **both are measured rejections** (`exists=no` / wrong-sha → rc=1, no
artifact), and that canary can only under-claim.

### 0bis.7 Frozen fixture and suite identities

- Input fixture `$EV/verify/tests/negatives/vadd_late_p1v2.mlir` = `d1023ac1fdfd22c8d7f2aed5c71241fdd83039744b383e43b19255d10a31f36b` (18,659 B).
- Frozen suite `$EV/verify/p2c-freeze/P2C-TEST-FREEZE.r9.txt` = `1e3afd3d9eb44746fcac34996d2b88658753d27bd42556716a667a35537c433b` (213,553 B).
- Golden `$TL/tests/golden/hivm/vadd_f32_single_tile/golden_codegen_input.tir` = `4194733b8412484c9bb8caf74ba3285348345f92ad61684784fe8c3a79e12a18` (3,338 B) — **immutable accepted
  input; never regenerated to match output.**
- Challenger verdicts: review-6 `0248aab4` (9,406 B) and review-5 `444a3cf0` (5,168 B).

### 0bis.5-CORRECTION — the §6.2 item-5 linked half is **CLOSED BY OBSERVATION**, not "NOT established"

**This supersedes the first bullet of §0bis.5 above, which is WRONG.** That bullet says the stronger
observation — *the linked kernel's machine-code descriptor word carries `4` in bits 60–63* — is
"**NOT established: this environment has no AIV disassembler that decodes the linked body**". That
environment negative is the **FINDING 2** claim of `lead/r9-review/R1-LINKED-CONSUMPTION-INVESTIGATION.txt`
(`374544aa41b6fad0461c31aa52a8c3b8edbbf087886e8ddc80b234191ef0b58b` / 19,149 B), and **that file's own
CORRECTION 3** (`:151-272`, written 2026-10-14T20:07Z — i.e. **before** this document's §0bis append)
**falsifies it**: *"FINDING 2 IS WRONG. An AIV disassembler DOES exist in this environment"*. The
rulings are `docs/decisions.md` **D112** (`:167`; re-landed complete as D113 `:168`) and **D120**
(`:178`, *"R1's residue … is CLOSED"*), which additionally names the still-awaiting-authorization item
as **VOID** — *"a forward propagation of the PRE-231"* state. §0bis.5's bullet, D126 and 追加 255 §2
item 5 are that forward propagation.

**Correct state — §6.2 item 5 measured on three sides, all ABI/encoding observations only, never a
numerical-correctness claim:**

| side | carrier | status |
| --- | --- | --- |
| emitter | `C2.stderr` final region: store libcall's 4th i32 operand = constant `4` | MEASURED |
| template | `dis15/p2.o2.ll`: the ciface shifts that parameter `shl …, 60` into bits 60–63 | MEASURED |
| **linked** | the run-of-record ELF `verify/work/run/A1/kernel.o` | **CLOSED BY OBSERVATION** |

**The linked-side observation, independently reproduced by the Lead this window (no compile, no NPU):**
- **Instrument**: `/home/s00653124/workspace/ccec-toolkits/20260908/ccec_compiler/bin/llvm-objdump`, sha256 `079bf9bbe6bd20ec154beab7fb28d8d71a9a1774e8fa1c7dc5924377f4dc8699` / 20,085,768 B. Its *version banner is not an identity*: five
  `llvm-objdump` candidates on this host all print `LLVM version 15.0.5` and register target
  `hiipu64`, yet **only this one decodes the AIV ISA** (`<not available>` count over the same subject
  = **0**; the control instrument `/data/pri/cann-9.1.0/bin/llvm-objdump` on the *same command and
  subject* = **26**).
- **Subject**: `verify/work/run/A1/kernel.o`, sha256 `2c4247c3b2ce11f79ad15415eb06d4ce1d5523a0c19c91ad3743bc08e7b8c4c9` / 4,448 B.
- **Store body** `store_ubuf_to_gm_1d_float` (vaddr `0x198`, 52 B), terminal DMA instruction:
  `MOV_UB_TO_OUT_ALIGN_V2  [X0], [X3], X2, X1` — **3rd operand `X2 = 0x4000010000000010`**
  (assembled from `MOVI X2,#0` / `MOVI X2,#16` / `MOVK X2,#256,#2` / `MOVK X2,#16384,#3`) →
  **bits 60–63 = 4**.
- **Discriminating control** (same instrument, same object, the sibling function whose ABI has *no*
  cache mode): the load body `load_gm_to_ubuf_1d_float`'s DMA
  `MOV_OUT_TO_UB_ALIGN_V2.b32  [X2], [X0], X3, X1` — **`X3 = 0x0400010000000010`** → **same field
  bits 60–63 = 0**. The field is not a constant the template bakes in: it tracks the parameter, and
  the two functions differ exactly as their ABIs do.

**C8 remains UNSPENT — but the reason is corrected.** Not "the residue is unclosable here" (withdrawn),
but **FINDING 4**: C8's sole candidate (the D087 template-swap differential) is non-discriminating by
construction (`instr.diff` = 0 bytes; the store entry sits at the same line 743742 in both files with a
byte-identical signature already carrying `l2_cache_mode`; 455 occurrences in each). The observation
above used **existing artifacts and an existing instrument**, so R1 never needed C8. review-6: *"I do
not require spending it on the disproved template-swap experiment."*

**record-not-repair**: the FINDING 2 text (a Verifier-frozen artifact) is **not edited** — it is
superseded in place by its own CORRECTION 3; the OPEN QUESTION section of
`lead/r9-review/STEP9-EVIDENCE-MAP.txt` (the Lead's own working note, **not evidence**) is annotated
superseded. Corrected here by appending; the §0bis.5 bullet above keeps its bytes.
