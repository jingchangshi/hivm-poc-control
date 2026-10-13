# P2 independent verification record

**Status: test freeze IN PROGRESS; all acceptance rows NOT_RUN.** No P2 production code exists yet.
This file records the **independence** of the verification, the frozen test inventory with its expected
outcomes, and the acceptance matrix. It is written by the **Lead**; the tests themselves are owned and
executed by the **Verifier**, whose expectations must be defined independently of the implementation and
**before** it exists (CLAUDE.md, and goal §7).

Two rules decide whether a row here may ever read PASS:

1. **A test must fail for its intended semantic assertion** — not because an input SHA mismatched, not
   because MLIR failed to parse, and not because a command exited non-zero for an unrelated reason.
   Every row names the assertion it is testing for.
2. **Every non-executed test is NOT_RUN, never PASS.** No assertion is weakened after a failure, and no
   oracle is edited to match output.

## 0. Independence

| # | requirement | status |
|---|---|---|
| 0.1 | the Verifier's suite is authored and sealed **before** implementation begins | **FROZEN v1 (Verifier, 2026-10-13 13:28:57Z)** — 35 tests (A×13, B×12, C×6, D×4) under `p2-20261013-late-pipeline/verify/`; `P2-TEST-MANIFEST.json` pins SHA-256 of 29 artifacts with a sidecar over the manifest; `tests/config/p2-bindings.json` is a byte-copy of the template. **P2 production code was 0 lines at freeze.** C6 is the one test executable without the P2 compiler: **PASS** at freeze, all 20 fixtures matching the plan's calibration table (19 parse rc=0; `b03_cache_bad7.mlir` rc=1 with the D012 value gate). The other 34 are **NOT_RUN**. Three measured freeze-time baselines: bare parse rc=0; ordinary path on the Late input at the default target rc=1 (the D012 target gate); at the 950 target rc=134 SIGABRT in PlanMemory. Amended after Lead review — see 0.1a |
| 0.1a | **Lead review of the frozen suite**: three coverage gaps and one binding-shape issue, each derived from the round-4 requirements | **raised with the Verifier, open** — (i) A10/A11/A12 use `--print-pass-id --mlir-print-ir-after-all` with no assertion that the four `--mlir-print-ir-*` filters are off and no `before-all`; a positive control (a header printed over an interval where the IR cannot have changed) is the discriminating proof. (ii) B12 is **one** negative where round-4 requires **four** — absent, wrong-identity (the in-tree pre-D012 `44e4387b…`), forbidden extra bitcode, and combined. (iii) no test names `--save-linked-ir`, `--enable-static-bare-ptr=false`, `--enable-optimized-metaop`, `--enable-hivm-auto-storage-align`, `--enable-lib-call-no-inline`, `--enable-hivm-unit-flag-sync` or `--enable-preload`. (iv) `bitcode.aiv_c310_o2_path`/`_sha256` are frozen non-null to the **build-tree** `5e7ccae2…`, while `p2_compiler` and `late_stage_flag` use the null discipline — but at acceptance the compiler is the **staged** `<stage>/bin/…`, whose template resolves to `<stage>/lib/…`, so the pinned path is not the file the run consumes; and `5e7ccae2…`'s relation to the committed D012 template sources is undetermined (§3.3 of `p2-architecture.md`). Suggested: make it a null acceptance-time binding and keep `5e7ccae2…` as a named P1-historical field. Any amendment must be a **named, recorded v2 freeze** with its own manifest and timestamp, leaving the v1 manifest and sidecar byte-unchanged — never a silent edit, and still before implementation |
| 0.2 | frozen inputs are byte-identical to the accepted P1 fixtures (no silent regeneration) | **VERIFIED (Lead, 2026-10-13)** — `verify/tests/negatives/vadd_late_p1v2.mlir` SHA-256 = `d1023ac1fdfd22c8d7f2aed5c71241fdd83039744b383e43b19255d10a31f36b`, identical to `p1v2-20261013-d025/snapshot/impl/vadd_late.mlir`; the 22 negative inputs are 22 distinct SHAs |
| 0.3 | the Verifier's expectations are not derived from the Implementor's code | NOT_RUN |
| 0.4 | tests run against the **newly built, staged** `bishengir-opt` (`p2-architecture.md` §3.3 G1/G3), not the pinned P1 binary | NOT_RUN |
| 0.5 | the Challenger independently reviews the final Git diff, the real execution logs, and the key negatives | NOT_RUN |

