# P0 待裁决事项

**状态：已于 2026 年 10 月 13 日裁决完毕，本文件转为只读快照。** 权威记录是 `docs/decisions.md` 的 D021。本文件保留裁决前的原始表述，供追溯。裁决结果见第 6.2 节。

**追溯性修正（2026 年 10 月 13 日）：** 第 5.2 节的 D013 "已知事实" 做过一次事实更正，已用 **更正** 标出。原表述把 `ascend_reference.asc` 当作编译产物，实际它是 AscendC 源码；原引用 `FuncToLLVM.cpp:716-719` 过于简略。裁决内容（第 6.2 节）未改动。

日期：2026 年 10 月 13 日。

范围：本文件只列 P0 阶段未决的事项。事实依据是 `docs/status.md`、`docs/p0-verification.md`、`docs/decisions.md` 和 `/home/s00653124/workspace/hivm-poc-evidence/p0-20261013-late-pipeline/` 下的证据。本文件不改动上述文件，也不改任何门禁状态。

## 1 结论

**本节是裁决前的表述，已被第 6.2 节取代。** 保留原文供追溯。

P0 门禁未关闭，P1 未获授权。独立 Challenger 已给出裁决：Late-IR 契约边界为 REVISE，P1 推进为 BLOCKED；管线切割点对现有 flag 为 BLOCK，对架构为 REVISE。裁决不是 APPROVE。

未决事项分四类：

| 类 | 含义 | 事项 |
|---|---|---|
| A | 需你裁决，属架构或范围变更 | D012、D013、管线切割点 |
| B | 方案已明确，如无异议由团队实施 | D004、`AppendDeviceSpec` |
| C | 无需裁决，列为 P1 义务 | D009 |
| D | 已完成 | Golden 冻结、分支、二进制身份、D018 |

A 类定了，P1 才有依据。

## 2 怎么用这份文件

第 5 节逐项给出决策问题、已知事实、未知部分、选项、不决定的后果、关闭条件和 Lead 倾向。第 6 节是裁决清单和记录表。你只需回答第 6 节。

Lead 倾向是我作为协调人的判断，不是 Challenger 的裁决。凡与 Challenger 裁决冲突之处，以 Challenger 为准。

## 3 术语

项目没有术语表。下表是本文件用到的词，宜放进项目。

| 词 | 意思 | 不用 |
|---|---|---|
| 门禁 | 阶段之间的一道检查，不通过就不进入下一阶段 | 关卡、闸门 |
| 硬门禁 | 不关闭就不启动下一阶段的门禁 | 阻塞项 |
| 载体 | 把参数从 TIR 传到下游的 MLIR 属性 | 承载、通道 |
| 完成语义 | DMA 结束后，结果何时可见 | 收尾、屏障 |
| 裁决 | 你对未决事项做决定 | 决策（指你的动作时） |
| 关闭条件 | 满足它就能把该事项标为已解决的那份证据 | 完成标准 |
| Challenger | 只读的独立架构复核角色 | 挑战者、评审员 |
| Verifier | 独立测试与证据角色 | 验证者、校验员 |
| Lead | 协调与把关角色 | 负责人、主控 |

## 4 依赖关系

三项 A 类裁决互相独立，但都阻塞 P1。

1. 管线切割点决定 P1 与 P2 走哪条代码路线。不裁决，团队无法开始 P2 的入口设计。
2. D012 决定 P1 写出的 store 是否合法。不裁决，P1 的 Late IR 可能一开始就错。
3. D013 决定 P1 的尾部是否完整。不裁决，P1 可能在一个会竞争的内核上通过。

B 类两项不阻塞 P1 的启动，但在对应代码路径被触及时就会生效。

## 5 逐项详述

### 5.1 D012：`cache4` 的载体

**决策问题：** Golden 要求的 store `cache4` 用什么方式落到 HIVM 方言里？

**为什么现在定：** P1 要手写 Late IR。写 store 的那一刻就要决定 `cache4` 怎么表达。写错了，P1 的产物从第一条指令起就不符合 Golden。

**已知事实：**

