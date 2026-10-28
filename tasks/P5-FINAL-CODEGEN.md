# P5-FINAL-A — TileLang HIVM CodeGen 接入与语义验收

> **本文件是本轮（2026-10-28）的任务单。** 技术验收内容全部有效，
> 但 **§8 的编排与权限段落是上一轮 PiEngineeringHarness 时代的写法**，已由下方
> “§8（当前运行时）”取代；旧段落在“§8 历史注记”里以**摘要**形式保留（非逐字原文）。
> §9 的交付与停止条件、以及“仅 READY_FOR_P5_GATE”的边界未改。
> 本文件此前未被跟踪（`git status` 显示 `??`），本轮整理**不删除**其任何技术要求。
>
> 当前现场（时间快照，执行时须以 `git rev-parse HEAD`、`docker ps` 和 AgentGates `doctor` 复核）：
> - TileLang `6fe6f091`（`feat/hivm-backend-poc`，工作区有未提交的 5 个文件改动）
> - AscendNPU-IR `bfefe180`（clean）、控制仓 `a4358b23`
> - Agent 运行时为 **DeepSeek Harness + Agent Teams**；机器门禁为 **AgentGates MCP**
>   （workspace `tilelang` 的 adapter `tilelang-hivm` / `tilelang-hivm-npu`）。

你是 Agent Team 的 Lead。

使用多 Agent 工作流，完成 P5 源码终审和必要的最小修复。

本轮授权修改 TileLang。不得修改 AscendNPU-IR，不得启动 P6 实机 E2E。

## 1. 恢复现场

生产仓库：`/home/s00653124/workspace/tilelang`

只读参考：

- `/home/s00653124/workspace/AscendNPU-IR-1`
- `/home/s00653124/workspace/hivm-poc-control`

预期 HEAD：

- TileLang `6fe6f09182c854e680a5488f4d8461fe5d5a76a6`
- AscendNPU-IR `bfefe18011152cca671e7e6ddf03793afbacaa2a`
- 控制仓 `a4358b23472be409cea9742a2f9ae2a90bf1af32`

读取：

- `docs/current-state.md` §13。
- `tasks/P5-TILELANG-HIVM.md`。
- `evidence/p5/p5-2-p5-3/README.md`。
- 两份 P5 CodeGen 测试日志。
- P1/P2 已批准的 Late IR 契约。

核对 Git HEAD、工作区和当前 Adapter Gate。保留所有既有改动。

控制仓曾存在用户预先留下的 `docs/status.md` 修改。不得 stash、覆盖或代替用户提交该修改。

## 2. 已有成果

当前代码已实现：

- `src/ascend/codegen/hivm_codegen.cc`：真实 TIR Visitor。
- `src/ascend/codegen/rt_mod_ascend_hivm.cc`：独立 GENERATED 编译入口。
- `tilelang/contrib/hivm.py`：P2 BiSheng 编译回调。
- `hivm-generated-aibin`：独立的 GENERATED Module 格式。
- `testing/ascend/codegen/`：21 项测试。

固定 Golden 的新生成 Binary 已与 P3 Binary 逐字节一致。

不要重写 emitter，不重复构建 P2，也不要修改冻结 Golden。

## 3. 必修项 R1：正式 Backend 绑定

当前 `tilelang/ascend/codegen.py` 中 `HIVM_CODEGEN.build` 仍指向：

`target.build.tilelang_hivm_prebuilt`

而 P5 新增的真正入口为：

`target.build.tilelang_hivm`

修复此处，使正常 `hivm` Backend 默认选择 GENERATED 编译。

保留 P4 PREBUILT Global Function，供明确的 TEST_ONLY 测试直接调用。不得让正常 HIVM Backend 自动选择它。

核对 `tilelang/ascend/backend.py` 的 `HIVM_BACKEND`、Backend Module callbacks、`create_backend_context` 和 `device_codegen` 调用关系。

应满足：

1. `target="hivm"` 正常解析到 `HIVM_BACKEND`。
2. `compile_device=True` 使用 GENERATED 编译。
3. `compile_device=False` 返回与现有 `DeviceCodegen` 契约兼容的 Source-only Module。
4. 没有注册编译 Hook 时，编译模式应明确失败。
5. 不因编译失败退回 PREBUILT。
6. 旧 AscendC/PTO 路由不变。

