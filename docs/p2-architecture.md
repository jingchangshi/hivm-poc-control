# P2 architecture — an explicit, mutually exclusive Late Pipeline entry

Date: 2026-10-13. **Status: PROPOSED — awaiting Challenger review (execution order step 3). No
production code has been written.** Every source claim below was read live from
`/home/s00653124/workspace/AscendNPU-IR-1` at HEAD `80e29e6f27192833d2d6cd3fd846e0b78ee02d1c`, branch
`feat/tilelang-hivm-late-pipeline`, worktree clean. HEAD sits exactly **one commit above** `e878f3492`,
the HEAD the frozen P1 artifact was authored at, and that one commit **is** the D012 change set —
`[HIVM] fix: carry the store L2 cache policy through to the DMA descriptor`, authored 2026-10-13
19:47 +0800, 11 files: the store's `OptionalAttr<I32Attr>:$l2_cache_mode` (`HIVMDMAOps.td:181,198`),
`verifyStoreCacheMode` (`Conversion/HIVMToStandard/regbase/HIVMToStandard.cpp`, +155),
`HIVMDMAOps.cpp` `StoreOp::verify`, the C310 template sources
`lib/Template/lib/RegBase/Vector/{Copy1D,Copy2D,Copy3D}.cpp` and
`lib/Template/lib/RegBase/Vector/include/RegBase/DMAUtils.h`, and two tests. The strings
`invalid store l2_cache_mode` occur at HEAD and **nowhere** at `e878f3492`, so the store-side cache
policy is new in the very commit P2 builds — which is why §3.3's staged-template hash requirement is
load-bearing rather than bookkeeping. Earlier rounds produced their evidence at `e878f3492` with those
11 files modified in the worktree; the commit recorded those bytes verbatim, so **no reviewed content
changed**. Line numbers are from HEAD and must be re-checked if HEAD moves.

Authority for the *cut* is `docs/pass-cut.md` §2/§4 and its D018/D020 corrections. Authority for the
*input contract* is `docs/late-ir-contract.md`. Authority for what counts as acceptance is
`docs/acceptance.md` (P2 row) and `docs/p2-goal-1013.md` §4–§7.

## 0. Scope and completion criterion

**P2 completion criterion (goal §9):** a real Late input passes through the new entry and exactly one
HIVMC A5 lowering and produces an auditable, reproducible, correct target binary; the independent
tests and the Challenger review pass. **P2 does not require real-NPU numerical correctness** — that is
P3's ACL harness. A file that exists, or a compiler that exits 0, is not P2 success (goal §6.3).

## 1. What already exists, and why it is not the entry

Read live from `bishengir/tools/bishengir-compile/bishengir-compile.cpp`:

| | fact | line |
|---|---|---|
| D1 | `main` scans `argv` for `--target=<regbase arch>`; `isRegBasedArch` decides whether the regbase route is taken at all. An A5 attribute **inside** the input MLIR does NOT select the driver. | `:53-70`, `:161-163` |
| D2 | If `BISHENGIR_LEGACY_A5_REGBASE` is **present** (the code checks presence, not the value `1`), the process re-execs `bishengir-compile-a5` and never parses the MLIR itself. | `:161-166`, `:72-79` |
| D3 | Config is built by `BiShengIRCompileMainConfig::createFromCLOptions(regbase)`; the input is parsed; then `regbase::runRegBaseCompile(module, config, originalCLArgs)`. | `:158-190` |
| D4 | `runRegBaseCompile` calls `inferLayoutOptimization` unconditionally, `inferMixedCV` when `getEnableTritonKernelCompile()`, then `runRegBasePipeline`. | `regbase/Driver.cpp:63-87` |
| D5 | `runRegBasePipeline` runs a **retry loop** (`tryTimes = 2`) that clones the module and bumps `hfusionMaxBufferCountTuning` on failure — a failing Late input must fail loudly, not be re-scheduled. | `regbase/BiShengIRCompileMain.cpp:393-456` (loop), `tryTimes = 2` at `:376`, enclosing scope `:345-466` |
| D6 | Flow is chosen by `getCompileFlow`: Mixed (simd+simt) → PureSimt → **Simd** (the default branch). | `:53-59` |
| D7 | The Simd flow runs `buildBiShengHIRPipeline` → `buildFinalHIVMPipelines` → `buildBiShengHIRAVEToLLVMPipeline` → `addBitcodeAttrsToModule` → `runHIVMCCompileA5`. | `:400-452` |
| D8 | `buildBiShengHIRAVEToLLVMPipeline` calls `buildLowerAVEPipelines` when `getEnableHIVMCompile()`, and the conservative legalization tail `buildLowerToLLVMPipeline` when `getLowerToLLVM()`. These are **two separate branches**; the Late route needs only the second. | `regbase/PassPipeline.cpp:213-230` |
| D9 | `buildLowerAVEPipelines` has exactly **one** call site, inside that first branch — so an entry that runs only the `getLowerToLLVM()` branch skips it *structurally*, not by a flag. | `regbase/PassPipeline.cpp:223` |
| D10 | The `--enable-direct-hivm-lowering` flag **cannot** express this bypass: `setupHIVMAVEPipelineOptions` never populates `hivmAVEPipelineOptions.enableDirectHIVMLowering`, so the AVE guard is always true on this path (D018, falsified). Using it would be "disguising the entry". | `regbase/PassPipeline.cpp:150-210`; `HIVMAVE/Pipelines/HIVMAVEPipelines.cpp:39` |
| D11 | `addBitcodeAttrsToModule` uses `addIfExists`: a **missing** `../lib/meta_op.aiv.c310.<O0\|O2>.bc` is silently skipped, not an error. | `regbase/BiShengIRCompileMain.cpp:81-99` |
| D12 | Options are TableGen-generated: a `def XxxYyy` in `bishengir/include/bishengir/Tools/bishengir-compile/Options.td` with `compileConfigName = "BiShengIRCompileMainConfig"` generates the CLI flag `--xxx-yyy`, the field `xxxYyyFlag`, and `getXxxYyy()`/`setXxxYyy()` via `CompileConfigs.cpp.inc`. **There is no `--input-ir-stage` option today** (grep: zero hits). | `Options.td:26-45`; `Config.h:63` |

**Consequence.** The entry must be a *new, explicit stage selector* plus a *new Late branch that
returns before D4/D5/D6*; it must not be a reinterpretation of an existing flag.

## 2. P2-A — the entry

### 2.1 The stage selector

Add to `bishengir/include/bishengir/Tools/bishengir-compile/Options.td`:

```tablegen
def InputIRStage : FeatureControlOptions<
  "Select the IR stage of the input. 'hir' (default) is the ordinary RegBase "
  "front end; 'late-hivm' enters the Late HIVM/AVE pipeline, which bypasses "
  "HIR/HFusion, bufferization/planning, automatic synchronization, multibuffer "
  "and buildLowerAVEPipelines, and runs the legalization tail followed by "
  "exactly one HIVMC A5 lowering.",
  "std::string",
  "\"hir\""
>
```

**Type and default spelling verified against the generator, not assumed** (Lead, 2026-10-13). The
`ty` field is fed straight to `ConfigOption::getType()` (`ConfigOptions/Options.cpp:144-146`) as a raw
string, and the only string-typed options in this file use the literal **`"std::string"`** — e.g.
`InjectIRFromFile` and `SaveTemps` (`Options.td:330-348`) and `HIVMSyncSolverVersion`
(`Options.td:210-215`) — with the empty-string default written as
**`"\"\""`**. There is no `"string"` spelling anywhere in `Options.td`; using it would be an
unexercised generator path. The default here is likewise written `"\"hir\""`.

The three derived identities are exact, from `ConfigOption::ConfigOption` (`Options.cpp:124-134`) and
`lowerLeadingAcronym` (`Options.cpp:32-59`, which leaves `InputIRStage` alone past the first letter
because `result[1]` is lowercase) plus `convertToDashSnakeFromCamelCase` (`Options.cpp:61-85`):

| artifact | value |
|---|---|
| CLI flag | **`--input-ir-stage=<value>`** — `convertToDashSnakeFromCamelCase("InputIRStage")` |
| generated config field | `std::string inputIRStageFlag` — `cppName + "Flag"` |
| generated accessors | `setInputIRStage(const std::string&)` (chainable, returns `BiShengIRCompileMainConfig&`) / `std::string getInputIRStage()` — the accessors drop the `Flag` suffix |
| generated CLI registration | `CompileOptions.cpp.inc`: `llvm::cl::opt<std::string, /*ExternalStorage=*/true> inputIRStage("input-ir-stage", …, llvm::cl::location(inputIRStageFlag))`, plus its entry in the string→option lookup table |
| `ConfigUtils.cpp.inc` config↔pipeline-options conversion | **NOT generated** — `ConfigsToOptionsConversionGen.cpp:80-82` `continue`s on an empty group list |
| `PassOptions.cpp.inc` pass options | **generated anyway, into the synthetic `ALL` group** — see the correction below. This row is the one an earlier draft got wrong |
| compileConfigName | `BiShengIRCompileMainConfig` (inherited from `FeatureControlOptions`) |

The rows above are the **observed** shape of the generated code for the `std::string` option
`SaveTemps` (`saveTempsFlag` / `setSaveTemps` / `getSaveTemps` at `CompileConfigs.cpp.inc:1542-1552`;
`cl::opt<std::string,true> saveTemps("save-temps", …, cl::location(saveTempsFlag))` at
`CompileOptions.cpp.inc:977`), re-derived for the new `def`.

**Correction — there are two generators, and an earlier draft of this section read only one of them
and generalised.** `SaveTemps` carries a `passGroup`; the proposed `InputIRStage` would not. The
empty-group `continue` is real, but it lives in **`ConfigsToOptionsConversionGen.cpp:80-82`**, which
governs `ConfigUtils.cpp.inc` **only**. The pass-options generator is a different file and does **not**
behave that way: `PassOptionsGen.cpp:96-103` inserts **every** option into a synthetic
**`"ALL"`** group *before* testing the group list —

```cpp
for (const Record *R : specs) {
  ConfigOption cfgOpt(R);
  std::vector<StringRef> groups = cfgOpt.getPassGroups();
  passGroup2Options["ALL"].emplace_back(R);   // <-- unconditional
  if (groups.empty())
    continue;
  ...
```

— and `GEN_ALL_OPTION_REGISTRATION` is #define'd in **both** compile-pass translation units
(`bishengir/lib/Tools/bishengir-compile/PassPipeline.cpp:241`,
`bishengir/lib/Tools/bishengir-compile/regbase/PassPipeline.cpp:570`), each immediately followed by
`#include ".../PassOptions.cpp.inc"`. So a group-less `InputIRStage` **does** reach
`PassOptions.cpp.inc` and is emitted there as
`Pass::Option<std::string> inputIRStage{*this, "input-ir-stage", llvm::cl::desc(…), llvm::cl::init("hir")}`
for **both** `BiShengIRCompilePass` and `regbase::BiShengIRCompilePass`. `FeatureControlOptions` sets
only `compileOptionCategories` (`Options.td:59-65`); `passGroup` is declared per-option, and options
such as `EnableTritonKernelCompile` (`Options.td:72`) set it explicitly.

**What this does and does not change.** It does **not** change the routing decision: the Late branch
still reads `config.getInputIRStage()` in the driver, and that still needs no pass-options member, so
the decision to omit `passGroup` **stands**. What it changes is the *description* of the generated
surface, and it removes a premise from the §2.3.1 obligation-2 argument. `PassOptions : protected
llvm::cl::SubCommand` (`mlir/include/mlir/Pass/PassOptions.h:92`), so a `Pass::Option` is reachable
through **that pass's own `cl` sub-command namespace**, not through the top-level `--input-ir-stage`
flag — the two declarations are distinct `cl::Option` objects in distinct registries, so the top-level
one's `getNumOccurrences()` is not doubled by the pass-level one. But that is a **source-reading
argument, not a measurement**, and it is exactly the kind of claim this project has had to retract
before, so it is recorded as an **obligation to check in the A13 sub-runs** rather than as an
established fact: assert that the pass-namespace declaration cannot receive a value on a Late
invocation (and that supplying one does not reach the config the Late branch reads), and record the
`--help` output for both namespaces. The implementor adds a `passGroup` **only if** the value must
reach a pass as an option, and that addition must be justified rather than inherited. No `InputIRStage`
/ `inputIRStage` / `input-ir-stage` collision exists at the pinned HEAD.