- Golden TIR 第 43 行要求 store 带 `cache4`。GV-04 也记录了一次 32768 字节的 cache4 写。
- `hivm.hir.store` 没有 cache 操作数（`HIVMDMAOps.td:157`）。
- `HIVM_EvictionPolicyAttr` 只在 `LoadOp` 上，属 MTE2（`HIVMDMAOps.td:101`）。原始 `l2_cache_mode` 只在 `ND2NZOp` 上，也属 MTE2（`HIVMDMAOps.td:396`）。两者都不是 store 侧。
- store 的 libcall lowering 只追加 `atomicKind`（`HIVMToStandard.cpp:1020-1032`）。
- RegBase C310 模板把 `l2_cache_ctl` 硬编码为 0（`DMAUtils.h:277`）。
- 探针 SD-03 在 `hivm.hir.store` 上设 `{l2_cache_ctl = 4 : i64}`。该属性解析通过，但 `-convert-hivm-to-std` 的输出与 cache0 的 SD-01 逐字节相同，只有函数名不同。libcall 仍收到 `%c0_i32`。
- 冻结的 AscendC 参考如实转发 `asc_store_l2_cache_mode(4)`。

**未知部分：**

- `cache4` 与 `cache0` 在本负载上是否可观测。没有人测过。
- `cache4` 在 Ascend950PR 上的实际语义。没有设备级证据。
- 加一个 ODS 扩展的代价，以及上游是否接受。未知。

**警告：** 不要在 Late IR 里写 `cache4`，然后靠编译成功来确认它生效。该属性被静默丢弃。你写出 `cache4`，编译干净通过，拿到的是 cache0 的二进制。数值相等不能豁免这一项。

**选项：**

| 选项 | 做法 | 代价 | 关闭条件 |
|---|---|---|---|
| (a) 窄扩展 | 在 HIVM 方言加一个 store 侧的 cache 字段，改 lowering，经评审合入 | 改两个仓库的源码，需要独立评审 | 发出的 LLVM 里能看到 cache4 的载体，且有测试 |
| (b) 等价性论证 | 用独立的目标级源码或设备证据，证明 `cache4` 与发出的 cache0 对本 store、本目标语义等价 | 需要尚不存在的证据 | Challenger 接受该证据 |
| (c) 换 store op | 找一个自带 cache 字段的 op | 载体搜索已穷尽，没有现成 op | 找到一个 op，或承认 (c) 并入 (a) |
| (d) 明确降级 | 本 POC 放弃 `cache4`，在验收矩阵里把 B4 一行标为按范围 BLOCKED | POC 不再复现 Golden 的 store 语义 | 你书面接受该偏差 |

选项 (c) 与 (a) 实际是同一件事。载体搜索没找到现成 op，所以 (c) 最终也要改源码。

**不决定的后果：** P1 无法写出合法的 store。团队只能停在契约草稿。

**Lead 倾向：** (a)。(b) 缺证据，(c) 并入 (a)。(d) 是范围变更，只有你能定。

### 5.2 D013：末尾 DMA 的完成语义

**决策问题：** 内核末尾的 DMA 何时可见，这个问题在哪个阶段关闭？

**为什么现在定：** 它决定 P1 的尾部要不要加东西。若不加，P1 可能在一个会竞争的内核上通过，而通过不代表正确。

**已知事实：**

- 末尾屏障由 GraphSyncSolver 插入，不是由 legalizer 插入。`insertBarrierAllBeforeReturn`（`SyncSolverCodeGen.cpp:923-933`）在设备入口函数的 `return` 前插入 `PIPE_ALL` 屏障，调用点只在 `isIntraCoreMode()` 为真时（`:975-982`）。该 pass 对带 `hivm.vector_function` 的函数直接返回（`GraphSyncSolver.cpp:70-72`）。`InjectSync` 只在 `BARRIERALL` 模式下插入同一屏障（`InjectSync.cpp:50-67`）。上游 `ReturnOpLowering`（`FuncToLLVM.cpp:678-719`）把 `func.return` 映射为 `llvm.return`，不加屏障。Late 切割点跳过这两个 pass，因此丢掉该屏障。
- **更正：** 冻结的 `ascend_reference.asc` 是 AscendC **源码**，不是编译产物。它没有末尾屏障，不能说明编译器发出的代码没有屏障。原先"设备层很可能已补上"的推断比最初记录的弱，不再作为依据。
- Golden 的四个 flag 与 wait 操作应逐字保留。

**未知部分：**

- bisheng 设备层或运行时是否补上完成语义。没有查证。
- 缺了它是否真的竞争。没有实验。

**判断：** AscendC 路径是否也有这个问题，取决于 AscendC 编译器发出的代码，而冻结的参考是源码，无法回答。这个问题不能靠参考文件降低风险。实验只能测 MLIR pass 层，设备层留到 P3。

**选项：**