## A. Entry and pipeline (goal §7 A)

| # | test | expected | status |
|---|---|---|---|
| A1 | correct Late routing | Late routing line present; the ordinary-`runRegBasePipeline` boundary line absent (logged at the function entry, `BiShengIRCompileMain.cpp:345`, so both callers are covered) | NOT_RUN |
| A2 | ordinary RegBase path unchanged | same pass sequence as before the change | NOT_RUN |
| A2b | zero calls to both inference functions | **absence** of the boundary log at `inferLayoutOptimization` / `inferMixedCV` function entry (`Utility.cpp:100`, `:61`) — asserted as zero *calls*, the pass trace being blind to both by construction | NOT_RUN |
| A3 | wrong target rejected | explicit diagnostic; no artifact | NOT_RUN |
| A4 | wrong stage rejected | explicit diagnostic | NOT_RUN |
| A5 | wrong core rejected | explicit diagnostic | NOT_RUN |
| A6 | wrong device spec rejected | diagnostic **naming the differing field** | NOT_RUN |
| A7 | forbidden passes did not execute | zero scheduled invocations, coverage demonstrated | NOT_RUN |
| A8 | required pass order correct | the §3.1 order, with per-pass IR effects | NOT_RUN |
| A9 | LLVM lowering exactly once | `--convert-hivm-to-llvm` count = **1 in total**, aggregated over every entered manager; measured per operation scope at the pass boundary (construction entries are not executions). One execution per scope is **not** a PASS | NOT_RUN |
| A10 | Late without a target | Late-specific diagnostic, **no** artifact; must never reach the ordinary pipeline | NOT_RUN |
| A11 | Late + legacy delegation env (`BISHENGIR_LEGACY_A5_REGBASE`) | explicit diagnostic, **no** delegation | NOT_RUN |
| A12 | the §2.3.1 option policy, row by row | each **rejected** row produces a diagnostic naming *that* option; each **pinned** row is pinned by an explicit argument (so `hasExplicitCLOption` is true and `applyArchDependentCompileDefaults` cannot move it), including the three instruction-selection rows frozen to `false` (`--disable-fma`, `--disable-decompose-reduction`, `--disable-reorder-instruction`), each of whose `true` requests must be rejected with *its own* named diagnostic. The pinned arguments must be present in `argv` **before `:121`** — earlier than the parse (`:144`, calling `cl::ParseCommandLineOptions` at `:102`) because `:121-124` captures `originalCLArgs` from `argv` and `:186` forwards *that vector* downstream — and the entry must read back the post-parse `getNumOccurrences()`. A rejection test that passes because the driver reported an unknown option is a **wrong-reason pass**, not acceptance (`--compile-host` is the measured example) | NOT_RUN |
| A13 | `--save-linked-ir` rejected in Late mode | named diagnostic; **no** binary — it switches the artifact to `.ll` and appends `-Xclang -disable-llvm-optzns -S -emit-llvm`, so it cannot appear in an invocation meant to produce the accepted binary. A diagnostic-only invocation is permitted but is **separately labelled and produces no artifact** | NOT_RUN |

## B. Semantic counterexamples (goal §7 B) — the heart of P2

Each row names the **semantic** assertion it must fail on. The frozen inputs are the Verifier's
`verify/tests/negatives/*.mlir`, all 22 distinct SHAs derived from `d1023ac1…`.

| # | counterexample | must be caught by | status |
|---|---|---|---|
| B1 | cache4 → cache0 | a **value-level** assertion, not an IR-text match | NOT_RUN |
| B2 | cache carrier deleted | value-level assertion | NOT_RUN |
| B3 | illegal store cache / D012-unsupported layout | explicit rejection **before any mutation** (preflight V7, not the conversion mid-tail) | NOT_RUN |
| B4 | pb32 deleted | value-level assertion | NOT_RUN |
| B5 | entry-terminal `PIPE_ALL` deleted | entry-scoped terminal-barrier assertion | NOT_RUN |
| B6 | `PIPE_ALL` moved **into the VF helper** or to a wrong control-flow position | must **not** obtain a false PASS | NOT_RUN |
| B7 | UB byte offsets changed | no false PASS | NOT_RUN |
| B8 | DMA payload changed | no false PASS | NOT_RUN |
| B9 | GM block offset changed | no false PASS | NOT_RUN |
| B10 | missing or wrong C310 bitcode | explicit failure, no binary — split into **N1** (required template absent), **N2** (template present but not the recorded identity — the pre-D012 `44e4387b…` case), **N3** (`--link-aicore-bitcode` supplied), **N4** (absent *and* an extra supplied, rejected without prescribing diagnostic precedence). "Today produces a warning and a binary" is a source-predicted hypothesis and is **NOT_RUN** until executed | NOT_RUN |
| B11 | (negative) mutated store reaching the **scalar fallback** while keeping cache mode 4 | preflight rejection naming the unsupported DMA shape — textual strides alone are not enough | NOT_RUN |

