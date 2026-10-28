# P5 — TileLang HIVM 真正 DeviceCodeGen 开发

> **原文保留（Historical Harness 时代任务单，2026-10）。技术要求仍然有效。**
> 需要历史化的只有“用哪套 Harness / 谁来执行”这一面：
> - §任务身份 中“使用已安装的 `/engineering` 工作流”与“不得使用 Claude Code Agent Teams 或另一个
>   多 Agent 框架代替 PiEngineeringHarness”的规定**已被当前轮取代**：本轮改由 **DeepSeek Harness +
>   Agent Teams** 执行，机器门禁改由 **AgentGates MCP** 提供（见 `README.md` §2）。
> - §1 的“预期基线（GitHub 已确认参考版本）”只是**时间快照**，本地实际 Git HEAD 才是执行身份。
> - §2 的角色名（Explorer/Architect/Implementer/Validator/Challenger/Lead）在当前运行时下对应
>   Agent Teams 的同名角色；**模型路由以运行时（Harness）会话配置为准，本仓不记录也不指定模型别名**。
> - §3 的 P5-0 Adapter 准入（`bwrap`、`engineering-writer-guard`、Harness Runtime 兼容性）属旧执行器要求；
>   当前对应义务是 AgentGates `doctor` 现场核验与提交前 `verify_requirements`。
> - §7.1 “新增 P5 专用 Gate（`tilelang-hivm-codegen`）”与 §7.2 PackStore 重装流程是**该轮**的
>   P5-4 计划项，须以现场 Adapter/Receipt 核验其是否已落地；不要仅凭本文断言其状态。
>
> **其余全部技术要求（§4–§6、§8 禁止项、§9 交付与 P5 关闭标准）逐条不变，必须继续遵守。**

## 任务身份

你是 PiEngineeringHarness 的 Engineering Lead。

使用已安装的 `/engineering` 工作流，以及以下两个 Adapter：

- `tilelang-hivm`
- `tilelang-hivm-npu`

目标：实现真正的 Prepared TIR → Late HIVM/AVE MLIR → AscendNPU-IR Late Pipeline → Ascend950PR AIV Binary。

本轮正式授权 P5-0 至 P5-4。

禁止自动实施 P6/P7。

不得使用 Claude Code Agent Teams 或另一个多 Agent 框架代替 PiEngineeringHarness。

## 1. 现有项目

工作仓库：

- TileLang：`/home/s00653124/workspace/tilelang`
- AscendNPU-IR：`/home/s00653124/workspace/AscendNPU-IR-1`
- 控制仓：`/home/s00653124/workspace/hivm-poc-control`
- Harness：`/home/s00653124/workspace/PiEngineeringHarness`

预期基线：

- TileLang `d1281a98`
- AscendNPU-IR `bfefe180`
- 控制仓 `b42d39d4`
- Harness `4695e730`

以上仅为 GitHub 已确认的参考版本。本地实际 Git HEAD 是本次执行身份。

首先读取控制仓：

- `CLAUDE.md`
- `docs/current-state.md`
- `docs/goal.md`
- `docs/acceptance.md`
- `docs/late-ir-contract.md`
- `docs/p2-architecture.md`
- `docs/p2-binary-abi.md`

继续读取 P1 v2 已批准的 Late MLIR、P2 编译命令与工具身份，以及 P4 最终审查结论。

不得再次执行已经关闭的 P0–P4 开发任务。

## 2. 工作权限与 Agent 分工

遵守 Harness 当前 `AGENTS.md` 和现有角色定义。

- Explorer：只读调查 TIR、MLIR、Pass 和 Runtime。
- Architect：确定 CodeGen 接口及语义映射。
- Implementer：唯一生产源码 Writer。
- Validator：独立构建测试、运行语义 Oracle。
- Challenger：独立审阅代码、反例和验收结果。
- Lead：编排任务、记录 Receipt、向用户汇报。

优先并行两个只读角色；只允许一个 Writer 修改 TileLang 工作树。

使用当前 Profile 的模型路由。不得修改 `models.json` 或绕过 Model Scope。

每个 Agent 的报告必须引用实际工具输出。模型推断不是 Machine Evidence。

如收到需要用户裁决的请求，使用 Harness 的 Owner/Supervisor 机制。Lead 不得代替用户批准。