| 选项 | 做法 | 代价 | 关闭条件 |
|---|---|---|---|
| 1 在 P2/P3 关闭 | 带未决项进 P1，在 P2 或 P3 比对二进制 | 未决语义进入 P1，P1 的通过可能掩盖尾部缺陷 | P2/P3 的二进制比对显示完成结构正确 |
| 2 P1 前先做实验 | 用现有编译器做一个判别实验，观察末尾语义 | 推迟 P1 的启动 | 实验给出明确结论 |
| 3 显式加屏障 | 在 Late IR 尾部无条件加一个屏障 | 与 AscendC 参考不一致，多了一个参考里没有的东西 | 独立证据说明该屏障正确且无害 |

**不决定的后果：** P1 的验收标准里留一个空洞，P1 通过也不能说明尾部正确。

**Lead 倾向：** 选项 1。理由是 E0 已接受的参考也没有末尾屏障，说明设备层或运行时很可能已经补上。选项 3 与参考不一致，不宜先做。

### 5.3 管线切割点

**决策问题：** 用哪条路线绕过 AVE 优化器？

**为什么现在定：** 它决定 P2 的入口形态，也决定 P1 的产物要满足什么约束。

**已知事实：**

- D018 已否证 `--enable-direct-hivm-lowering`。在 `bishengir-compile` 的设备路径上，该 flag 到不了 AVE guard，因为 `setupHIVMAVEPipelineOptions` 不填充它。AVE 字段保持 `cl::init(false)`，guard 恒真，AVE 优化器总会运行。
- `buildLowerAVEPipelines` 只有一个调用点，在 `regbase/PassPipeline.cpp:223`。
- Challenger 独立确认了 D018。

**未知部分：**

- 专用入口的确切 pass 序列。要在 P2 用 pass dump 定。
- 保留哪些 legalizer。要逐项验证。

**选项：**

| 选项 | 做法 | 代价 | 关闭条件 |
|---|---|---|---|
| (a) 接 getter | 把 config getter 接进 `setupHIVMAVEPipelineOptions` | 窄修复，不是切割点 | 不适用，Challenger 认为它不够 |
| (b) 专用 Late 入口 | 在现有 RegBase 驱动加一个专用且互斥的入口 | 改驱动，需独立评审和 P2 pass 插桩 | P2 的 pass trace 证明只跑一次 `runHIVMCCompileA5`，且跳过列出的各阶段 |
| (c) 证明原样穿过 | 证明 primitive Late 输入原样穿过 AVE 优化器 | 无证据，且假设不安全 | Challenger 接受 |

方案 (b) 的具体要求：不加新 target kind；跳过 HIR/HFusion、bufferization、planning、auto-sync、loop lowering、multibuffer 和 `buildLowerAVEPipelines`；保留已验证的 legalization；只运行一次 `runHIVMCCompileA5`。

**不决定的后果：** P2 无法开工，P1 的产物也没有验收对象。

**Lead 倾向：** (b)。与 Challenger 一致。

### 5.4 D004：运行时谓词互斥

**决策问题：** 加了 `hivm` 键之后，三个后端谓词怎样保持互斥？

**已知事实：** `target_is_plain_ascend` 按缺少 `pto` 定义。加一个 `hivm` 键会让它同时匹配，`_resolve_backend_module` 会报错。出错的是解析，不是注册。

**方案：** 重新定义 plain 谓词，排除 `hivm`。三个谓词互斥。

**证据：** `lead/lead-source-trace.md` §19、§19a。

**Lead 倾向：** 按方案实施。这一项不改架构，属实现细节。如无异议，团队在 P5 前实施并加路由测试。

### 5.5 `AppendDeviceSpec` 非幂等

**决策问题：** Late 入口遇到已存在的 device spec 时，校验还是覆盖？

**已知事实：**

- 该 pass 在已有 spec 不同时先警告，再无条件覆盖（`AppendDeviceSpec.cpp:142-151`）。
- `setNPUTargetSpec` 构造一个全新的 `TargetSystemSpecAttr`，不是合并（`HACC/Utils/Utils.cpp:192-202`）。
- 共有三个 `createAppendDeviceSpecPass` 站点。
- Verifier 补充：只在已有 spec 不同时才非幂等；同一 pass、同一选项重跑是值幂等的；TTIR 与 HIR 是两条管线，当前驱动不会加两次。

**方案：** Late 入口校验一个完整且匹配的 spec。不匹配时报错，不静默覆盖。

**理由：** 静默覆盖会把目标 spec 从手写 Late 模块下面换掉，与本项目反对静默失败的一贯立场一致。

**Lead 倾向：** 校验并报错。如无异议，团队在 P2 实施。

### 5.6 D009：谓词粒度（无需裁决）

**结论：** 机制已确认，列为 P1 义务，不需要你裁决。