**Why omitting `passGroup` is nonetheless the *safe* choice, and the half of the original claim that
survives.** The pass-options surface is not inert on its own — but the bridge between it and the config
is generated **per pass group**, and it points **one way only**: `genConfigToOptions`
(`ConfigsToOptionsConversionGen.cpp:37-43`) emits nothing but
`options.<cppName> = config.get<CapitalizedCppName>();`, guarded by `#ifdef GEN_<GROUP>_OPTION_SETUP`.
A group-less option is not in any group map at all — `emitConfigsAndOptionsConversion` `continue`s on
the empty group list (`:80-82`) *before* any group is populated — so **no generated path populates
`inputIRStage`'s `Pass::Option` from the config, and no generated path copies a pass-option value back
into the config**. The only other collector, `emitCollectConfigs` (`CompileOptionsGen.cpp:108-136`),
is filtered on `isSharedWithDownstreamToolchain`, which `InputIRStage` does not set (the default is 0 —
`EnableTritonKernelCompile` has to set it explicitly). So the option's *absence* of a `passGroup` is
what keeps the second declaration from becoming a second way to set the stage selector; the decision to
omit it is load-bearing rather than cosmetic, and it must not be reversed without re-deriving this.
What remains genuinely open is narrower than the paragraph above first stated: whether the
pass-namespace declaration is reachable **from argv** at all, and the `--help` evidence for both
namespaces. That is the obligation, and it is recorded rather than asserted.

**Checked on the generated artifacts, not only in the generator source — and this yields a falsifiable
prediction for the acceptance gate.** The three claims above were re-verified against the generated
files in the current (pre-P2) build tree:

| generated file | measured | prediction after the P2 build |
|---|---|---|
| `ConfigUtils.cpp.inc` | **0** occurrences of `inputIRStage` / `inputIRStageFlag`; 9 `GEN_<GROUP>_OPTION_SETUP` guards, each body an `options.X = config.getX();` | still **0** — the def is group-less, so no guarded block can contain it. A non-zero count means the option acquired a `passGroup` or the generator changed |
| `ConfigUtils.cpp.inc` | **0** occurrences of `GEN_PASS_OPTION_TO_CONFIG` | still **0**. The `#define` at `regbase/PassPipeline.cpp:540` guards **nothing** today: the reverse direction (pass options → config) is not generated at all, and the `// TODO now` beside it is the only trace of it. So "no path copies a pass-option value back into the config" is a property of the *generator*, not of this option's group list |
| `PassOptions.cpp.inc` | (pre-P2: no `InputIRStage`) | exactly **one** `Pass::Option<std::string> inputIRStage` per translation unit that defines `GEN_ALL_OPTION_REGISTRATION` — two in total |

**A bounded consequence on the pass route, recorded now because it is the same "silent ignore" family
as §2.2's two doors.** `regbase::BiShengIRCompilePass::runOnOperation` builds a **default-constructed**
`BiShengIRCompileMainConfig` and relies on `GEN_PASS_OPTION_TO_CONFIG` to map pass options into it —
which, per the table, is a **no-op macro guarding nothing**. So on that route *every* option is ignored
by the same mechanism, and a `--input-ir-stage=late-hivm` supplied there would be silently ignored and
run ordinary. This is **not a P2 regression** — the route ignores all pass→config mapping and did so
before P2 — and the Late entry's declared scope is the compile tool (§2.2 item 1), so no P2-A change is
implied. It is recorded because §2.2's worst case is "a Late request silently running the ordinary
pipeline", and this is a third door to it that §2.2 does not name. Whether the pass route is in scope
for the Late stage selector is a question for the Challenger at §8 step 8, not an implementation
requirement here.

The flag is therefore **`--input-ir-stage=late-hivm`**, exactly the goal's example. The default `hir`
makes the option's absence byte-for-byte the old behaviour. Any value other than `hir` or `late-hivm`
is a hard error at the entry (§4.1.9 requires that an ordinary request is untouched, and a typo must
not silently fall back to a different pipeline).

> An enum-typed option was considered and rejected: the TableGen option generator is exercisable in
> this file only for `bool`, `std::string` and list types, and a `std::string` with an explicit
> whitelist check gives the same rejection guarantee without depending on unexercised generator
> behaviour for a new enum class.

### 2.2 Where the branch goes — before D2, D4 and D5

1. **In `main`, before the `BISHENGIR_LEGACY_A5_REGBASE` delegation (D2).** `llvm::cl` is not parsed
   yet at `:161`, so this must be an `argv` scan exactly like `hasRegBaseTarget` (`:53-70`). If
   `--input-ir-stage=late-hivm` is present **and** `BISHENGIR_LEGACY_A5_REGBASE` is set, the process
   must **fail with an explicit diagnostic** instead of delegating: goal §4.1(8) requires that a Late
   request cannot be intercepted by the old driver. (Delegation is the *silent* failure mode here —
   the legacy binary would compile the input through the whole normal pipeline and could even
   produce a plausible ELF, which is the worst possible outcome for this project.)
2. **In `runRegBaseCompile`, as the first statement**, before `inferLayoutOptimization` and
   `inferMixedCV` (D4): if the config is in Late mode, call `runLateHIVMPipeline` and return. This is
   literally "分流" before both — goal §4.1(1).

   One statement sits between D4's two calls and `runRegBasePipeline` and must not be skipped by
   accident: `config.setClArgs(filterRegBaseForwardedHIVMCOptions(originalCLArgs))` at
   `regbase/Driver.cpp:79`. The filter (`regbase/Utility.cpp:124-149`) forwards every `-`-prefixed argv
   except `-o/--output` (+ its value), `--hivmc-args`, and anything containing `enable-lir-compile` —
   i.e. it looks like the mechanism by which CLI options reach the downstream toolchain. It is not:
   **measured, `BiShengIRCompileMainConfig::clArgsFlag` is written at `Driver.cpp:79` and read
   nowhere** — the only `getClArgs()` in the tree are two unrelated accessors (`Config.h:86` for the
   main config, `hivmc/Config.h:746` for a different class that fills its own field via
   `readCLArgs`), and the transfer helper never calls `hivmcConfig.clArgs(...)`. So a Late branch that
   returns before `:79` does not drop any option that the ordinary route would have delivered. That
   must be re-checked if the driver changes, because the failure mode is silent by construction — the
   dead store is indistinguishable from a live one by reading the call site alone.
3. The Late branch **never enters** `runRegBasePipeline` (D5/D6/D7), so it inherits neither the
   retry loop nor `buildBiShengHIRPipeline`, `buildFinalHIVMPipelines` or
   `buildBiShengHIRAVEToLLVMPipeline`.

**Late detection must be independent of target dispatch.** Measured at `bishengir-compile.cpp`:
`hasRegBaseTarget(argc, argv)` is evaluated **three separate times** — `:114` (legacy delegation),
`:147` (`config = …::createFromCLOptions(regbase)`), and implicitly the `if (regbase)` dispatch that
sends the module to `runRegBaseCompile` at `:184-186`, with everything else going to
`runBiShengIRPipeline` at `:188`. Therefore a Late request is **not** automatically on the regbase
route: `bishengir-compile --input-ir-stage=late-hivm` with **no** `--target=`, or with a non-RegBase
target, reaches `:188` and runs the **ordinary membase pipeline**, silently ignoring the Late request
and possibly emitting a plausible artifact. That is the same worst-case failure mode as the legacy
delegation in item 1, by a different door.

The rule is therefore: the Late stage value is scanned from `argv` **once, unconditionally, before
`:114`**, and:

- Late + `BISHENGIR_LEGACY_A5_REGBASE` → explicit diagnostic, no delegation (item 1);
- Late + no target, or Late + non-`isAscend950` target → explicit diagnostic **at `:114`**, before
  parsing and before `:153`, never a fall-through to `:188`;
- Late + RegBase target → the branch in item 2.

The check must not be written as a predicate consulted only inside the `if (regbase)` region. Its
negatives are: (a) Late without a target produces a Late-specific diagnostic and **no** artifact;
(b) an ordinary request with no `--input-ir-stage` is byte-for-byte unaffected (§4.1(9)). Both are
P2-D cases.

### 2.3 Mutual exclusion and option rejection

In Late mode the entry must **reject, before any destructive transformation**, a request that carries
any of: `--enable-triton-kernel-compile`, `--enable-triton-ir-compile`,
`--enable-simd-simt-mix-compile`, `--pure-simt`, `--enable-sanitizer`, `--enable-debug-info`,
`--enable-debug-variables` (and `--save-temps` is allowed, as a diagnostic only), plus
`BISHENGIR_LEGACY_A5_REGBASE`. Each rejection prints the offending option name. Goal §4.1(7).

**Every option named in this section and in §2.3.1 was checked for existence on the pinned binary, in
the container** (`B=/home/s00653124/workspace/AscendNPU-IR-1/build/bin/bishengir-compile;
$B --help | grep -- "--<name>"`; the binary cannot run on the host — it needs the container's newer
glibc/libstdc++, which is itself a reminder that all these probes belong in-container). Three of the
names an earlier draft used were **wrong**, and one names an option that does not exist:

| earlier draft | measured reality |
|---|---|
| `--multi-kernel` | **absent**; the real flag is **`--enable-multi-kernel`** |
| `--enable-auto-multibuffer` | **absent**; the real flag is **`--enable-auto-multi-buffer`** |
| `--compile-host` | **absent, and not merely misspelled** — `$B --compile-host=1` exits `0` with `Unknown command line argument '--compile-host=1'`. `CompileHost` is a `BiShengIRCompileInternalStateOpt` (`Options.td:1550-1554`) documented in-file (`:1547-1549`) as *"not currently surfaced as normal generated CLI/pass options"*; the config field exists (default `false`), the CLI flag does not |

That last row is a **test-design hazard, not a cosmetic fix**. A negative written as
`not bishengir-compile --compile-host …` would "pass" — but because the driver rejected an *unknown
option*, not because Late mode refused a host compile. That is precisely the failure goal §7 B
prohibits ("must fail for the expected semantic assertion, not merely because … "). The `CompileHost`
guard is therefore a **config-state** assertion (`config.getCompileHost()` is false, and the Late entry
never reaches the `buildLowerHACCToLLVMPipeline` early return), and its test must be written as such.
The same care applies to every row of the §2.3.1 table: a rejection test must demonstrate the *named*
diagnostic, never a generic unknown-option error.

`--target=Ascend950PR_9579` (or any `isAscend950` target) is **required**: Late mode is only defined
for the Ascend950PR AIV path (§4.1(6)). A non-Ascend950 target in Late mode is rejected, not
downgraded.

#### 2.3.1 The ABI/bitcode/sync/output option policy — the transfer helper is the contract

`HIVMCFromBiShengIRConfig` (`regbase/BiShengIRCompileMain.cpp:281-326`) is the **only** bridge from
`BiShengIRCompileMainConfig` to `HIVMCMainConfig`. Measured, that helper transfers, among others:
`targetBackend`, `appendBishengOptions`, `setExtraDeviceBCPaths(config.getLinkAicoreBitcode())`,
`injectBarrierAllSync(config.getEnableHIVMInjectBarrierAllSync())`, `setDisableFMA`,
`setSaveLinkedIR`, `saveTemps`, `setSharedDynamicSize`, `setNumWarps`/`setThreadsPerWarp`/
`simtStackLimit`, the triton/mix/simt/sanitizer/debug switches, and the meta-op bitcode selection
inputs.

**The Golden's ABI was *not* produced through this helper, and an earlier draft of this section said it
was.** The frozen P1 artifact states in its own header that it is *"HAND-AUTHORED text, not compiler
output"* (`vadd_late_p1v2.mlir:9-11`, sha `d1023ac1…`), and P1 produced **no** accepted A5 binary — the
artifact is the **input** to the P2 Late pipeline, not an output of any route. So there is no
helper-produced reference ABI to compare against, and the helper cannot be "the contract the Golden was
built through". What it is instead is the **single** bridge (§6.3's C-rows are therefore P2's *first*
ABI measurement, not a regression against a prior one).

**Policy: Late mode calls the transfer helper verbatim.** It adds no field, removes no field, and
reaches into no `HIVMCMainConfig` member directly. That removes one *class* of suspect — missing or
extra plumbing on the Late route — and nothing more. It does **not** establish the ABI: the helper reads
config fields, so a Late entry pinning the §2.3.1 rows to different values would produce a different ABI
*through the same helper*; and with no helper-produced reference in existence, agreement has nothing to
agree *with*. The ABI is therefore asserted on the **emitted artifact** — the §6.3 rows (C1–C11) and the
`p2-binary-abi.md` C-row measurements — and each of those is a fresh measurement whose expected value
comes from the frozen artifact and the template source, never from "the same helper ran".