## 3. P5-0：环境与 Adapter 准入

先执行以下检查，再允许写生产源码。

### 3.1 Git 与 Writer Guard

检查：

- 实际 `git --version` 不低于 2.28。
- `bwrap` 在 PATH 中。
- `bwrap` 能成功创建所需的 user namespace。
- `engineering-writer-guard` 能在当前环境中执行一次受控写入测试。
- Harness Runtime 与 Pi 1.2.0、pi-subagents 插件兼容。

注意：最新 Harness 的 `require_bwrap()` 不再接受“命令存在但沙箱不可用”的情况。

如果 bwrap 报 `setting up uid map: Permission denied`，标记 `ENV_BLOCKED`，不得关闭 Writer Guard、换用无沙箱执行器或伪造写入成功。

### 3.2 Docker 与源码身份

检查 Docker daemon、现有容器、bind mounts 和源码所有权。

必须证明：

- 宿主机与容器访问同一份 TileLang 源码。
- 容器中导入的 `tilelang` 来自当前工作树。
- Native `.so` 来自当前构建目录。
- Docker 构建目录可被普通用户写入。
- 不存在残留的 Ninja 构建进程。
- 不存在 Docker 进程处于不可恢复的阻塞状态。

若 Docker 不健康，先报告阻塞原因。不得破坏其他容器或强制 reset 设备。

### 3.3 工具链和设备

读取 `adapters/tilelang-hivm/env.sh`。

核对 CANN、Conda、Python、torch、torch_npu、BiSheng 和 P2 编译工具。

检查已授权的 `TILELANG_HIVM_DEVICE_ID`。

不自动选择其他 NPU。

通过 Harness 执行当前三个 Machine Gate：

- `tilelang-hivm-build`
- `tilelang-hivm-regression`
- `tilelang-hivm-npu-sanity`

必须读取实际 Receipt。

设备 Gate 目前只验证固定 SIMD VectorAdd，不得标记为 P5 HIVM CodeGen PASS。

如果三个 Gate 全部具备执行条件，则继续 P5-1。否则将阻塞项分类为 HOST、SANDBOX、DOCKER、TOOLCHAIN、DEVICE 或 BASELINE，并提交最小诊断。

## 4. P5-1：冻结 CodeGen 设计与独立 Oracle

### 4.1 输入

读取冻结文件：

`tests/golden/hivm/vadd_f32_single_tile/golden_codegen_input.json`

SHA-256：

`576fa7a37bcaf1748e387efbd26508652e2e4b18790fa21aea874a7b756c0fe5`

通过 `tvm.ir.load_json()` 加载真实 IRModule。

禁止重新生成或修改 Golden。

该 IRModule 已经过 `_prepare_device_codegen_mod`，直接重放时不得再次执行准备 Pass。

### 4.2 输出

输出是 P2 Late Pipeline 可接受的真实 Late HIVM/AVE MLIR。

P1 的手写 MLIR 只用作独立语义 Oracle。

禁止从磁盘读取 P1 MLIR 再进行字符串替换。

禁止按照 Golden SHA 直接返回预置 MLIR。

### 4.3 架构调查

Explorer 与 Architect 需要回答：

1. `DeviceCodegen.build` 当前如何调用 P4 PREBUILT？
2. Prepared TIR 中 GM/UB、DMA、同步、VF 和循环的实际 AST 结构是什么？
3. 哪些 TIR 节点必须对应独立的 MLIR 生成逻辑？
4. `blockIdx` 如何进入每 block 的 GM 地址计算？
5. TIR UB 元素偏移如何转换为 HIVM 字节地址？
6. `pb32`、cache4、set/wait 和 PIPE_ALL 如何生成？
7. 如何复用 P2 的现有 Late 编译入口？
8. 如何保留 P4 Runtime 的 VECTOR_CORE Loader 策略？

优先实现薄型 C++ TIR Visitor 和 MLIR Text Emitter。

不要复制整个 `tilelang-mlir-ascend` 后端。

不要新增 MLIR C++ 链接依赖，除非源码调查证明现有机制无法满足契约。

### 4.4 测试先行

Validator 应先设计真实 TIR mutation 测试：

