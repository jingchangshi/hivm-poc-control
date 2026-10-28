# HIVM POC 控制仓 — 当前入口

本目录是 TileLang / AscendNPU-IR 两仓、真机 NPU 编译器 POC 的持久控制面。
**权威历史记录**是 `docs/status.md` 与 `docs/decisions.md`（只追加、不覆盖），
其中保留了 Claude Code + Magpie 时代（P0–P4）的完整过程与原始字节。

## 1. 当前状态

以 [docs/current-state.md](docs/current-state.md) 为准；P5 部分见该文件 §13。

**该文件 §13 记录的 P5-0..P5-3 完成属历史结论，须现场复核后才能引用**：
其依据是当时那轮编译与测试日志（`evidence/p5/p5-2-p5-3/`），
不是本次整理时的重新执行结果。整理者未重跑任何编译或门禁，
因此不得据本 README 断言 P5 已通过。P5-4 未完成，P6/P7 未授权。

## 2. 当前工具链（2026-10-28 现场形态，属时间快照，不是固定配置）

- **Agent 运行时**：DeepSeek Harness + Agent Teams（Lead / implementer / validator / reviewer / challenger 等角色），
  不再使用 Claude Code 隐式团队，也不再依赖 Magpie 模型别名。
- **机器门禁**：AgentGates MCP（server 名 `agentgates`）。流程是
  `list_workspaces` → `list_gates` → `doctor` → `submit` → `status`/`wait` → `verify_requirements`。
  `verify_requirements` 才回答“某个 run id 是否满足当前源码状态的必需门禁”；
  一次执行变成 FINISHED **不等于**其门禁 PASS。
  - workspace `tilelang`：adapter `tilelang-hivm`（`tilelang-hivm-build`、`tilelang-hivm-regression`）、
    adapter `tilelang-hivm-npu`（`tilelang-hivm-npu-sanity`）
  - workspace `ascendnpu-ir`：adapter `ascendnpu-ir`（`ascend-build`、`ascend-regression`）
- **容器 / 环境**：现场核验，不要写死。用 `docker ps` 与 gate `doctor` 确认当前运行的容器、
  CANN/Conda 路径与 `HIVM_COMPILER` 指向；`docs/environment.md` 里的容器名只是历史快照。

## 3. 仓库（HEAD 为时间快照，执行时以现场 `git rev-parse HEAD` 为准）

| 仓库 | 路径 | 角色 |
|---|---|---|
| TileLang | `/home/s00653124/workspace/tilelang` | 生产源码（P5 唯一可写仓） |
| AscendNPU-IR | `/home/s00653124/workspace/AscendNPU-IR-1` | P2 Late 流水线所在，本轮只读 |
| 参考实现 | `/home/s00653124/workspace/tilelang-mlir-ascend` | 只读参考，不得照搬 |
| 证据树 | `/home/s00653124/workspace/hivm-poc-evidence` | 只本地保存，不发布 |

## 4. 文档地图

| 文件 | 定位 |
|---|---|
| `docs/current-state.md` | **当前入口**：当代结论与门禁状态 |
| `docs/goal.md` | 目标、阶段划分与硬门禁（仍有效） |
| `docs/acceptance.md` | 验收矩阵（仍有效） |
| `docs/status.md`、`docs/decisions.md` | 权威历史，**只追加** |
| `docs/late-ir-contract.md`、`docs/p2-*.md`、`docs/p3-*.md`、`docs/p4-goal-1016.md` | 各阶段已封存的契约与设计 |
| `docs/p0-verification*.md`、`docs/p0-open-decisions.zh.md` | P0 历史核验记录 |
| `tasks/P5-FINAL-CODEGEN.md` | 上一轮任务单（未跟踪文件，本次保留）；§8 已换为当前运行时说明 |
| `tasks/P5-TILELANG-HIVM.md` | P5 原始任务单（旧 Harness 时代）；**技术要求仍是设计依据，但其完成状态与 Gate 状态须现场核验** |

## 5. 历史文件（可直接读到，但**不要照抄执行**）

- `CLAUDE.md` — Claude Code 与 Magpie 时代的团队/模型规则。工程约束仍适用；
  子代理名、模型别名、“implicit team”机制已废弃。
- `docs/environment.md` — 2026-10-12 环境快照。其中 `s00653124_tilelang` 容器现已 exited。
- `prompts/01-start.md` … `prompts/04-escalate.md` — Claude Code 会话提示词。
- `.claude/agents/*.md` — Claude Code 子代理定义（`implementor-sonnet`/`verifier-haiku`/`challenger-opus`）。
- `scripts/in-container.sh` — `docker exec` 包装器，只转发 `HOME` 与 `HIVM_CONDA_ENV`。
  其 sha256 已被历史证据引用，**不要改动内容**。
- `scripts/check-golden-readonly.sh` — P0 只读校验；Golden 现已位于
  `tests/golden/hivm/vadd_f32_single_tile/`（旧 `tests/tests/golden/...` 路径已不存在）。
- `p3/`、`lead/`、`evidence/` — P3 harness 源码、Lead 临时物与已封存证据，均按原样保留。

## 6. 不变约束

- 冻结 Golden（`golden_codegen_input.json` `576fa7a3…`、`.tir` `4194733b…`）不得修改或重生成。
- 不执行 `git reset --hard`、`git clean`、`git stash` 覆盖用户改动；不删历史证据。
- 未获授权不 push、不改远端、不清理容器、不重置 NPU。
- 未获授权不改 AscendNPU-IR；P6/P7 未授权前不得启动。
## 7. 历史原文（原文逐字保留，仅作记录，不要照此执行）

> 下面是本 README 在 Claude Code + Magpie 时代的原始内容。其中"在宿主机跑 Claude Code、
> 按 sonnet/haiku/opus 别名核对 Magpie 路由、P0 之前先跑 check-golden-readonly"等步骤
> 属于该轮次的工作流；当前入口以本文件 §1–§6 为准。

```markdown
# HIVM POC Agent Teams kit

This directory is the persistent control plane for a two-repository, real-NPU compiler POC.
Run Claude Code on the host; run all compiler builds and NPU tests in the existing Docker container.

## Usage

1. Unpack this directory as `~/workspace/hivm-poc-control`.
2. Keep your existing Magpie endpoint and credential exports; do not place secrets in this directory.
3. Verify tier mapping: sonnet = GPT-6.1 Sol, haiku = Haiku 5.5, opus = Opus 5.5, using the real IDs that passed your Magpie tests.
4. Run `bash scripts/check-golden-readonly.sh` before any file migration.
5. Start Claude Code with the commands shown in `prompts/01-start.md`.
6. Give the lead `docs/goal.md` and the task in `prompts/01-start.md`.
7. Run P0 separately from implementation. Use `prompts/02-implement.md` only after P0 and the architecture gate pass.
8. On a new session, use `prompts/03-resume.md`.

Do not directly run generators, compiler installers, or destructive git commands before P0 validates the existing staged Golden artifacts.
```
