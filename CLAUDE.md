# TileLang HIVM backend POC - permanent control rules

> **身份说明（2026-10-28）。** 本文件最初是 Claude Code 会话的常驻规则。
> **§ Team and models 已历史化**：Claude Code 隐式团队、Sonnet/Haiku/Opus 经 Magpie 的别名路由、
> `.claude/agents/*.md` 子代理名与“最多两名执行中队友”的调度方式**均已不再使用**；
> 当前运行时是 DeepSeek Harness + Agent Teams；**角色划分见 `README.md`，
> 模型路由以运行时（Harness）会话配置为准，本仓不记录也不指定任何模型别名**；
> served-model 审计历史见 `docs/current-state.md` §2。
> **§ Nonnegotiable engineering controls 与 § Reproducibility 仍然有效**（除其中“写 status.md”
> 与证据路径的表述外，它们仍是本项目的工程约束）。当前入口见 `README.md` 与 `docs/current-state.md`。

Read docs/goal.md, docs/environment.md, docs/acceptance.md, and docs/status.md before doing work.
Use ONLY evidence from current local repositories for exact compiler pass order, dialect names, CLI flags, and CMake targets.

## Team and models

- Lead: Sonnet alias through Magpie (GPT-6.1 Sol), coordinator and gatekeeper.
- implementor: `.claude/agents/implementor-sonnet.md`, Sonnet alias through Magpie.
- verifier: `.claude/agents/verifier-haiku.md`, Haiku alias through Magpie.
- challenger: `.claude/agents/challenger-opus.md`, Opus alias through Magpie.
- Claude Code 2.1.296 uses an implicit team. Use Agent with a name to start a teammate; do not seek TeamCreate or manually author ~/.claude/teams config.
- Verify real served model via Magpie request logs. Do not infer it from an agent-type label.
- Keep at most two executing teammates plus the lead active in routine implementation. Wake the challenger only at architecture gates or for focused hard failures.

## Nonnegotiable engineering controls

- Golden TIR/JSON are immutable accepted input fixtures. Never silently regenerate or modify them to match output.
- Define tests and expected results independently of production implementation, BEFORE implementation.
- Preserve ABI, explicit synchronization, UB addresses, buffer spacing, grid size, and launch metadata.
- Avoid duplicating lowering inside bishengir-compile and in-process HIVMC. Verify the exact stage cut from local code and IR dumps.
- Start with single-tile F32 VADD only. No new generic optimizer, redundant runtime, new target kind without evidence, or premature multibuffer support.
- `tilelang-mlir-ascend` is read-only reference code. Do not copy APIs blindly.
- Implementor owns production code. Verifier owns isolated test scripts and execution evidence. Challenger is read-only reviewer.
- Do not edit test oracle or weaken assertions after failure. Every nonexecuted test is NOT_RUN, not PASS.
- Do not run multiple builds against one build directory simultaneously; serialize physical NPU tests.
- Never use `git reset --hard`, `git clean`, forced pushes, environment rebuilds, or edits to unrelated containers without explicit user consent.
- Do not push branches or change remote repository configurations without explicit user consent.
- Never leak gateway credentials, host environment secrets, or account tokens in artifacts.

## Reproducibility

Write `docs/status.md` after every phase with stage, outcome, commits, tests, log paths, blockers, next action.
Write per-phase evidence under `/home/s00653124/workspace/hivm-poc-evidence/` (host and Docker share /home).
A finished compiler or generated ELF does not prove numerical correctness; a prebuilt-ELF runtime run does not prove CodeGen E2E.
At most two targeted repair attempts for one root cause before independent Challenger review.
If an assumption fails, perform a small discriminating experiment, record observations, and escalate only architecture-changing decisions.