## C. Compiler and old-path regression (goal §7 C)

| # | item | status |
|---|---|---|
| C1 | the D012 verified positive/negative cases become **in-repo** maintainable MLIR regression tests | NOT_RUN — landing points identified (Lead, 2026-10-13): validator diagnostics via `bishengir-opt … -verify-diagnostics` under `bishengir/test/regbase/`; driver-level option/routing diagnostics in `bishengir/test/bishengir-compile/commandline.mlir`, whose existing pattern is exactly `not bishengir-compile %s --opt=… 2>&1 \| FileCheck %s --check-prefix=NAME`. Measured: **no** in-repo test currently uses `l2_cache_mode` (0 hits across all 1119 `.mlir`), so this is real work, not a re-run |
| C2 | default cache0 checked | NOT_RUN |
| C3 | the fourth C310 parameter checked | NOT_RUN |
| C4 | the non-C310 path retains the old ABI | NOT_RUN |
| C5 | old RegBase SIMD path: at least one representative compile regression affected by the driver change | NOT_RUN |
| C6 | any acceptance affected by the current driver/LLVM/runtime change is re-verified specifically | NOT_RUN |

## D. Binary authenticity (goal §7 D)

| # | item | status |
|---|---|---|
| D1 | the new binary came from **this** P2 compile | NOT_RUN |
| D2 | the binary's identity relationship to input, compiler and bitcode is traceable | NOT_RUN |
| D3 | deleting or replacing the output and recompiling yields a **real new** artifact | NOT_RUN |

Cross-reference: the measurement detail for D1–D3 lives in `docs/p2-binary-abi.md` D1–D7; the ABI rows
are not duplicated here.

## E. Challenger review (goal §7, §8 step 8)

| # | item | status |
|---|---|---|
| E0 | **gate 3, entry design** (goal §8 step 3) | **REVISE (round 4)** — the Challenger accepted the architectural cut and the corrected selector, validator registration and preflight plan, matching-spec reuse, filtering invariant, copied `bin/lib` staging and guarded restore, and stated a scoped **entry-design APPROVE** can follow the four listed corrections without waiting for the test freeze. All four were re-verified against local source and applied 2026-10-13 at `d2e6fae` — after which the Challenger confirmed the four groups satisfied and independently reproduced the 21-op census. **REVISE (round 5, scoped to two specification gaps)**: (i) the three instruction-selection rows still said "pin to measured values" without values — now frozen to `false` and shown to have no defaults override; (ii) the count rule placed every count inside a function scope, which would have let a module pass that ran in two managers read as "once" — now split into per-scope *measurement* with compile-wide *assertion* (`ConvertHIVMToLLVM` = 1 total, forbidden = 0 total, A5 = 1 total), propagated to `p2-pass-trace.md` and A9. Three nonblocking cleanups applied too (helper reuse does not prove ABI preservation; the host guard is a config-state assertion, not a CLI flag; explicit arguments must be injected before the parse). Three pre-existing `Options.td` citations were also corrected. Verdicts persisted verbatim at `review/challenger-entry-design-verdict-round4.md` and `…-round5.md` | recorded |
| E0b | **gate 3 closing** — scoped entry-design APPROVE on the corrected design | NOT_RUN — re-review requested immediately after the round-5 edits; the Challenger stated APPROVE "can follow" them | NOT_RUN |
| E1 | the Challenger independently reviews the **final Git diff** | NOT_RUN |
| E2 | … the **real execution logs** | NOT_RUN |
| E3 | … the **key negatives** | NOT_RUN |
| E4 | verdict stated as **APPROVE / REVISE / BLOCK** | NOT_RUN |
| E5 | items verified only from source are **not** marked executed PASS | NOT_RUN |

## F. Model identity (goal §3) — configured vs **served**

Goal §3 requires the **actually served** model to be read, not inferred from the agent-type label. Two
facts, kept apart. (An earlier draft of this section asserted served models without citing a source; it
was then over-corrected to "NOT_CONFIRMED" before the per-agent records below were located. Both drafts
were wrong; this one cites where each value comes from.)