特别注意：

`target.build.tilelang_hivm_source_only` 当前返回 `ffi::String`，但 `DeviceCodegen.lower` 和 `lower_with_context` 的相关路径需要 Module，并会访问 `inspect_source()`。

**禁止直接将该 String 入口绑定为 `build_without_compile`，然后假定正常 `lower()` 可以工作。**

先检查现有 `build_without_compile` 返回契约，再提供最小兼容的 Source-only Module 入口。

Source-only Module 应让调用者获取真实生成的 Late MLIR，不应加载冻结 P3 Binary。

## 4. 必修项 R2：编译 Hook 和 Target 一致性

审查：

`tilelang/contrib/hivm.py`

当前 `_hook(mlir, target)` 没有使用 `target`，而编译目标由 `HIVM_TARGET` 或默认值决定。

验证并修复以下情况：

- Target 选择 HIVM AIV，但编译命令选择另一目标。
- `HIVM_COMPILER` 指向非 P2 Late 编译器。
- 编译进程返回错误。
- 编译成功但没有生成对象文件。
- 未配置 Hook 时错误回退到 PREBUILT。

本轮仅支持 Ascend950PR_9579。

明确规定 Target、编译命令中的设备目标和输出 ELF 目标必须一致。

避免构建过程中通过环境变量隐式覆盖已验证的设备目标。

不需要添加跨代兼容功能。

## 5. R3：独立 TIR 语义变异测试

Validator 先设计反例。

检查 `hivm_codegen.cc`，尤其：

- `EmitGmSubview`
- `ElemBase`
- `EmitVadd`
- `EmitVsts`
- `EmitVfLoop`
- `EmitFlag`

优先构造以下有效 TIR 变异：

A. 将 GM 的 blockIdx 变量替换为另一个变量，但保留乘法常量 8192。

B. 保留 UB 基址，修改索引中的循环变量或步长。

C. 改变 VADD 的实际输入数据依赖。

D. 改变 VSTS 的写入值或 mask。

E. 调整 DMA 参数或同步顺序。

F. 改变 Store cache mode。

每个反例均应先确认变异后的 TIR 可以被正确构造和解析。

对于受支持的变异，生成的 MLIR 必须反映真实语义。

对于不受支持的变异，CodeGen 必须明确拒绝。

严禁：

- 输入已改变，输出却静默保持旧 Golden 语义。
- 因构造了非法 TIR 就宣称编译器验证成功。
- 把 Parser 拒绝冒充 CodeGen 语义检查。
- 用固定输出文本或 Golden SHA 代替真正的数据流验证。

Challenger 应检查每条反例是否真正改变了被测的语义。

发现确实被错误接受的反例后，Implementer 才做最小修复。不要为了支持新的算子变体而扩大本轮子集。

## 6. R4：GENERATED 产物来源

检查新 Module 的 `InspectSource`。

当前 GENERATED 路径主要保留描述性 provenance label，而非完整生成 MLIR。

要求：

- Source-only 能提供真实生成的 Late MLIR。
- GENERATED Binary 与本次输入 MLIR 具有可追溯的身份关系。
- PREBUILT 来源标签不出现在 GENERATED Module 中。
- 序列化与恢复保持 VECTOR_CORE Loader 策略。
- 序列化恢复不能重新声称 GENERATED Binary 来自冻结 P3。
- 保持 `cache_reusable=False`。
- 不开启内容寻址缓存开发。

不要求更改现有 Binary 格式的大框架。

## 7. 独立验证

先复用已有测试：

`testing/ascend/codegen/`

新增正式 Backend 路由、Source-only Module、Target 一致性和 TIR 变异测试。

将以下测试分开报告：

- 不依赖 P2 编译器的 Source-only 测试。
- 依赖真实 P2 编译器的编译测试。
- 现有 AscendC/PTO/PREBUILT 回归。

编译验收时必须配置真实 `HIVM_COMPILER`。

如果编译器未配置或无法执行，状态为 BLOCKED/NOT_RUN，不得以 4 SKIPPED 宣称 P5 PASS。

执行：