- 改变 blockIdx 数据流。
- 改变 UB offset。
- 改变 DMA payload。
- 改变 store cache4。
- 删除同步。
- 改变 pb32。
- 改变循环 extent。
- 添加不支持的 TIR 操作。

每个反例必须明确预期：

- 生成相应合法的新 IR；或
- 因超出已支持子集而明确拒绝。

禁止仅因输入 JSON SHA 不匹配就拒绝全部反例。

Challenger 确认测试能识别“固定 Golden MLIR 伪 CodeGen”。

P5-1 通过后冻结语义映射和测试契约。

## 5. P5-2：实现真实 HIVM CodeGen

Implementer 只修改 TileLang 生产代码。

新增独立 HIVM CodeGen 源文件和对应 CMake 配置。

第一版仅支持 Golden 的 TIR 子集。

### 5.1 Kernel

- `main_kernel(A,B,C)`。
- 三个 f32 GM 参数。
- Ascend950PR AIV。
- grid=64。
- 每个 block 8192 个 f32。

### 5.2 UB 和 GM

- A/B/C UB byte bases 分别为 0、65536、131072。
- 动态 UB arena 为 196608 bytes。
- GM 地址包含真实 blockIdx×8192。
- DMA payload 为 32768 bytes。
- store `l2_cache_mode=4`。

不得将元素地址与字节地址混用。

### 5.3 VF

- 从实际 TIR SIMD 区域识别 outlined VF。
- 生成 128 次循环。
- 每次计算 64 个 f32。
- 正确生成 vector load/add/store。
- 为 predicate 生成显式 pb32 类型。
- 保留零化及完整 mask 语义。

### 5.4 同步

- MTE2→V set/wait。
- V→MTE3 set/wait。
- 入口最终 store 后生成 PIPE_ALL。
- 不允许 VF 内的屏障替代入口终止屏障。

### 5.5 支持边界

只允许已明确支持的 TIR 节点、scope、dtype 和控制流。

对于不支持的操作，应给出包含节点类型和所在函数的诊断。

不得静默忽略节点。

不得通过调用 HFusion、PlanMemory、GraphSyncSolver 等自动 Pass 弥补生成器缺失的硬件决策。

### 5.6 Source-only 接口

实现正式 Source-only 入口，输入真实 Prepared TIR，输出生成的 MLIR 文本。

Source-only 不调用 BiSheng，也不需要 NPU。

验收：

- MLIR parse/verify 通过。
- P2 LateInputVerifier 通过。
- P1 语义契约全部满足。
- TIR mutation 正反例通过。
- 无 PREBUILT、AscendC、PTO 回退。

P5-2 未通过不得进入 P5-3。

## 6. P5-3：连接 P2 编译器

新增正式 DeviceCodeGen：

- `target.build.tilelang_hivm`
- Source-only 对应入口。

保留 `target.build.tilelang_hivm_prebuilt`，但限定为 TEST_ONLY。

将 `target="hivm"` 默认绑定到真实 CodeGen，而不是 PREBUILT。

### 6.1 调用链

真实流程：

Prepared TIR
→ TIR Visitor
→ Late MLIR
→ P2 `bishengir-compile`
→ HIVMC A5
→ BiSheng
→ 新 AIV Binary。

调用 P2 编译器时：

1. 使用明确的编译器绝对路径。
2. 显式传入 `--input-ir-stage=late-hivm`。
3. 显式选择 `Ascend950PR_9579`。
4. 使用已验证的 C310 bitcode。
5. 使用独立编译输出目录。
6. 保留命令 argv、日志和退出码。
7. 计算输入 MLIR、编译器、bitcode、输出 Binary 的 SHA-256。
8. 校验最终 ELF、Kernel symbol 与 ABI。

不得采用 shell 字符串拼接。

### 6.2 Binary 身份

新 Binary 必须来自本轮真实编译，不得读取或复制 P3 冻结 Binary。

无需强制新 Binary 的 SHA 不同于 P3。确定性编译可能得到相同字节。

但必须证明新 MLIR 被送入了真实编译器进程，并产生了本轮输出文件。

### 6.3 Module 与 Runtime

复用 P4 的 `AscendModuleCreate` 和 VECTOR_CORE Loader。

新生成型 Binary 应具有独立身份，不得继续显示 P3 PREBUILT 固定 SHA 来源标签。

