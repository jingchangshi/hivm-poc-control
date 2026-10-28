# First interactive turn to Claude Code

> **历史文件（Historical — 2026-10-12 时代）。** 本提示词面向 Claude Code + Magpie 模型别名的
> 隐式团队工作流，该工作流已不再使用。当前入口见仓库根 `README.md` 与 `docs/current-state.md`；
> 当前 Agent 运行时是 DeepSeek Harness + Agent Teams，机器门禁是 AgentGates MCP。
> 文中当前任务（P0）**早已关闭**，不得据此重跑 P0。原文保留，仅作记录。

Read CLAUDE.md and docs/goal.md, docs/environment.md, docs/acceptance.md, docs/status.md.
You are the Sonnet-routed GPT-6.1 Sol lead. Use a real Claude Code implicit Agent Team, not anonymous subagents.
Create the minimum named teammates through Agent(name=..., subagent_type=..., model=..., run_in_background=true):
- verify: `verifier-haiku`, `haiku`, investigate and independently execute P0 checks.
- review: `challenger-opus`, `opus`, independently inspect local AscendNPU-IR late stage and contract.
Spawn implementor `impl` using `implementor-sonnet`, `sonnet` only after tests and P0 acceptance plan are agreed, then keep it assigned to bounded changes.

Current task: P0 ONLY. Inspect existing staged user changes; safely reconcile `tests/tests/golden` with canonical `tests/golden` WITHOUT modifying the Golden byte content, re-running generator, resetting index, or touching unrelated user work. Record SHA, manifest metadata and actual paths. Confirm the JSON is after `_prepare_device_codegen_mod` and the generator is deterministic from project root. Inspect local compiler source for the actual A5 Late IR insertion point, prior pass assumptions, runHIVMCCompileA5 call graph, and runtime ABI. Produce docs/late-ir-contract.md, docs/pass-cut.md, docs/p0-verification.md, and independent tests BEFORE implementation. Do not implement P1 or P2. Ask for user review if local code contradicts the high-level plan or changes architecture.

Before any code changes: create isolated feature branches in existing repo directories, preserving already staged TileLang changes; do not create a new NPU-IR worktree or invalidate its build dir.
At completion: verifier reports run commands/exit codes, challenger returns APPROVE/REVISE/BLOCK, lead updates docs/status.md and tells user the short P0 summary and next action. Check Magpie served model logs if accessible; otherwise explicitly request manual confirmation.

Start now. Do not ask the user to repeat already known environment facts.