1. TileLang 增量构建。
2. MLIR parse/verify。
3. P2 LateInputVerifier。
4. 新 MLIR 经 P2 编译。
5. ELF、ABI 和 Binary SHA 校验。
6. TIR 变异测试。
7. AscendC/PTO/HIVM Backend 路由回归。
8. 必要的 Module 序列化回归。

对现存 45 项 BiSheng 失败，仅核对已保存的干净基线及受影响范围。如果没有新的差异，不修复无关测试。

## 8. Agent 编排与权限（当前运行时 — 2026-10-28）

本轮改由 **DeepSeek Harness + Agent Teams** 执行，机器门禁由 **AgentGates MCP** 提供。
PiEngineeringHarness 与 `AGENTS.md` 已不在本轮执行路径上。

- Explorer/Architect/Reviewer/Challenger：只读，分析调用链、语义与反例。
- Implementer：**唯一**生产源码 Writer。
- Validator：独立测试与独立执行证据。
- Lead：编排、记录 receipt、提交中文报告。

门禁要求：

- 现场核验优先。`docker ps`、`git rev-parse HEAD`、`git status` 的结果才是执行身份；
  容器名与历史快照中的环境**不得写死**。
- 机器门禁走 AgentGates：`list_workspaces` → `list_gates` → `doctor` → `submit` → `status`/`wait`。
- 一次执行 **FINISHED 不等于门禁 PASS**；只有 `verify_requirements` 能回答
  “该 run id 是否满足当前源码状态的必需门禁”。
- 本任务不交付新的 gate。现有 `tilelang-hivm`（build / regression）与
  `tilelang-hivm-npu`（npu-sanity）只覆盖构建、回归与设备基线，
  **不构成 P5 专项验收，也不能替代本轮 §3–§7 的 CodeGen 验收**。

不自动修改模型配置、清理容器、重置设备或推送远端。
不删除历史证据；不 `git reset --hard` / `git clean`；不覆盖用户未提交的改动。

### §8 历史注记（PiEngineeringHarness 时代，摘要，非逐字原文）

> 遵守 PiEngineeringHarness 最新 `AGENTS.md`。
> Explorer/Architect 只读；Implementer 唯一 Writer；Validator 独立测试；Challenger 最终审查；Lead 汇报。
> 该轮有用户批准的 `sandboxDegraded=true` 历史记录；若无法运行 bwrap，须确认 Harness 已获 Owner
> 明确授权 `PI_ENGINEERING_ALLOW_UNSANDBOXED_BASH=1`；允许降级时所有 Writer 记录须保留
> `sandboxDegraded=true`，不得把降级执行表述为沙箱隔离成功，也不得静默开启该变量。
>
> **这些是历史执行器要求，不再是本轮的门禁条件。**

## 9. 交付与停止条件

交付：

- 正式 Backend 路由的最小代码修复。
- Source-only Module 的实际调用证据。
- Target 与编译参数一致性测试。
- 至少 A–D 四类数据流变异的真实结果。
- 生成 MLIR 和 Binary 的身份链。
- 原始测试命令、退出码与日志。
- Git diff 与最终源码身份。
- Challenger 最终审查结论。

如果新发现的问题影响固定 Golden，必须修复并重新执行对应编译验收。

如果新发现的问题仅涉及不支持的 TIR 子集，允许明确拒绝并记录边界。

完成后停止，不启动 P6。

本轮仅说明 TileLang 代码是否 READY_FOR_P5_GATE。真正 P5 CLOSED 还需要下一轮当前机器门禁
（AgentGates）和最终独立裁决。

## 补充

我之前运行过一次当前任务，但因为某些原因未能完全完成，且遗留了一些修改

> 以下是整理时追加的现场快照（2026-10-28），不是用户原话：
> 上述"遗留修改"当前为 TileLang 工作区 5 个未提交文件
> （`src/ascend/codegen/hivm_codegen.cc`、`src/ascend/codegen/rt_mod_ascend_hivm.cc`、
> `tilelang/ascend/backend.py`、`tilelang/ascend/codegen.py`、`tilelang/contrib/hivm.py`）。
> 这些改动**必须保留**：不得 `git stash`、`git checkout --`、`git reset` 或代为提交，
> 本轮只在其上做最小增量。