**Consequence — options the helper does not transfer must not be silently ignored.** A verified
instance: `EnableStaticBarePtr` (`Options.td:153-160`) defaults to `true`, is
`isSharedWithDownstreamToolchain = 1`, and carries **`emitGetterSetter = 0`**, so the CLI writes
straight into `enableStaticBarePtrFlag` through `cl::location` and no config accessor exists
(`CompileConfigs.cpp.inc:981`, `CompileOptions.cpp.inc:540-543`). The transfer helper never calls
`barePtrCallConvForStaticShape(...)`, and the A5 consumer reads it from the config
(`A5/PassPipelineA5.cpp:50-51`), whose default is `true` (`hivmc/Config.h:830`). Therefore on the
regbase route **`--enable-static-bare-ptr=false` is silently ineffective** and the bare-ptr call
convention stays on. Late mode inherits this exactly; it must not "fix" it (that would break ABI
agreement with the Golden) and must not let a user believe the flag took effect. Each such option is
either proved irrelevant and recorded, or rejected in Late mode with a diagnostic naming it.

The options to adjudicate this way, because each can move ABI, bitcode, synchronization or output.
Each row's disposition is now **frozen** — "reject or pin" is not a disposition, it is a deferral, and a
Late invocation must have exactly one allowed value per row:

| option | touches | Late-mode disposition |
|---|---|---|
| `--enable-static-bare-ptr` | LLVM call conv (ABI) | **reject an explicit `=false`**; an explicit `=true` is accepted because it agrees with the effective value. Not transferred, so `=false` is silently ineffective and the convention stays `true` (`hivmc/Config.h:830`); accepting `=false` would mean accepting a request the toolchain does not honour |
| `--link-aicore-bitcode` | extra device bitcode list | **reject** — a substitute template must never mask the required C310 AIV one (§3.3) |
| `--enable-hivm-inject-barrier-all-sync` | injects `--cce-auto-sync=bar-all` | **reject** — automatic sync injection is forbidden (§3.2) |
| `--disable-fma` / `--disable-decompose-reduction` / `--disable-reorder-instruction` | instruction selection | carried unchanged by the helper — so unlike the bare-ptr row these *do* take effect; **pin each to `false`** and reject an explicit `true` with a diagnostic naming that option. The pinned values are the effective ones because **no arch-dependent default moves them**: `DisableFMA` (`Options.td:962-968`, `FeatureControlOptions`), `DisableDecomposeReduction` (`:1340-1346`) and `DisableReorderInstruction` (`:1348-1354`, both `SIMTOptimizationControlOptions`) each declare `bool` with default `"false"`, and a tree-wide search for `setDisableFMA`/`setDisableDecomposeReduction`/`setDisableReorderInstruction` finds only the forwarding sites (`regbase/BiShengIRCompileMain.cpp:318`, `:325-326`; `BiShengIRCompileMain.cpp:212`, `:218-219`) plus the config's own setters — no defaults function writes them. They are genuinely consumed on the regbase route (`PassPipeline.cpp:349-350` feeds `disableDecomposeReduction`/`disableReorderInstruction` into a pass options struct; the A5 side reads `setDisableFMA`), so instruction selection is part of the binary identity rather than a dead flag |
| `--save-linked-ir` | **not an output path — an artifact type.** `SaveLinkedIR` (`Options.td:1132-1139`, `bool`, default false) switches the ccec output extension `.o`→`.ll` and its filename gains `_linked` (`hivmc/A5/HIVMCMainA5.cpp:68-71`), and appends `-Xclang -disable-llvm-optzns -S -emit-llvm` to the ccec argument list (`:158-163`) | **reject in Late mode** — it both changes what is produced (LLVM text, not an object) and disables the optimization the binary identity depends on. It cannot appear in any invocation that is meant to produce the accepted binary |
| `--save-temps` | output **directory** only — `std::string`, default `""` (`Options.td:339-345`) | allowed as a diagnostic; must write outside the pinned paths. Kept separate from `--save-linked-ir` above: one is a destination, the other is a transformation |
| `--enable-optimized-metaop` | selects `meta_op.aiv.c310.<O0\|O2>.bc` (`Options.td:818-826`, `bool`, **default `true` = O2**) | **pin explicitly to `true`** — the choice is part of P2's recorded binary identity (§3.3), and it must be pinned by an explicit argument rather than inherited from a default |
| `--enable-tuning-mode`, `--enable-auto-multi-buffer`, `--multibuffer-mode` | tuning / multibuffer | **reject** — no tuning retry and no multibuffer in Late mode (§4). Spellings measured on the pinned binary (`--enable-auto-multibuffer` does **not** exist) |
| `--append-bisheng-options` | passes flags to the downstream compiler | **reject** — an arbitrary downstream flag set is not part of the frozen contract |
| `--enable-hivm-auto-storage-align` | `ConvertHIVMToStandardOptions::isOpsAligned` in the tail | **pin `true`** — TableGen default is already `true` (`Options.td:662-666`); pinning states it rather than inheriting it |
| `--enable-lib-call-no-inline` | `ConvertHIVMToStandardOptions::markLibCallNoInline` in the tail | **pin `true`** — and that is *not* the TableGen default. See the arch-dependent-defaults note below |
| `--enable-hivm-unit-flag-sync` | synchronization mode of the inject-sync pass | **pin `true`** — same mechanism; a synchronization-affecting option may not float |
| `--enable-preload` | CV-pipelining skew | **pin `true`** — same mechanism |
| `--enable-multi-kernel`, `--enable-bin-relocation`, `--enable-symbol-analysis`, `--enable-manage-host-resources` | host/link packaging | out of scope for P2; reject rather than guess. Spellings measured: `--enable-multi-kernel` (not `--multi-kernel`). `--compile-host` is **not a CLI option at all** — see §2.3 |

**One rule makes the table mechanical, and it is what "pinned" means above:** every row that is not
rejected is a row whose value must be **supplied explicitly** by the Late entry. An explicit argument
is the only way to make `hasExplicitCLOption` true, which is in turn the only way the arch-dependent
defaults below cannot move the value out from under a recorded binary identity. **Any explicit value
that differs from the pinned one is rejected with a diagnostic naming the option and both values** —
never silently overridden, because silent override would make the compile log disagree with what the
binary was actually built from.

**"Explicit" is a parse-time property, and that fixes exactly *where* the Late entry may inject the
arguments.** `hasExplicitCLOption` (`BiShengIRCompileConfig.cpp:366-371`) returns true only when
`getNumOccurrences() != 0`, and occurrences are counted by `cl::ParseCommandLineOptions`, called
**once** at `bishengir-compile.cpp:102` from `registerAndParseCLIOptions` (`:89`), whose only call site
is `main`'s `:144`. An entry that injects or rewrites the pinned arguments **after** that call has not
made them explicit: it changes the config field, and then `applyArchDependentCompileDefaults` — invoked
later, from `createFromCLOptions` — writes its own value over it, because `getNumOccurrences()` is
still zero. (For three of the five rows the arch default happens to **equal** the pin, so that
overwrite is benign *today* — which is precisely why the mistake would go unnoticed until a future
default changed, and why the entry must assert explicitness rather than assume its write survived.)

Concretely, `main` fixes the deadline, and it is **earlier than the parse**. It raw-scans the target at
`:114` (before the parse, the same way `hasRegBaseTarget` does at `:56-70`), then copies `argv` into
`originalCLArgs` at `:121-124` — also before the parse — and that vector, not `argv`, is what is
forwarded downstream at `:186` (`runRegBaseCompile(…, originalCLArgs)`, and thence
`filterRegBaseForwardedHIVMCOptions`). So the pinned arguments must be written into `argv` **before
`:121`**: after that point `hasExplicitCLOption` can still be satisfied (up to `:144`) while the
downstream command echo carries a different set of flags, which is the log-vs-binary divergence this
section exists to prevent.

Placement **before `:114`** — alongside the Late detection and target validation of §2.2, which must
already run there — satisfies this with room to spare, and the target the validator needs is available
from the same raw scan rather than from the parsed config. Placing it later than `:121` does not.

The entry's own assertion that each pinned option is explicit must read back the **post-parse
`getNumOccurrences()`**, not its own bookkeeping — otherwise the check passes on exactly the mistake it
exists to catch.

**How the pinned arguments are injected — obligations the entry must meet, not implementation detail
free for the implementor to choose.** Round 5's APPROVE (Challenger, `gpt-6.1-sol`, 2026-10-13) attaches
these to the design as conditions on that verdict, so they are part of the frozen design:

1. **Duplicate user occurrences are a rejection, not a merge.** `llvm::cl` accepts a repeated option and
   `getNumOccurrences()` counts *every* occurrence, so a user passing `--enable-preload=true` alongside
   the entry's own pinned occurrence yields two occurrences and a downstream echo carrying both. The
   entry must therefore **reject a contradictory user occurrence before injecting anything**, and the
   rejection must name the option and both values (§2.3.1's one rule). It must not "fix up" a duplicate
   by rewriting the user's argument: that is silent override wearing a different hat.
2. **Exactly one canonical occurrence per pinned option reaches both the parse and the downstream
   echo.** After injection, the post-parse `getNumOccurrences()` for each pinned option must be **1**,
   and the vector forwarded at `:186` must contain the **same** single canonical argument. Asserting
   only the config value would miss a doubled echo; asserting only the echo would miss a config moved by
   a default. Both, or the row is unproven. **The stage selector itself is the option this row is least
   safe to argue from source about**: §2.1 records that `--input-ir-stage` is declared in **two**
   namespaces (a top-level `cl::opt` and a `Pass::Option` inside each compile pass), so "one occurrence"
   must be shown by measurement for both, not inferred from `llvm::cl`'s registry structure
   (`p2-verification.md` §A coverage delta 4).
3. **`argc`/`argv` are not mutated in place.** `main` reads `argv` at `:114` and `:121-124`, and no
   source path reconciles the captured vector afterwards — which is the mechanical reason the deadline
   is `:121`. The entry builds the augmented argument vector **once**, and that same vector is what the
   target scan and the `originalCLArgs` capture see. Writing extra `argv` slots, or reordering in place
   while a stale capture survives, are both excluded.
4. **Scope of the contract, stated so it cannot be read two ways.** This holds **for argv injection
   under the current forwarding contract**: `originalCLArgs` → `Driver.cpp:79` → `config`'s `clArgs`.
   The Challenger notes that an implementor *could* deliberately reconcile after the capture via
   `config.setClArgs`; that is not impossible, but it **changes the forwarding contract** and would
   require its own identity test (config and downstream argv agreeing). The design chooses argv
   injection, so a later `setClArgs` reconciliation is **out of contract** and must not be introduced
   without amending this section.

**A TableGen default is not the effective default on the regbase route, and three rows above depend on
that.** `applyArchDependentCompileDefaults`
(`Tools/bishengir-compile/BiShengIRCompileConfig.cpp:374-383`, called at `:463`, `:481`, `:486`) returns
early unless `isRegBasedArch(config.getTarget())`, and otherwise sets, for each option **only when the
user did not pass it explicitly** (`hasExplicitCLOption`, `:366-371`):

```cpp
if (!hasExplicitCLOption("set-workspace-multibuffer"))     config.setSetWorkspaceMultibuffer(2);
if (!hasExplicitCLOption("limit-auto-multi-buffer-buffer"))config.setLimitAutoMultiBufferBuffer(MultiBufferStrategy::NO_LIMIT);
if (!hasExplicitCLOption("enable-hivm-unit-flag-sync"))    config.setEnableHIVMUnitFlagSync(true);
if (!hasExplicitCLOption("enable-preload"))                config.setEnablePreload(true);
if (!hasExplicitCLOption("enable-lib-call-no-inline"))     config.setEnableLibCallNoInline(true);
```

So `--enable-lib-call-no-inline`, `--enable-hivm-unit-flag-sync` and `--enable-preload` all read
**`false`** in `Options.td` (`:1200-1206`, `:584-588`, `:653-660`) and are all effectively **`true`** on
any Ascend950PR Late run. The TableGen file for `--enable-lib-call-no-inline` even says so in its own
description — *"Enabled by default on RegBase and disabled by default on membase."* A Late-mode check
written from the option list would therefore get three rows backwards. This is also why the disposition
is **pin**, not merely "don't set": the pinning argument must be passed explicitly, so that
`hasExplicitCLOption` is true and the arch-dependent function cannot move the value underneath a
recorded binary identity.

The two conversion-options rows are a **new finding from reading the tail's own source**, not from the
option list: `buildLowerToLLVMPipeline` builds `ConvertHIVMToStandardOptions` from the config before
adding the pass (`regbase/PassPipeline.cpp:253-260`):

```cpp
ConvertHIVMToStandardOptions hivmToStdOptions;
hivmToStdOptions.isOpsAligned = config.getEnableHIVMAutoStorageAlign();
hivmToStdOptions.markLibCallNoInline = config.getEnableLibCallNoInline();
if (config.getEnableSanitizer() || config.getEnableDebugInfo())
  hivmToStdOptions.markLibCallNoInline = false;      // silent override inside the tail
```

So the tail is **not** parameterless: two config values change what `ConvertHIVMToStandard` does, and
a third pair (`sanitizer`, `debug-info`) silently overrides one of them. The frozen Golden's IR was
produced with whatever these were at P1 time, so Late mode must either pin them to the P1-verified
values or reject any request that sets them — it must not inherit them silently.

This table is a **policy**, and each row needs its own executed check (P2-D): the expected outcome is
either an early rejection naming the option, or the option being demonstrably inert for that stage.
An option marked "not transferred" must be shown inert by measurement, not by reading the helper.

### 2.4 Late input validation (§4.2) — before any destructive transformation

A new validator, run on the parsed module and on the config, with **fail-closed** behaviour. It checks,
in this order:

| # | check | reject condition |
|---|---|---|
| V1 | stage identity | the module is not a Late HIVM/AVE stage: it contains HFusion/tensor ops, `llvm.func`/`llvm.*`, or any op outside the **exact** P1-verified whitelist enumerated below — the whitelist is a closed list of fully-qualified operation names, **never** a prefix pattern like `hivm.*` or `ave.*` (see below); **or** any function carries `mix_mode` / any `FilterPassesAttr` (either activates the Mix/Aiv pass-execution policy — see §3.2) |
| V2 | target agreement | `hacc.target` present in the module at parse time **and** equal to the CLI target; a module without it is rejected in Late mode (the normal path's `AppendDeviceSpec` is not allowed to invent a missing target silently here) |
| V3 | module/core metadata | `hivm.module_core_type = #hivm.module_core_type<AIV>` present; entry carries `hacc.entry` + `hacc.function_kind<DEVICE>` + `hivm.func_core_type<AIV>` |
| V4 | entry ABI | exactly three GM arguments, `memref<524288xf32, #hivm.address_space<gm>>` in A/B/C order, void return, no extras |
| V5 | VF/pb32 | the VF function carries `hivm.vector_function` and its call site carries the same marker; the predicate carries an **explicit** pb32 carrier (the contract's D009 requirement — the canonical template's silent default is a known defect, `late-ir-contract.md` §5) |
| V6 | geometry/sync | UB pointer_cast bases 0/65536/131072 bytes; three DMA ops with 32768-byte payloads; the four Golden set/wait ops; **exactly one** `PIPE_ALL` and it is the entry's terminal op |
| V7 | store cache | `l2_cache_mode = 4 : i32` on the entry's single store, in the D012-approved conservative DMA subset (rank-1 unit stride, zero static source offset, **exactly one** constant 32-byte-aligned `pointer_cast` address operand); anything outside the subset is rejected with a diagnostic rather than degraded |

V7 is where "cache4 符合 D012 已批准的保守 DMA 子集" (§4.2) becomes executable. Any value other than
0/1/2/4 is already rejected by the `StoreOp` verifier added in D012; V7 is the *narrower* Late-mode
gate.

**V7 reads the parsed op, never the artifact's comments — the two disagree, and that is already on
record.** The frozen artifact's header (`:25`,`:27`) and its inline note (`:184-188`) assert the
opposite of what the file contains: they say the store "carries NO cache mode", that the pre-D012
spelling `{l2_cache_ctl = 4 : i64}` "PARSES BUT IS SILENTLY DISCARDED", and that "the emitted libcall
receives 0, not 4". The op itself (`:189`) carries `{l2_cache_mode = 4 : i32}` — the **recognized**
StoreOp attribute, whose valid store values are exactly {0,1,2,4} (`Dialect/HIVM/IR/HIVMDMAOps.td:181,198`;
`HIVMDMAOps.cpp:391-413`). This contradiction is a **known, recorded fact, not a new finding**:
`challenger-d025-zqumt69b/entry-evidence/p1v2/scoped-vf/INTEGRITY.json` carries a `stale_header_comment`
field — "written BEFORE the D012 fix … That prose is STALE. It is deliberately NOT edited, because
editing it would change the reviewed artifact identity (`d1023ac1…`, APPROVED by the independent
Challenger)". MLIR parsing discards comments, so the prose cannot affect a parsed-input check. The
artifact is immutable and **must not be edited** to remove the contradiction. The same record states
that the frozen P1 checker's `B8.TERMINAL` branch (`p1_check.py:1561-1576`) has **no FAIL path** — it
is a recorder, not an asserter — so P2's terminal-barrier row (`p2-pass-trace.md` D11) must be a real
assertion of our own, not a reuse of the P1 oracle.

**The validator must be a real preflight, and it must not be the conversion's own check.** Measured:
the D012 conservative store predicate lives in `verifyStoreCacheMode`
(`Conversion/HIVMToStandard/regbase/HIVMToStandard.cpp:1131-1160`) and is invoked from the
**conversion pattern** — i.e. it fires *during* `ConvertHIVMToStandard`, which is item 10 of the
§3.1 tail. Two consequences:

- A failure there happens **after** items 1–9 have already run on the module, so it cannot satisfy
  goal §4.2's "在破坏性变换之前" (before the destructive transformation). V1–V7 must run as a
  separate preflight on the **unmodified** parsed input, before item 1 of the tail and before the
  device-spec step.
- Reusing or hoisting that predicate would make the Late gate a copy of the very code it is meant to
  guard, and would tie the gate's diagnostics to a conversion utility. V7 is therefore its own
  implementation over the parsed op, and it may *agree* with `verifyStoreCacheMode`'s conservative
  subset without calling it. The agreement itself is worth an executed check: a module inside the
  Late subset must pass both, and a module outside it must be rejected by V7 with a preflight
  location, not by the conversion mid-tail.

Two implementation constraints on V7, both load-bearing and both easy to get subtly wrong:

- **The single-operand rule is not cosmetic.** `getConstantUBByteAddr`
  (`HIVMToStandard.cpp:1241-1250`) **rejects multiple address operands** because A5 lowering consumes
  **operand zero only**. V7 must retain exactly that rule; accepting a multi-operand form would admit
  an address whose effective base differs between validation and lowering.
- **V7 must test the nested rank/scalar dispatch, not just the textual memref strides.** The
  conversion reaches the predicate through dispatch structure (`:1104-1112` invokes it while
  converting/reducing the store), so a shape can carry innocuous-looking strides and still land on the
  scalar fallback. The negative is therefore a mutated store that reaches that fallback **while
  keeping cache mode 4** — V7 must reject it before any mutation, with a diagnostic naming the
  unsupported DMA shape.

The independent `StoreOp` verifier (`Dialect/HIVM/IR/HIVMDMAOps.cpp:391-413`) checks valid values and
the target; it is a third check, not a substitute for V7.

**Whitelist source of truth for V1 — an executed census, not a reading of the pretty-printed file.**
An earlier draft of this section listed the operations from the artifact's *custom* (pretty-printed)
form, and that form is misleading in exactly the way a whitelist must not be: it hides the resolved
names of the enclosing and terminating operations. **Measured** on the frozen artifact
(`p1v2-20261013-d025/snapshot/impl/vadd_late.mlir`, `d1023ac1…`) with the pinned
`build/bin/bishengir-opt` in the container:

```
bishengir-opt vadd_late_p1v2.mlir --mlir-print-op-generic -o -
```

Generic form is the right oracle because it prints every operation's **canonical** name. The census
has **21** distinct operations:

| op | count | | op | count |
|---|---|---|---|---|
| `builtin.module` | 1 | | `hivm.hir.store` | 1 |
| `func.func` | 2 | | `hivm.hir.set_flag` | 2 |
| `func.return` | 2 | | `hivm.hir.wait_flag` | 2 |
| `func.call` | 1 | | `hivm.hir.pipe_barrier` | 1 |
| `arith.constant` | 7 | | `hivm.hir.pointer_cast` | 3 |
| `arith.muli` | 1 | | `ave.hir.pge` | 1 |
| `arith.index_cast` | 1 | | `ave.hir.vload` | 2 |
| `memref.subview` | 6 | | `ave.hir.vadd` | 1 |
| `scf.for` | 1 | | `ave.hir.masked_store` | 1 |
| `scf.yield` | 1 | | `hivm.hir.load` | 2 |
| `hivm.hir.get_block_idx` | 1 | | | |

Three corrections to the earlier list, all of which would have made V1 **reject the frozen input**:

- `module` → **`builtin.module`** (the top-level op's canonical name);
- `return` → **`func.return`** (the VF helper's and the entry's terminators);
- **`scf.yield` was missing entirely.** `scf.for` has exactly one body terminator, `scf.yield`, once.
  Both `func.return` and `scf.yield` are terminators that a whitelist built by skimming the body would
  omit — and a V1 that rejected the accepted fixture would fail every Late test for the wrong reason,
  which is the failure §7 B forbids.

V1's whitelist is this table's op column, **exactly**, with no wildcard rows: `cf` is deliberately
**excluded**, because `ConvertSCFToCF` runs *later* in the tail, so a `cf` op in the input means the
input is not the P1 Late stage. The whitelist must be implemented as a closed set of full operation
names compared for equality — a prefix test such as `op.getName().startswith("hivm.")` would admit
`hivm.hir.mmad`, `hivm.hir.copy` and every other HIVM op, i.e. would not distinguish the Late stage
from a pre-Late one at all. The census command above is also the **regeneration procedure**: if a
future accepted fixture changes the op set, re-run it and re-derive the list; never widen the pattern.

**Implement the validator as a registered MLIR pass, defined inside the regbase library.** Rationale,
and it is a testability argument rather than a style one: as a registered pass (e.g.
`-hivm-verify-late-input`) the verdict can be exercised with `bishengir-opt` on any candidate module,
**independently of the driver and of the production `bishengir-compile` binary**, which is the
independence `docs/acceptance.md` and goal §7 demand. The CLI target needed by V2/V3 is supplied as a
pass option. It also appears in the pass instrumentation of §5, so "validation ran before anything
destructive" becomes a measured ordering fact rather than a code reading. The pass must **fail closed**
(`signalPassFailure`) and must not mutate the IR.

**Where the pass lives is a two-part requirement, and linkage alone satisfies neither part.**
An earlier draft of this section claimed the location was *forced* by the link topology. That was
too strong, and the correction matters because a pass that is linked but not registered is invisible
to `bishengir-opt` while still costing a relink. Both parts are measured:

1. **Linkage** — the pass's implementation must end up in a library linked into the binary that runs
   it. Measured: `bishengir-opt` links the global property `BISHENGIR_LIBS`
   (`bishengir/tools/bishengir-opt/CMakeLists.txt:24`), and `add_bishengir_library` appends
   `BiShengIRRegBaseCompileLib` to that same property (`cmake/modules/AddBiShengIR.cmake:28-38`);
   confirmed by `bishengir-opt --help` listing `--bishengir-compile-regbase`. So the regbase library
   is a **sufficient** home — not a necessary one. Any library in `BISHENGIR_LIBS` would do; the
   regbase library is chosen because the Late entry lives there anyway.
2. **Registration** — linkage does **not** register a pass. Registration is a separate, explicit act:
   `bishengir::registerAllPasses()` (`include/bishengir/InitAllPasses.h:71`) is a hand-written
   composition of per-dialect `register*Passes()` calls, each generated by `GEN_PASS_REGISTRATION`
   (e.g. `include/bishengir/Dialect/HIVM/Transforms/Passes.h:466`). The new pass is therefore added to
   a `.td` whose generated registration function is already reached from that composition, **or** a
   new `register…Passes()` call is added to `InitAllPasses.h`. The existing RegBase registration is
   the exemplar: `bishengir::regbase::registerBiShengIRCompilePass()` at `InitAllPasses.h:110` — a
   hand-written call, not something linkage produces. Declaring `def` and building is not
   sufficient; the implementor must show `bishengir-opt --help` listing the new argument, which is the
   executed proof that registration happened.

The build-identity consequence stands independently of both: because the regbase library is in
`BISHENGIR_LIBS`, a change there makes `bishengir-opt` out of date, which §3.3 handles.

### 2.5 Device spec — the three-state rule (§4.2)

`hacc::createAppendDeviceSpecPass` overwrites unconditionally and is **not idempotent**
(`pass-cut.md` §2). The Late entry therefore does not inherit it blindly:

1. **Absent** → build the full spec from the verified target attributes and run `AppendDeviceSpec`
   **exactly once in total** — counted by the pass instrumentation of §5 and aggregated over every
   entered manager (§3.2 precondition 6), because "once per scope" would let a second manager append a
   second time and the pass is not idempotent.
2. **Complete and matching** → validate it field by field against the CLI target and reuse it; do not
   run the pass.
3. **Partial or conflicting** → **reject** with a diagnostic naming the differing field. An incomplete
   spec must never reach a later pass.

**"Complete" cannot be inferred from presence — it must be enumerated.** Measured:
`hacc::utils::getNPUTargetSpec` (`Dialect/HACC/Utils/Utils.cpp:179-190`) returns an `optional`
interface, so it distinguishes *no spec* from *an interface* — but a **partial** spec still returns an
interface. There is no single completeness predicate. Completeness is therefore an explicit
requirement list: V2 queries **every required `DeviceSpec` key** through the interface's
`getSpecForIdentifierEnum` (`include/bishengir/Dialect/HACC/IR/HACCInterfaces.td:138-157`) and
compares **every value, including `ARCH`**, against the selected `Ascend950PR` target. A complete spec
has **14 entries** in the current generated specification. The absence of such a check is a known
crash mode: a partial `dlti.target_system_spec` can fault during conversion, which is what makes the
partial-spec negative mandatory rather than optional.

The three-state experiment to run (§7 step 7, P2-D): feed the validator (1) no target spec → append
once; (2) the complete current spec → reuse, **zero** appends; (3) a spec containing only `UB_SIZE` →
explicit rejection; and additionally (4) a complete spec with one field changed (`ARCH` or `UB_SIZE`)
→ an explicit diagnostic naming that field.

The overwrite is measured, not inferred: `AppendDeviceSpec.cpp:145-148` warns
`"Overwriting the device spec..."` when a spec is already present and then calls
`setTargetDevice`/`setNPUTargetSpec` **unconditionally** — the warning is informational, not a guard.
Note the pass's other early-out: `:129-130` returns immediately when **both** the option target and
the IR target are unknown, so in state 1 the pass does nothing unless the CLI target is supplied —
which is why V2 requires a target. Given all that, the
decision between the three states must be made by the **preflight validator (V2) before the pass is
scheduled** — the pass cannot be relied on to detect state 3, because by then it has already
overwritten. The pass runs only in state 1, and its execution count must be exactly 1 in that state
and 0 in state 2, both read from the pass instrumentation of §5. Note also that for a regbase module
the pass's only effects are `setTargetDevice` and `setNPUTargetSpec` — the `HIVMCVersion` /
`HIVMCCompatiblePrint` attributes it also handles are membase-only — so state 2 is a genuinely
pass-free path, not merely a semantically equivalent one.

**Case 2 is now supported by an executed experiment, not by reading source.** The Challenger ran, in
the project container with the pinned `bishengir-opt` `0761c6e6…` on the frozen input `d1023ac1…`,
parse/print **with** and **without** `--hacc-append-device-spec=target=Ascend950PR_9579`: both exited
**0** and emitted **byte-identical** canonical IR (SHA
`884c947d25f491184adf975deeca60fd8afbd3808107e624e1f04a1d39a980e8`). So for the frozen
target/spec the pass is a no-op *for this input*, which is exactly the case-2 claim. Caveats recorded
with it: the experiment used **default core overrides**, and custom core-count options can change spec
entries, so reuse requires validating against the intended defaults or **rejecting those overrides**;
and the full-tail and binary comparisons remain **NOT_RUN** — if a later binary hash differs while the
IR does not, investigate nondeterminism before concluding the pass must run.

The `UB_SIZE=2031616 bits` device-capacity value must not be confused with the kernel's
`dyn_shared_memory_buf = 196608 bytes` launch contract (§4.2, and `late-ir-contract.md` §2). These are
different quantities in different units; the validator asserts them separately.

## 3. P2-B — the pass sequence

### 3.1 The sequence

`runLateHIVMPipeline(module, config)` is, in order:

```text
1  Late input validation (§2.4, §2.5)            -- fail-closed, before anything destructive
2  device spec: exactly one AppendDeviceSpec iff case 1 (absent)
3  buildLowerToLLVMPipeline(pm, config)          -- regbase/PassPipeline.cpp:240-295, the
                                                    conservative legalization tail, unchanged
4  addBitcodeAttrsToModule(...)                  -- with the Late-mode hard check of §3.3
5  runHIVMCCompileA5(module, HIVMCFromBiShengIRConfig(config))   -- exactly once
```

Step 3 is invoked **directly**, not via `buildBiShengHIRAVEToLLVMPipeline`, which would take the
`buildLowerAVEPipelines` branch first (D8/D9). This is the structural guarantee of §4.1(5).

**The wrapper's three gates, and which of them Late mode must care about** (read from
`regbase/PassPipeline.cpp:213-295`, not inferred):

```cpp
void buildBiShengHIRAVEToLLVMPipeline(OpPassManager &pm, const BiShengIRCompileMainConfig &config) {
  if (config.getCompileHost()) { hacc::buildLowerHACCToLLVMPipeline(pm, ...); return; }   // :215-218
  if (config.getEnableHIVMCompile()) { ... hivmave::buildLowerAVEPipelines(pm, opts); }  // :220-225
  if (config.getLowerToLLVM()) { buildLowerToLLVMPipeline(pm, config); }                 // :226-228
}
```

- `getCompileHost()` — an early `return` that would replace the whole tail with the HACC host
  lowering. There is **no CLI surface for it to be reached through**: `CompileHost` is a
  `BiShengIRCompileInternalStateOpt` (`Options.td:1550-1554`) the file itself documents (`:1547-1549`)
  as *"not currently surfaced as normal generated CLI/pass options"*, so `--compile-host` is an unknown
  argument rather than a flag Late could "reject" (§2.3, and the wrong-reason-pass warning at
  `:196-202`). What makes Late's direct call safe is therefore a **config-state assertion**: the entry
  asserts `config.getCompileHost() == false` — the field's own default — as an invariant of the Late
  branch, and the corresponding test asserts the config state, never an unknown-option diagnostic.
- `getEnableHIVMCompile()` — gates only the AVE builder, i.e. the branch Late **skips**. Because Late
  calls `buildLowerToLLVMPipeline` directly, this flag is **irrelevant to Late mode** and must not be
  forced by the entry; forcing it would only widen the blast radius on the ordinary route.
- `getLowerToLLVM()` — the gate that governs **the ordinary route's** reach into the tail. Measured:
  `LowerToLLVM` is a `BiShengIRCompileInternalStateOpt` (`Options.td:1556-1562`, comment at
  `:1547-1549`) with default **`true`**, explicitly documented in-file as *"still real
  options/config entries … not currently surfaced as normal generated CLI/pass options"*, and **no
  caller of `setLowerToLLVM` exists anywhere in the inspected production source** (grep over
  `bishengir/**/*.cpp`, `*.h`, `*.td`). The scope of that claim is deliberate and narrow: the
  TableGen generator *does* emit a setter — measured at the generated
  `build/tools/bishengir/bishengir/include/bishengir/Tools/bishengir-compile/CompileConfigs.cpp.inc:1402`
  (`setLowerToLLVM`, with `getLowerToLLVM` at `:1407`) — so "no setter exists" would be false, and a
  source-tree-only grep is silent about it precisely because the file is generated into `build/`. What
  is measured is that nothing in the hand-written tree **calls** one, and a generated-but-uncalled
  accessor cannot move the flag. So on this build the tail always runs, and Late's direct call is
  consistent with the ordinary route rather than a special case.

  **Precision worth keeping, because it decides what the entry must do:** the direct Late call
  bypasses all three wrapper gates, so `LowerToLLVM` is **not a dependency of Late mode** — the direct
  `buildLowerToLLVMPipeline` call would run whatever the flag said. Asserting `LowerToLLVM = true` is
  therefore an **entry invariant**, i.e. a statement about the configuration the Late entry fixes so
  that the module's lineage is unambiguous, not a condition its own call is subject to. The Late entry
  asserts the value rather than assuming it, because a default is not a contract; but a negative that
  claimed "Late did not run the tail because the flag was false" would be testing a mechanism that
  cannot exist on this path.

**The tail contains a conditional sub-block that the exact-20-pass claim depends on being dead.** The
SIMT/DPX block inside `buildLowerToLLVMPipeline` (`:269-289`: `AdaptGPUKernel`,
`HoistCallScalarToCaller`, and conditionally `DPXDivOptimization` / `SIMTInsertInitAndFinishForDebug`)
is guarded by `if (config.getPureSimt() && config.getUseDPX())`. Those four passes are **not** in the
§3.1 table precisely because Late mode rejects `--pure-simt` (§2.3). This is the loop closing: the
"exactly these 20 passes" claim is not a claim about the source alone, it is a claim about the source
*under Late mode's option rejections* — and the P2-B trace is what shows the guard was false.

**Step 5 must be outside any retry loop, and that is not how the ordinary route is written.**
Measured: in the ordinary route `runHIVMCCompileA5` is called at
`regbase/BiShengIRCompileMain.cpp:448`, which sits **inside** the retry loop's `for (int i = 0; i < tryTimes; i++)` body, `:393-456`
(`tryTimes = 2` at `:376`; the enclosing scope is `:345-466`). The Late route must not inherit that
structure: the goal requires exactly one HIVMC A5 lowering, so `runLateHIVMPipeline` calls
`runHIVMCCompileA5` **once, after** the single `buildLowerToLLVMPipeline` invocation, with no
surrounding retry. The A5 count in §3.2's boundary log is what empirically distinguishes the two
(structurally different) shapes, rather than assuming the ordinary loop's count.

**The bitcode reaches the linker through the A5 path, not through this tail.** `HIVMCMainA5.cpp`
composes the ccec invocation and passes the linked modules with `-cce-link-aicore-ll-module`
(`:181-188`); the attach-time attributes written by step 4 are what that path reads. This is the
mechanism §3.3's hard check protects.

The tail's own contents are exactly `pass-cut.md` §3's table — i.e. the goal's §5 list. **The
sync-lock finalization passes are kept**: `pass-cut.md` §3 and goal §5 both note they are not the
same mechanism as automatic sync inference, and goal §5 forbids deleting them on a name-based
argument.

**Verified registered pass-argument names (measured 2026-10-13, execution order step 1; source =
`bishengir-opt --help` on the pinned `build/bin/bishengir-opt` `0761c6e6…`, not source grep).**
These exact spellings are what the P2 implementation and the P2-B instrumentation must use:

| # | pipeline item (C++ symbol) | registered pass argument |
|---|---|---|
| 1 | AnnotationLowering | `--annotation-lowering` |
| 2 | MemRefAllocToAlloca (HIVM) | `--hivm-memref-alloc-to-alloca` |
| 3 | InsertInitAndFinishForDebug (HIVM) | `--hivm-insert-init-and-finish-for-debug` |
| 4 | MarkDisableLoad (HIVM) | `--hivm-mark-disable-load` |
| 5 | InferSyncBlockLockNumAndInitFunc | `--hivm-insert-infer-sync-block-lock-num-and-init-func` |
| 6 | MarkSyncBlockLockWithSubblock | `--hivm-mark-sync-block-lock-with-subblock` |
| 7 | BindSyncBlockLockArg | `--hivm-bind-sync-block-lock-arg` |
| 8 | LowerCreateSyncBlockLock | `--hivm-lower-create-sync-block-lock` |
| 9 | InsertFreeLockVarBeforeReturn | `--hivm-insert-free-lock-var-before-return` |
| 10 | ConvertHIVMToStandard | `--convert-hivm-to-std` |
| 11 | ConvertHIVMAVEToStandard | `--convert-hivmave-to-std` |
| 12 | FixCallUnknownLoc | `--fix-call-unknown-loc` |
| 13 | ExpandStridedMetadata | `--expand-strided-metadata` |
| 14 | ConvertHIVMAVEToAVEIntrin | `--convert-hivmave-to-ave-intrin` |
| 15 | HoistVstas | `--hoist-vstas` |
| 16 | ConvertAscendDPXToHIVMRegbaseIntrin | `--convert-ascend-dpx-to-hivmregbaseintrins` |
| 17 | DecomposeFRem | `--decompose-frem` |
| 18 | ConvertSCFToCF | `--convert-scf-to-cf` |
| 19 | LowerAffine | `--lower-affine` |
| 20 | ArithExpandOps | `--arith-expand` |
| — | ConvertHIVMToLLVM (HIVMC A5-owned, **not** in this tail) | `--convert-hivm-to-llvm` |

Three of these are **not** the obvious derived spelling and were only settled by measuring the
binary: the alloca pass is `--hivm-memref-alloc-to-alloca` (not `--alloc-to-alloca`), the disable-load
pass is `--hivm-mark-disable-load`, and the DPX pass is
`--convert-ascend-dpx-to-hivmregbaseintrins` — one word, no hyphen between `hivm` and `regbase`.
Items 5–9 are the `SyncBlockLockPipelinePhase::Finalize` family
(`hivm/Transforms/HIVMRegbasePipelines.cpp` via `addSyncBlockLockFinalizePasses`); item 4 is a
`MarkDisableLoad` on the HIVM dialect, not the `simt-` namesake
(`--simt-insert-init-and-finish-for-debug` exists and must **not** be used in Late mode). This table
also fixes the D3/D8 ordering question in `pass-cut.md` §3: the listed positions are the real
pipeline's, and P2-B does not reorder them.

### 3.2 Forbidden passes — must be measured at zero

**The instrumentation mechanism is measured, not assumed** (Lead, 2026-10-13, on the pinned
`build/bin/bishengir-opt` `0761c6e6…`): the binary exposes the full MLIR pass-instrumentation set —
`--mlir-print-ir-before-all`, `--mlir-print-ir-after-all`, `--mlir-print-ir-before=<pass-arg>`,
`--mlir-print-ir-after=<pass-arg>`, `--mlir-print-ir-after-change`, `--mlir-print-ir-module-scope`,
`--mlir-print-ir-tree-dir=<dir>`, `--mlir-pass-statistics`,
`--mlir-pass-statistics-display=<value>`, and `--mlir-disable-threading`. P2-B's evidence therefore
comes from a per-pass IR trace with the pass names of §3.1, **not** from source grep and not from a
count. `--mlir-disable-threading` is mandatory for that trace: without it the after-all order is not
deterministic.

**A trace is only evidence if coverage is demonstrated.** Six preconditions, each of which must be
shown for the run that produces the archived trace, because a pass that ran but was not instrumented
is indistinguishable from a pass that did not run:

1. **Every PassManager in the run installs the options.** The flags are registered in
   `bishengir-compile.cpp:95,97` (`bishengir::registerPassManagerCLOptions()` and
   `mlir::registerPassManagerCLOptions()`), before any pass manager is built. The A5 lowering runs
   **in-process** — `runHIVMCCompileA5` is called directly at
   `regbase/BiShengIRCompileMain.cpp:448`, defined at `hivmc/A5/HIVMCMainA5.cpp:944` — so the A5
   MLIR pipelines, including the single `ConvertHIVMToLLVM`, are inside the same trace. The ccec
   **subprocess** is not, and is covered instead by P2-C's binary/ABI checks, not by this trace.
2. **No filtering of any kind is on — and there are three distinct filters, all measured present on
   the pinned binary.** The coverage run uses the **unfiltered**
   `--mlir-print-ir-before-all` **and** `--mlir-print-ir-after-all`, and none of:
   `--mlir-print-ir-after-change` (*"only print if the IR changed"*),
   `--mlir-print-ir-after-failure` (*"only print if the pass failed"* — a failure-only filter, which
   would leave a clean trace looking as if nothing ran), or the per-pass selectors
   `--mlir-print-ir-before=<pass-arg>` / `--mlir-print-ir-after=<pass-arg>`, which silently narrow the
   trace to the named passes. Verified on `build/bin/bishengir-opt` `0761c6e6…` via `--help`. A trace
   produced with *any* of the four is **not** usable for a zero in §3.2, because "absent from the
   trace" then no longer implies "not scheduled".
3. **Complete logs are captured.** The trace is written with `--mlir-print-ir-tree-dir=<dir>` as a
   file tree under the P2 evidence directory, and the run's stdout/stderr are captured in full and
   archived — not pasted into a report and not truncated.
4. **Sentinel passes demonstrate the trace reaches both ends — but endpoints are not coverage.** The
   archived trace must contain the first tail pass (item 1 `--annotation-lowering`) and the A5-owned
   `--convert-hivm-to-llvm`; if either is absent the trace's coverage is unproven and **every** zero
   read from it is void. That is necessary and **not sufficient**: two sentinels say nothing about the
   pass managers *between* them. Coverage must be established **per pass manager** — enumerate the
   managers the run creates (the regbase tail's, the RetriablePassManager retry wrappers, and the A5
   **actually entered** — not the ones the source suggests could be built — and show each appears in
   the trace. "Actually entered" is the operative word in both directions: a manager that is
   constructed and then not run contributes headers that prove nothing, and a manager that ran without
   the options contributes nothing at all. The enumeration is therefore read **from the trace** (which
   nested-pipeline headers appear, and inside which pass), and the run's own log line for each manager
   entry is the cross-check. The converse is equally load-bearing: a retry wrapper that the Late route
   **correctly bypasses** does not appear, and its absence is **not** a coverage hole — requiring every
   manager the *source* can construct to be present would make a correct Late run unfalsifiable. The
   list of managers that must appear is the list the Late invocation actually enters. A run in which
   one of *those* was built without the registered options produces a trace that looks complete and
   silently omits everything that manager did.

5. **The trace must not be silently filtered.** See the filtering sub-section below. This is the
   precondition most likely to invalidate an otherwise clean trace.

6. **Counting is per operation scope; the assertion is compile-wide. The two must not be confused.**
   "Twenty construction entries do not imply twenty executions." The registration /
   `GEN_PASS_REGISTRATION` machinery, nested `PassManager` construction inside a pass body (the
   in-pass form at `PassPipeline.cpp:564` is exactly this), and multi-threaded cloning can each
   produce several *entries* for one *execution*. So each count is **measured** at the instrumented
   `runOnOperation`/`execute` boundary of the scope that actually runs it — the module scope, or one
   function scope — and the raw trace is archived so the attribution is checkable. But scoping the
   measurement must **not** weaken what is asserted: the goal's claims are about the whole invocation,
   so the per-scope measurements are then **aggregated across every manager the run enters**:

   | claim | measured per scope | asserted, compile-wide |
   |---|---|---|
   | `ConvertHIVMToLLVM` (§3.2 C2) | count at the module scope of the A5 pipeline that owns it | **exactly 1 in total.** "Once per scope, or once in the tail and once in A5" does **not** satisfy it — a module pass that ran in two entered managers has run twice |
   | forbidden passes (§3.2 B) | zero at each entered scope | **exactly 0 in total**, summed over all scopes. A pass absent from one manager but present in another is a failure, not a local exemption |
   | `runHIVMCCompileA5` (A4) | the function-entry boundary log | **exactly 1 in total**, the log being at the function entry precisely so both call sites are covered |
   | tail passes (§3.1 C1) | per pass **and** per operation scope — nested function passes run separately on the entry and on the VF helper, so one pass legitimately yields two executions | the §3.1 **order** across the run, with each pass's effect present in the emitted IR |

   Where a claim is about a *function* rather than a pass (§3.2's boundary logs), the count is the
   boundary log's, not the trace's — and that log too is aggregated over the whole invocation, because
   a function-entry log placed at a single call site would silently miss the other caller.

**A printed header does not prove the pass body ran.** Measured in
`third-party/llvm-project/mlir/lib/Pass/Pass.cpp`: the instrumentation callback fires at `:516-518`
(`pi->runBeforePass(pass, op)`) and only then, inside
`op->getContext()->executeAction<PassExecutionAction>(…)` at `:521-527`, is `runOnOperation`
invoked. So the trace can show a pass's before/after headers while its body never executed. The
consequence is asymmetric and must be respected:

- **Absence** of a pass's headers, under demonstrated coverage, establishes that it was **not
  scheduled** — which is the claim §3.2 actually needs for the forbidden passes.
- **Presence** proves only that the pass was scheduled, **not** that it ran. So for the §3.1 tail,
  where the claim is that a pass *did* something, presence is not evidence; the shipped IR must show
  the required **effect** (e.g. `cf` blocks present after `--convert-scf-to-cf`, no `hivm.hir.*`
  remaining after the conversions), or the run must carry a counter at the execution boundary.

**Filtering: the trace's blind spot, and why it does not bite here.** BishengIR substitutes its own
pass manager. `BiShengIRPassManager` (`include/bishengir/Pass/PassManager.h:44-97`) registers a
context action handler that implements **Mix/Aiv filtering** for "all pipeline entry points
(bishengir-compile regbase, RetriablePassManager, hivmc runPipeline, unit tests)", with a deliberate
note at `:67` that construction paths which bypass the handler "silently ran unfiltered passes". The
policy (`include/bishengir/Pass/PassExecutionPolicy.h:29-62`) is a **denylist** — `AivDisabledPasses`
— and a denylisted pass is skipped **in Aiv mode** while still being scheduled, i.e. its headers can
print. A second, context-global per-op `FilterPassesAttr` path exists as well.

But the mode is chosen from the module: `detectPipelineMode` (`PassExecutionPolicy.h:42-58`) resolves to
Aiv only when a function carries `mix_mode = "aiv"`, and **every other case — no `mix_mode` at all, a
non-`"aiv"` value, or disagreeing values — keeps the default full (Mix) mode**, in which the denylist is
not consulted. The detection happens once per manager run, lazily, on that run's first action
(`PassExecutionPolicy.h:76-99`) — which is what makes the mode a **per-manager** fact rather than a
per-action one. **Measured: the frozen Late artifact contains zero occurrences of `mix_mode`** (grep over
`v0…/vadd_late.mlir`), so the P1 Late input runs in Mix mode and no pass body is suppressed by this
policy. Three consequences:

- The §3.2 zeros are not artifacts of the denylist for this input. They must be reported **together
  with the resolved mode**, not on their own.
- This becomes an explicit Late invariant: because Late mode is AIV-only by construction, a Late
  input that carried `mix_mode = "aiv"` would activate a denylist that can drop pass bodies the
  frozen contract depends on. The preflight validator (V1) therefore **rejects a Late input that
  carries `mix_mode` at all**, rather than relying on a policy that happens to be inert today. The
  same rule covers `FilterPassesAttr`.
- **V1's input check is necessary and not sufficient, so the mode is recorded at each pass-manager
  boundary and a forbidden transition is an assertion failure.** The granularity matters and is
  measured, not assumed: the mode is *not* recomputed per pass action. `PassExecutionPolicy` detects it
  **lazily, on the first action of a run, and freezes it for the rest of that run** —
  `detectPipelineMode` is guarded by a one-shot `modeDetected` atomic and `getMode()` thereafter only
  reads `mode` (`PassExecutionPolicy.h:76-99`, whose own comment says the mode "is a property of the
  module being compiled", is "state of a single compilation/run", and is detected "on the first action
  of the run, and freezes it for the rest of the run"). The instance lives in the `BiShengIRPassManager`
  that runs the pipeline. So the mode is frozen **per manager run**, and a rewrite of `mix_mode` /
  `FilterPassesAttr` can only take effect at the **next** manager — which is precisely the window V1's
  input check cannot see.

  **The invariant is therefore about transitions, not about a per-action recomputation: record the mode
  at each manager boundary (its first action) and assert that no forbidden transition occurs.** For the
  frozen Late input every report must read **Mix**; a later transition to **Aiv** is a **failure**, not
  a curiosity, because the denylist can then drop pass bodies the frozen contract depends on.

  **Scope, stated so it is implementable rather than aspirational.** The record is taken at **every
  manager the Late entry itself invokes** — the tail, then the A5 pipeline above all — because those are
  the points the entry controls: it holds the module between calls and can inspect it before the next
  `run`. It is **not** a claim about managers nested inside a pass body (`PassPipeline.cpp:564`
  constructs one), which the entry cannot intercept; for those the guarantee rests on V1 plus the
  measured absence below, and that limit is stated rather than papered over.

  Measured today, nothing in the production tree writes either attribute (the only writers in-tree are
  `unittests/Pass/PassExecutionPolicyTest.cpp` and `unittests/Pass/FilterPassesTest.cpp`;
  `OptimizeScalarTransfers.cpp:443` only reads), so every boundary report is expected to read Mix and no
  transition is expected — but the guarantee may not rest on that absence, for the same reason the §2.2
  dead-store finding and the C9 zero-consumer finding may not rest on theirs. The policy's own header
  gives the structural reason the absence is not self-sustaining: *"the passes that would rewrite
  `mix_mode` are not in the denylist"*, so such a pass keeps executing even in Aiv mode, and freezes
  nothing about the manager after it.

**Functions are not pass identities — and one of them is not even a pipeline.** `runRegBasePipeline`,
`inferLayoutOptimization`, `inferMixedCV`, `buildLowerAVEPipelines` and `runHIVMCCompileA5` are C++
functions; the trace shows only the passes they add, so where the *invocation* is the claim, absence
must come from a driver-level observation.

**One Late routing line is not enough to prove those functions never ran.** A single
`"[INFO] … Late …"` print at the branch point only proves the branch was taken; it says nothing about
whether the ordinary pipeline, the inference functions, or the retry loop ran later, because a
function that prints nothing is invisible. The requirement (Challenger refinement, round 3) is
therefore **small boundary logs placed at the actual boundaries**, not a new framework:

| boundary | what the log records | why it is the right place |
|---|---|---|
| **ordinary regbase pipeline** | one line at the **function entry** of `runRegBasePipeline` (`regbase/BiShengIRCompileMain.cpp:345`) | the fall-through destination the Late route must never reach. It must be at the **function**, not at a call site: `runRegBasePipeline` has **two** callers — `regbase/Driver.cpp:81` and the in-pass `BiShengIRCompilePass::runOnOperation` at `regbase/PassPipeline.cpp:564` — so a log at `Driver.cpp:81` alone would leave the second entry uncovered and a Late run that reached the pass form would print nothing |
| membase ordinary pipeline | one line immediately before `runBiShengIRPipeline` (`bishengir-compile.cpp:188`) | the other fall-through destination. Kept for completeness even though an Ascend950PR Late run cannot reach it: `hasRegBaseTarget` is true, so `:185` always returns first |
| inference | one line at the **top** of `inferLayoutOptimization` (`regbase/Utility.cpp:100`) and `inferMixedCV` (`:61`) | these are plain `module.walk`s, invisible to the pass trace; only a log can show they did or did not run. Both calls are in `runRegBaseCompile` (`Driver.cpp:68`, and `:73` under `getEnableTritonKernelCompile()`), which the Late branch returns out of |
| retry | one line per iteration of the retry loop (`regbase/BiShengIRCompileMain.cpp:393-456`, `tryTimes = 2` at `:376`) | the loop re-runs the pipeline; a count, not a boolean, is what shows "exactly once" |
| A5 compile | one line at the **function entry** of `runHIVMCCompileA5` (`hivmc/A5/HIVMCMainA5.cpp:944`) | proves exactly one A5 invocation. Again the **function**, not the call site: there are two callers — the retry loop at `regbase/BiShengIRCompileMain.cpp:448` and the standalone `hivmc` tool at `hivmc/HIVMCMain.cpp:45` — so a count taken at `:448` would not be a count of A5 compiles, only of one caller's calls to them |

The existing `"[INFO] Using merged native A5 regbase pipeline\n"` (`bishengir-compile.cpp:118`) is
the pattern: cheap, greppable, already in the codebase. The archived log must then contain the Late
routing line and **no** ordinary-`runRegBasePipeline` line, **no** `runBiShengIRPipeline` line, **no**
`inferLayoutOptimization` line, **no** `inferMixedCV` line, **no** retry line, and exactly **one**
`runHIVMCCompileA5` line — each of those absences/counts being a positive assertion against a log that
*would* have shown them. Each boundary line carries its own stable prefix so that absence is
distinguishable from a truncated log; a log that ends early must not read as absence.

**The zero-calls claim for both inference functions is restored, and an intermediate draft of this
section had it wrong in the other direction.** That draft argued that because
`inferLayoutOptimization` adds no passes and only sets a config flag, "zero executions" was not a
meaningful claim. The measurement behind it was right and the conclusion was wrong, because it
described the **ordinary** route. Measured at HEAD: `inferLayoutOptimization` is called
**unconditionally** at `regbase/Driver.cpp:68` and `inferMixedCV` at `:73`, and both calls sit inside
`runRegBaseCompile` (`:63-85`) — the function P2-A branches out of as its **first statement** (§2.2
item 2). So on the Late route neither function is entered even once, which is exactly what goal
§4.1(1)`s "分流" requires, and the claim is both meaningful and assertable.

The two facts are not in tension; they are two different pieces of evidence for the same row:

- **Zero calls** is asserted from the **boundary log at each function's top** (`Utility.cpp:100`,
  `:61`) — a log line that is absent because control never arrived.
- **A pass trace cannot substitute for it.** Both functions are plain `module.walk`s that add no
  passes (`Utility.cpp:100-120`; `:61-97`), so the trace is blind to them by construction. That is
  *why* the boundary log exists, not a reason to drop the claim.

The **secondary** claim — that no layout-optimization pass is scheduled and nothing consumes the
`setEnableLayoutOptimization` flag — is worth stating as well, but as a consequence, not as the
primary assertion: it is what would still have to hold if a future refactor moved the calls above the
branch point. `inferMixedCV` likewise only sets `config.setEnableMixedCV` (`Utility.cpp:61-97`).

So the §3.2 negatives — zero executions of `buildLowerAVEPipelines`' contents (the AVE optimizer:
VectorToHIVMAVE, ArithToHIVMAVE, reduction/loop/layout transforms, AVENormalizeOps), GraphSyncSolver,
InjectSync, bufferization / PlanMemory, multibuffer, and HFusion scheduling/vectorization — are read
from a trace satisfying the four preconditions above, and `ConvertHIVMToLLVM` executes **exactly
once**, owned by HIVMC A5 (`hivmc/A5/PassPipelineA5.cpp:47-88`), with the Late front end never
calling it. Each of these counts is reported with the sentinel evidence that makes it meaningful.

### 3.3 P2-C — bitcode identity and a hard failure

`addIfExists` (D11) silently skips a missing template. In Late mode a missing **`meta_op.aiv.c310`**
bitcode is a **hard error**: the Late route's whole purpose is to produce a real A5 AIV binary, so an
input that cannot link its template must not produce an artifact that merely looks compiled. The
entry records the resolved absolute path, the SHA-256, and the resolved O0/O2 level
(`resolveTemplateBitcodeOptLevel`, `BiShengIRCompileMain.cpp:104`) into the compile log.

**There are two silent-skip sites, and they compose into a substitution vector.** Measured:

1. **Attach time** — `addBitcodeAttrsToModule` (`regbase/BiShengIRCompileMain.cpp:79`) uses
   `addIfExists`, so a missing `meta_op.aiv.c310.<O0|O2>.bc` contributes **no attribute** and no
   error.
2. **Link time** — `HIVMCMainA5.cpp:110-117`: `bitcodePathList` is initialised from
   `config.getExtraDeviceBCPaths()` (the user's extra bitcode) and the module's meta-op path is merely
   **pushed onto that same list**. If the core type is absent from `bitcodePaths`, the code emits only
   `"[WARNING]cannot find bitcode from hivm attribute for coreType:…"` (`:112-114`) and **continues to
   link**, producing a binary built from whatever extra paths were supplied. `bitcodePathList.empty()`
   is the only hard failure (`:180`) — and by construction it cannot trigger when the user supplied an
   extra bitcode, because that list is non-empty at entry. Every surviving path is then appended as
   `-cce-link-aicore-ll-module <BCPath>` (`:185-188`). So an extra `--link-aicore-bitcode` is **always
   linked**, whether or not the required template is also there; it does not need to *displace* the
   template to be wrong, it only needs to be in the link.

**A correction to an earlier draft of this section: `-Wno-override-module` is not a duplicate-symbol
suppression, and the substitution vector does not depend on it.** The earlier draft claimed the flag
suppressed a duplicate-symbol diagnostic so that a user-supplied bitcode could "define the same symbols
as the required template". Measured, that is wrong: `-Wno-override-module` maps to
`DiagGroup<"override-module">` whose only diagnostic is
`"overriding the module target triple with %0"`
(`third-party/llvm-project/clang/include/clang/Basic/DiagnosticFrontendKinds.td:47-48`) — a **clang
frontend warning about the target triple being overridden** by the command line. Its two occurrences
(`hivmc/A5/HIVMCMainA5.cpp:97`, in the ccec argument list immediately after `-cce-bitcode-is-aicore`;
and `:227`, inside the `--save-linked-ir` command echo) are both consistent with that and with nothing
else: it is a warning-suppression flag for the compile, not a link-time symbol rule. The vector is
entirely explained by items 1 and 2 — a missing template contributes no attribute, and the link
proceeds on whatever extra paths exist — with no help needed from this flag. This matters for test
design: a negative asserting that `-Wno-override-module` is absent would prove nothing about
substitution and would pass on a build that still substituted.

Therefore P2-C's hard failure must be enforced **at the entry, on the resolved path**, before either
site can be reached: resolve the required C310 AIV template, fail closed if it does not exist or its
SHA-256 does not match the recorded identity, and **reject `--link-aicore-bitcode` in Late mode**
(§2.3.1) so no substitute can enter the link.

**The negatives are four separate tests, not one combined case.** An earlier draft collapsed them into
a single "isolated template directory with the required AIV file absent and a distinct extra bitcode
supplied", which conflates three independent mechanisms and cannot tell which one fired:

| # | input state | what it isolates | expected |
|---|---|---|---|
| N1 | required `meta_op.aiv.c310.O2.bc` **absent** from `<stage>/lib`, no extra bitcode | the entry's existence check | diagnostic **naming the missing required template**; **no binary** |
| N2 | a template **present but not the recorded one** (the independently pinned pre-D012 `44e4387b…` from `build/install/lib`, or any file whose SHA-256 differs) | the entry's **SHA-256** check, as distinct from mere existence | diagnostic naming the path **and the hash mismatch**; **no binary**. This is the row that catches the two-template hazard below |
| N3 | `--link-aicore-bitcode <some other .bc>` supplied, required template present and correct | the option policy of §2.3.1 | rejection naming the option; **no binary** |
| N4 | required template **absent** *and* a distinct extra bitcode supplied — the combined case | that the entry's checks are **not** bypassed by a non-empty bitcode list | rejection; **no binary**, **without prescribing which diagnostic fires** |

N1/N2 differ in kind, not degree: existence and identity are different checks, and only N2 can fail
when a *stale but real* template sits under `<stage>/lib`. N3 is the only one that exercises the option
policy. N4 deliberately does **not** fix diagnostic precedence: because `--link-aicore-bitcode` is
rejected first (§2.3.1), N4 may legitimately report the *option* rejection rather than the missing
template, and a test that demanded the template diagnostic would be over-specified and would fail on a
correct implementation. That is exactly why N1 exists separately — N1 has no extra bitcode, so the
template check is the only one that can fire.

**"Today it produces a warning and a binary" is a hypothesis, and it is NOT_RUN until executed.**
It is stated here as the expected finding (§3.3 item 2 predicts it from the source), not as a measured
result — no unaudited probe has been run. The same applies to N4's current outcome: if the harness
cannot demonstrate **that no binary was written**, the row stays **NOT_RUN** rather than being inferred
from an exit code. Existing behaviour at both sites is to be logged as a finding, not fixed for the
non-Late route.

**Two distinct C310 templates exist in-tree, and the P1-pinned one predates the committed template
*sources*.** So "the template" is not something P2 may inherit by path. Measured 2026-10-13:

| template | path | SHA-256 (first 16) | size | mtime |
|---|---|---|---|---|
| P1-pinned, P2-tree build output | `build/lib/meta_op.aiv.c310.O2.bc` | `5e7ccae20f4e2a18…` | 31,206,796 | 13:46 |
| **pre-D012 install staging** | `build/install/lib/meta_op.aiv.c310.O2.bc` | `44e4387baea74c96…` | 31,179,496 | **2026-10-09** 15:01 |

`getLibDirFromExecutable` resolves `<real exe dir>/../lib` (`regbase/BiShengIRCompileMain.cpp:63-76`,
and `real_path` **resolves symlinks**). `build/install/bin/` holds its own Oct-9 `bishengir-compile`,
`bishengir-opt` and `hivmc`. A bundle staged from `build/install/{bin,lib}` would therefore resolve
`../lib` to the **pre-D012** template while looking entirely legitimate — a concrete, present-in-tree
instance of §6.2's "no old bitcode … or fallback template" and of the B7 check, not a hypothetical.

The pinned P2-tree template is itself **older than the committed template source**.
`bishengir/lib/Template/lib/RegBase/Vector/include/RegBase/DMAUtils.h` was written at **14:42**, and
that directory is on the ccec include path (`-I …/RegBase/Vector/include`, recorded verbatim in the
adjacent `Copy1D.aiv.c310.O2.bc.ccec.args`), while the per-file template bitcodes were produced at
**13:37:55** and the merged one at **13:46:45**; **no** `RegBase/Vector/*.bc` was regenerated after
14:00. `DMAUtils.h` is one of the files the D012 commit changes (79 changed lines). Whether the 14:42
write changed bytes or rewrote identical content is **not decidable from the filesystem**, and a
dry-run `ninja -n` cannot settle it — it stops at "Re-running CMake" because `build.ninja` is already
stale. **Consequence for P2-C: rebuild the template from the P2 tree and record the resulting SHA-256
as P2's own identity; do not assume `5e7ccae2…` corresponds to the committed template sources.**
Goal §6.2's requirement that the *new kernel's* chain consumed the *correct* bitcode is unsatisfiable
against an unverified template.

Goal §6.1: the P1-pinned `bishengir-opt` SHA `0761c6e6…` is a **historical identity, not P2's
expected SHA**. P2 builds a new `bishengir-compile` and records its own identity. The build must not
overwrite the P1-pinned `build/bin/bishengir-opt` or `build/lib/meta_op.aiv.c310.O2.bc` in a way that
breaks the P1 snapshot's re-verifiability (`snapshot/INTEGRITY.json` I5 pins both). D031's compiler pin
is never edited to fit a P2 build.

**The P2 build makes `bishengir-opt` stale — the earlier "any ninja target relinks it" phrasing was
too broad and is corrected here.** The P1 digest pins
`/home/s00653124/workspace/AscendNPU-IR-1/build/bin/bishengir-opt` = `0761c6e6…` **by path**. P2 adds
code to `BiShengIRRegBaseCompileLib`, which both `bishengir-opt` and `bishengir-compile` link
(§2.4, measured). `bishengir-opt` and `bishengir-compile` are **separate Ninja targets with separate
link edges**: building only the compile target leaves `build/bin/bishengir-opt` untouched at
`0761c6e6…`, and building only the opt target leaves the compile binary untouched. What is true is
that the *shared library* is a common input, so **once both are built**, both change. The plan
therefore names the targets explicitly and builds **both** — the Late route needs a new
`bishengir-compile`, and the validator's standalone-testability (§2.4) needs a `bishengir-opt` that
registers the new pass. After that, `snapshot/run_p1v2.sh` step 1 (integrity I5) would fail on the
compiler row — not because P1 drifted, but because the build tree moved. Neither the snapshot nor its
digests may be edited to accommodate that.

**Plan (the "构建/暂存方案" §6.1 asks for), executed before any P2 build:**

1. **Preserved** (done, 2026-10-13, before any P2 compile):
   `…/p2-20261013-late-pipeline/lead/preserved-p1-compiler/` holds byte copies of
   `bishengir-opt` (`0761c6e6…`), `meta_op.aiv.c310.O2.bc` (`5e7ccae2…`) and the **pre-P2**
   `bishengir-compile` (`c6838fc0…`, kept so a behaviour change can be attributed to P2 rather than
   assumed), with `SHA256SUMS.txt`. The first two hashes were re-verified against the P1 digest after
   copying — they match exactly.
2. **Build** in the existing incremental directory (goal.md §6: keep the incremental CMake/Ninja
   path; no fresh worktree, no second 3 GB build tree).
3. **Stage a bin/lib bundle, not bare binaries.** This is not tidiness — it is forced by how the
   template is found. `getLibDirFromExecutable` (`regbase/BiShengIRCompileMain.cpp:63-76`) takes the
   **executable's own path**, passes it through `llvm::sys::fs::real_path` (**which resolves
   symlinks**), strips the filename, appends `../lib`, and canonicalizes. A staged `bishengir-compile`
   dropped into a bare directory therefore resolves its template relative to *that* directory and
   finds nothing; and a **symlink** to the built binary resolves back to the build tree, silently
   using `build/lib` — the opposite of what staging is for. (The path is taken from
   `getExecutablePath(argv[0], …)`, i.e. `/proc/self/exe`, so this cannot be dodged from the command
   line either.) The stage therefore looks like:

   ```
   <stage>/bin/bishengir-compile          # a real copy, not a symlink
   <stage>/lib/meta_op.aiv.c310.<O0|O2>.bc  # the selected template, hash-recorded
   ```

   So `<stage>/bin/bishengir-compile` resolves its lib dir to `<stage>/lib` — a path P2 controls, whose
   template SHA-256 the compile log records, and which no later rebuild of the build tree can move.
   All P2 tests and the Verifier use the `<stage>/bin/...` absolute path. The same stage holds the new
   `bishengir-opt` (it does not need a template, but it must not be confused with the pinned one).
   The `/proc/self/exe` detail is verified, not assumed: `getExecutablePath`
   (`lib/Tools/Utils/Utils.cpp:360-375`) is `llvm::sys::fs::getMainExecutable` followed by
   `real_path`, so the path is the real binary's, twice over.
4. **Restore** the pinned `build/bin/bishengir-opt` from the preserved copy and re-run
   `sha256sum -c` on the out-of-band digest, expecting **47/47 OK** again. Restoring is what keeps the
   P1 entry re-verifiable; it is a deliberate, recorded act, not drift, and it is why the copy in
   step 1 exists. Two consequences to record honestly:
   - `build/bin/bishengir-opt` then does **not** match the current source tree until the next build.
     That is the trade the goal's §6.1 asks us to make, and it is stated in `docs/status.md` so no
     later reader mistakes the stale binary for a bug.
   - **The restore also makes Ninja consider the stale output current — now demonstrated, not
     feared.** The Challenger executed the counterexample in a container scratch tree. Exact
     mechanism, as reported: a temporary `build.ninja` containing `rule copy` with
     `command = cp input output`, and `build output: copy input`; `input` contains P2 bytes;
     `ninja output` builds it; `shutil.copyfile` of the preserved P1 bytes then overwrites `output`;
     `ninja -n output` reports **"no work to do"** while `output` still holds P1 bytes. The root
     cause is that Ninja tracks dependency timestamps and build metadata, not output content
     identity. A host probe further showed that making `output`'s mtime **ten seconds older than
     `input`** causes `ninja -n` to schedule a rebuild. Both probes ran in isolated temporary
     directories and modified no real build product — they are the Challenger's executed evidence,
     not a Verifier acceptance row.

   **Therefore step 4 is a *guarded* restore, and the guards are mandatory.** A plain restore is
   unsafe and must not be performed. The restore is allowable inside the existing incremental build
   — which is what goal.md §6 wants — only with all four guards, each producing a recorded artifact:

   | # | guard | how |
   |---|---|---|
   | G1 | explicitly rebuild **both** `bishengir-opt` and `bishengir-compile` before every P2 staging | never rely on Ninja noticing; the two are separate targets with separate link edges |
   | G2 | make the restored output **older than its dependencies** (or apply another proven invalidation) so Ninja cannot treat it as current | e.g. backdate the restored file rather than only copying bytes |
   | G3 | verify the staged bundle's hashes **and** that the staged `bishengir-opt` registers the new pass | `sha256sum` on the staged files + `bishengir-opt --help` listing the new argument |
   | G4 | freeze a complete staged `bin/`+`lib/` bundle and pin the tests to it | step 3's layout |

   If G1–G4 cannot be enforced for a given build, the fallback is a **separate build directory** —
   cleaner, but explicitly not technically required. Note that the P2 validator, the negatives and
   the D012 tests **must** run against the **newly built, staged** `bishengir-opt`; the restore does
   not prevent that, provided that opt was built and copied **first**, which G1 and G3 enforce.
5. If a P2 build is ever re-run, steps 3–4 repeat, guards included.

`meta_op.aiv.c310.O2.bc` is not touched by a driver-only change, but it is preserved anyway because it
is cheap insurance and because P2-C must record the bitcode identity the new kernel actually links
against.

## 4. Explicit non-goals

- No new target kind; no new runtime; no new generic optimizer; no multibuffer support.
- No change to the normal RegBase path: the option defaults to `hir`, and a regression compile of an
  ordinary SIMD input must produce the same pass sequence as before (§4.1(9)).
- No P3/P4/P5/P6 work: no ACL launch, no TileLang backend selection, no DeviceCodeGen.
- The Late input is the **frozen P1 artifact** (`d1023ac1…`) and, for negatives, derivatives of it.
  Golden TIR/JSON and the P1 v1/v2 oracles are immutable and are never regenerated to match output.

## 5. What P2 must prove (measurement plan)

| claim | how it is measured | where it is recorded |
|---|---|---|
| correct Late routing | the entry runs and an ordinary input is unaffected | `docs/p2-pass-trace.md` |
| forbidden passes = 0, required order, `ConvertHIVMToLLVM` = 1 | MLIR pass instrumentation on the real input, with execution counts by pass identity and scope | `docs/p2-pass-trace.md` |
| contract preservation | AIV grid=64, per-block GM indexing, 8192 f32/block, UB byte offsets 0/65536/131072, DMA payload 32768, dynamic UB 196608, cache4, 128×64 vector work, pb32, the four set/wait ops, the entry-terminal `PIPE_ALL` — each asserted on the IR at each stage | `docs/p2-pass-trace.md` |
| binary authenticity | a real new A5 AIV kernel binary, with reproducible command, exit code, SHA-256, symbols and target identity | `docs/p2-binary-abi.md` |
| ABI | three GM pointer params, no extra device-entry params, kernel symbol vs FunctionInfo, AIV core type / Ascend950PR target, blockIdx and GM offsets, pb32 staying b32, DMA ranges, sync not deleted or reordered | `docs/p2-binary-abi.md` |
| independence | the Verifier's suite (`…/p2-20261013-late-pipeline/verify/`), sealed before implementation | `docs/p2-verification.md` |

The entry-terminal-barrier check must be **entry-scoped**, not module-wide: the artifact declares a
second function (`@main_kernel_simd_vf_0`), and a `PIPE_ALL` moved into that helper must not satisfy
it. This is the exact defect the P1 contract gate was rewritten for, and goal §5 requires P2 to
inherit that semantic.

## 6. Open items this document does NOT settle

- ~~Whether the frozen artifact already carries a complete spec~~ — **read, not assumed:** the frozen
  artifact **does** declare a module-level `dlti.target_system_spec` for `"NPU"` with 14 entries
  (`AI_CORE_COUNT` 28, `CUBE_CORE_COUNT` 28, `VECTOR_CORE_COUNT` 56, `UB_SIZE` 2031616, `L1_SIZE`
  4194304, `L0A/L0B_SIZE` 524288, `L0C_SIZE` 2097152, the three alignment entries, the two D-cache
  entries, and `ARCH` = `"dav-c310"`), plus `hacc.target = #hacc.target<"Ascend950PR_9579">` and
  `hivm.module_core_type<AIV>`. So the **expected** path for the frozen input is **case 2 (reuse)**,
  and case 1 (append) is the path a *future* input without a spec would take. **Still open, and it is
  a real risk:** whether skipping `AppendDeviceSpec` is safe depends on whether that pass has effects
  *beyond* writing the spec. **Read (not assumed) — it does not:**
  `AppendDeviceSpec::runOnOperation` (`bishengir/lib/Dialect/HACC/Transforms/AppendDeviceSpec.cpp:116-162`)
  performs exactly three things: `hacc::utils::setTargetDevice(moduleOp, finalTarget)`,
  `hacc::utils::setNPUTargetSpec(moduleOp, spec)` (unconditional overwrite, preceded by the
  "Overwriting the device spec…" warning), and — **membase architectures only** — the
  `HIVMCVersion`/`HIVMCCompatiblePrint` attributes, which a regbase A5 module never reaches. For the
  frozen artifact both attributes already exist and match the CLI target, so case 2 is a pure
  no-op-equivalent. The confirming experiment is still required as evidence rather than as belief:
  run the tail with and without the pass on the frozen input and compare the resulting IR and the
  final binary hash. If they differ, case 2 is not safe and the entry must run the pass.
- The minimal tail: goal §5 explicitly does **not** require minimizing the pass count in the first
  version, so the conservative tail is used unchanged.
- Exact `hivmcConfig` field set for A5 — copied from `HIVMCFromBiShengIRConfig` as the normal path
  does, with the same manual A5 defaults (`autoVectorizeV2(false)`, `deterministicComputing(true)`,
  `useDPX(true)`, `onlyRunHIVMPipeline(false)`), because diverging from them would be an unreviewed
  architecture change.