### F.1 Configured slot → upstream model (evidenced)

Read from `/home/s00653124/SoftwareProfiles/claude/run_claude_using_magpie.sh` and confirmed against
this shell's live environment:

| slot | configured upstream model | agent type using it |
|---|---|---|
| `ANTHROPIC_DEFAULT_SONNET_MODEL` | `group/auto-deepseek-v4-1-flash` | `implementor-sonnet` (and the Lead) |
| `ANTHROPIC_DEFAULT_HAIKU_MODEL` | `opencode-go/mimo-v2.6-flash` | `verifier-haiku` |
| `ANTHROPIC_DEFAULT_OPUS_MODEL` | `codex/gpt-6.1-sol` | `challenger-opus` |

The launcher's **intended** mappings (`codex/gpt-6.1-sol` for Sonnet, `opencode-go/claude-haiku-5-5`
for Haiku, `claude/claude-opus-5-5` for Opus) are present but **commented out**.

### F.2 Served models for **this** session (evidenced, per agent)

Source: the gateway-echoed `model` field in each agent's own transcript record under
`~/.claude/projects/-home-s00653124-workspace-hivm-poc-control/ea69c136-c295-41a6-96a9-140265430a03/`
(the main `*.jsonl` for the Lead, `subagents/agent-*.jsonl` per teammate).

| role | agent type (label) | **served model** | evidence |
|---|---|---|---|
| Lead | team-lead | **`deepseek-v4.1-flash`** | main transcript, 4173 records |
| implementor | `implementor-sonnet` | **`deepseek-v4.1-flash`** | `agent-aimplementor-619b17ef9c142eac.jsonl` 381×, `agent-aimplementor-2-433e3f35596bf587.jsonl` 155× |
| verifier | `verifier-haiku` | **`mimo-v2.6-flash`** | `agent-averify3-c52add0e1050031b.jsonl` 800×, `agent-averifier-28ea1ab050148399.jsonl` 208×, `agent-averify-fdc56caba59a491c.jsonl` 237×, **current** `agent-averify-fd1c402be6c7e059.jsonl` 169× |
| challenger | `challenger-opus` | **`gpt-6.1-sol`** | `agent-achallenger-00ae991f5d9687bd.jsonl` 266×, `agent-areview-175c0b7e40a24cfe.jsonl` 144×, `agent-areview-44c1079fda146d71.jsonl` 72×, **current** `agent-areview-2-744b9123f07b8871.jsonl` 224× |

Two agents (`agent-averify2-fbb55375e6152a82`, `agent-averify-b230d5081b517ca5`) contain only
`<synthetic>` model records — they made no recorded model call, so they are **not** evidence either way.

**No agent in this session is served by a Claude model.**

### F.3 Does this match the intended division? No.

Goal §3's intended division is **GPT-6.1 Sol / Claude Haiku 5.5 / Claude Opus 5.5**. Measured, the
routing differs on **all three** slots: the Lead — intended to be GPT-6.1 Sol — is served by
`deepseek-v4.1-flash`, while `gpt-6.1-sol` is the model behind the **challenger**; the verifier is
`mimo-v2.6-flash`, not Claude Haiku 5.5; the challenger is `gpt-6.1-sol`, not Claude Opus 5.5. This is
reported here as required, before the P2 architecture review is concluded.

### F.4 Consequences in force for P2

- **No verdict in this project may be recorded as an "Opus 5.5 review."** The `challenger-opus` label is
  an agent-type routing label; the model behind it is `gpt-6.1-sol`.
- Conversely, no verdict may be recorded as *being* the intended Claude model. Attributions in this
  project name the **served** model from F.2, with this file as the source.
- Re-confirm at each gate: a slot's served model can change between sessions, and the only reliable
  source is the per-agent transcript / Magpie request log for the session in question.
- **The P0 session's gateway-wide log is not a substitute.** `/home/s00653124/refusal-capture/
  hivm-p0-20261012T124105Z.E3UJQ9/magpie-requests-at-exit.json` is (a) a different session and (b) a
  mixed set (`gpt-6.1-sol` 266, `deepseek-v4.1-flash` 125, `step-5-preview-free` 87, `claude-haiku-5-5`
  15, `claude-opus-5-5` 5, `mimo-v2.6-flash-free` 2) that does not match that session's own
  configuration, so it cannot be used to attribute a model to a specific agent. Its own team record
  states `"served_models": "NOT_VERIFIED"`.