**事实：** `AVEPgePattern`（`AVENormalizeOps.cpp:587-631`）在 `functionType` 缺失时按 `VL_BITS/numElems` = 2048/numElems 推导宽度，只接受 8、16、32。Golden 的掩码是 `boolx256`，2048/256 = 8 是合法宽度。所以省略载体得到的是静默的错误代码 `pge.b8`，不是诊断。掩码为 32 元素时，推导出 64 并中止（PG-03，rc 134）。

`HIVMPgeOpLowering`（`HIVMAVEToAVEIntrin.cpp:1419-1443`）在这里盖 `mask_bit_width`，它自己不做校验。`buildPgeOp`（`RegbaseUtils.cpp:604-629`）是第二道 guard。Golden 的推导值 8 被两道 guard 静默接受。

**注意：** 模板 `arith-constant-op-to-hivm-regbase-broadcast.mlir:20` 写 `pge <ALL> : vector<64xi1>`，不带载体，会静默推导出 32。该模板有误导性，不应照抄。

**P1 义务：** 直接断言发出的 `mask_bit_width = 32`。编译成功不是证据。

### 5.7 已完成，仅需知悉

- Golden 冻结：PASS。Verifier 的独立脚本 55/55 通过，退出码 0。会话末重算五个 fixture 的 SHA-256，未变。
- 分支：两个分支已存在且干净。本阶段未提交、未推送。
- 编译器二进制身份：RESOLVED。`build/bin` 与 `build/install/bin` 只差 RUNPATH 的一个字节。容器 `PATH` 解析到 CANN 9.2 工具链，不是本地构建。
- D018：`--enable-direct-hivm-lowering` 到不了 AVE 优化器的 guard。该假设已否证。

## 6 请你裁决

### 6.1 裁决清单

1. D012：选 (a) 窄扩展、(b) 等价性论证、(c) 换 op，还是 (d) 明确降级？
2. D013：选在 P2/P3 关闭、P1 前先做实验，还是显式加屏障？
3. 管线切割点：是否批准方案 (b) 专用 Late 入口？
4. B 类两项（D004、`AppendDeviceSpec`）是否按 Lead 倾向实施？
5. P1：是否启动？

### 6.2 记录表

裁决日期：2026 年 10 月 13 日。权威记录：`docs/decisions.md` D021。

| 事项 | 你的裁决 | 日期 | 备注 |
|---|---|---|---|
| D012 | 选 (a) 窄扩展 | 2026-10-13 | store 侧 cache 载体扩展，需独立评审；P1 应断言载体出现在发出的 IR 里 |
| D013 | 选 2，P1 前先做实验 | 2026-10-13 | 用现有编译器做判别实验；预期结果在实验前冻结；实验排在 P1 生产工作之前 |
| 管线切割点 | 批准 (b) 专用 Late 入口 | 2026-10-13 | 与 Challenger D020 的裁决一致 |
| D004 | 按 Lead 倾向实施 | 2026-10-13 | 重新定义 plain 谓词，排除 `hivm` |
| `AppendDeviceSpec` | 按 Lead 倾向实施 | 2026-10-13 | 校验完整且匹配的 spec，不匹配时报错 |
| P1 是否启动 | 启动 | 2026-10-13 | 附加顺序约束：D013 实验先于 P1 生产工作 |

**边界未解除：** Challenger 的 D020 裁决仍为 REVISE / BLOCK，未转为 APPROVE。D012 与 D013 仍为硬门禁，直到各自的关闭条件满足。Golden 保持不可变。P2–P6 未授权。

## 7 证据位置

证据根目录：`/home/s00653124/workspace/hivm-poc-evidence/p0-20261013-late-pipeline/`。

| 事项 | 主要证据 |
|---|---|
| Golden 冻结 | `verify/verify_golden.log`、`verify/summary.tsv` |
| 二进制身份 | `verify/build-artifact-identity.json`、`verify/path-tool-resolution.log` |
| D012 | `lead/lead-source-trace.md` §17、`p0v2/logs/SD-01.out`、`SD-03.out` |
| D013 | `lead/lead-source-trace.md` §18 |
| 管线切割点 | `lead/lead-source-trace.md` §16、`docs/decisions.md` D018 与 D020 |
| D004 | `lead/lead-source-trace.md` §19、§19a |
| `AppendDeviceSpec` | `verify/challenger-claims-check.log`、`docs/decisions.md` D020 |
| D009 | `lead/lead-source-trace.md` §28、§30 |
| Challenger 裁决 | `docs/decisions.md` D020 |

**注意：** 本文件与 `docs/decisions.md` 记的是同一批事项。`decisions.md` 是权威来源。裁决后只更新 `decisions.md`，本文件应随之作废或转为只读快照。