Module 序列化和恢复必须保留正确 Loader policy。

P5 继续禁用 HIVM Kernel Cache 复用，不开展缓存优化。

P5 阶段不启动完整 TileLang → Runtime → NPU 数值 E2E。

## 7. P5-4：Harness 确定性验收

### 7.1 新增 P5 专用 Gate

在 P5 CodeGen 测试稳定后，允许扩展 Harness 的 `tilelang-hivm` Adapter。

新增独立 Recipe，建议命名：

`tilelang-hivm.codegen`

该 Recipe 应执行固定的 P5 自动验收。

建议 Machine Gate：

`tilelang-hivm-codegen`

要求报告可验证字段：

- `goldenLoaded`
- `generatedMlir`
- `mlirParsed`
- `lateContractPassed`
- `mutationTestsPassed`
- `compilerInvoked`
- `compilerIdentityVerified`
- `newArtifactProduced`
- `kernelAbiVerified`
- `noPrebuiltFallback`
- `testsCollectedPositive`
- `allPassed`

字段必须由实际运行、文件身份或结构化测试结果导出，不能由脚本无条件写 true。

Verifier 应验证失败用例确实让对应字段变为 false。

### 7.2 Adapter 更新纪律

修改 Adapter 后：

1. 为新测试提供独立、固定的 Recipe。
2. 保留现有 build/regression Recipe。
3. 重新计算被 manifest 固定的脚本 SHA。
4. 运行 `tests/core/test_tilelang_hivm_adapter.py`。
5. 通过 PackStore 正式重装并启用新 Adapter。
6. 让 Harness 读取新的 Receipt。
7. 不手动编辑 PackStore 内部索引。
8. 不用未经固定的临时 shell 命令伪造 Machine Gate。

不要修改 `tilelang-hivm-npu.sanity` 的固定 SIMD 测试。该 Gate 继续作为设备环境基线。

### 7.3 独立验证

Validator 必须独立执行：

- Source-only Golden 正例。
- TIR mutation 反例。
- Late MLIR Parser/Verifier。
- P2 LateInputVerifier。
- 正式 CodeGen → P2 → Binary。
- 目标 ABI 和 ELF 检查。
- AscendC/PTO/P4 PREBUILT 路径回归。
- Harness 新旧 Gate 回归。

Challenger 独立审查最终 diff、原始日志、Receipt 和全部语义不变量。

裁决只能是 APPROVE、REVISE 或 BLOCK。

## 8. 必须禁止的行为

- 禁止修改冻结 Golden、P1 Oracle 或 P3 Binary。
- 禁止在 P5 使用 P4 PREBUILT 冒充新 Binary。
- 禁止在测试失败时回退到 AscendC/PTO。
- 禁止单纯通过文本比较认定 MLIR 语义正确。
- 禁止把 Adapter 的 NPU Sanity PASS 当作 P5 PASS。
- 禁止绕过 Writer Guard。
- 禁止让只读 Validator 修改生产代码。
- 禁止对同一根因无限重试。
- 禁止未经授权清理容器、重置 NPU 或自动 rebase。
- 禁止未经用户许可推送代码。

## 9. 最终交付

更新控制仓 `docs/current-state.md`，记录 P5 的最新状态及证据入口。

提交：

1. 真实 TIR→Late MLIR CodeGen 源码。
2. 支持的 TIR AST 节点与映射表。
3. Golden Source-only 生成结果。
4. TIR mutation 测试。
5. P2 真实编译命令及新 Binary。
6. ELF/ABI/工具身份。
7. `tilelang-hivm` 新 CodeGen Machine Gate。
8. 构建、回归和 NPU Sanity Receipt。
9. 独立 Verifier 和 Challenger 报告。
10. P6 交接说明。

主 Agent 以中文汇报，按 `hai-simplified-technical` Skill 编写最终报告。

P5 关闭标准：

真实 Prepared TIR 经正式 CodeGen 生成 Late HIVM/AVE MLIR，再由 P2 编译为真实 Ascend950PR AIV Binary。语义测试、Mutation 测试、ABI、独立验证和 Challenger 均通过。

**P5 通过后停止。不得自动开始 P6 的真实 DeviceCodeGen E2E。**

