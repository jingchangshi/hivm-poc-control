# 当前状态：P2 Final Gate / P3 交接

2026-10-15（gen 6）。本文件提供当前入口，历史原文保留在 `status.md` / `decisions.md`。
前九代均逐字节保留、均可复算（权威清单 `EV/resume-04/lead/CURRENT-STATE-MANIFEST.json`，其 6 行自校验通过）：
gen 1（门禁中途稿）`…/current-state.gen1-pre-rewrite.md`（`4307866895…`，6,112 B）、
gen 2（含 D129 归属更正前措辞）`…/current-state.gen2-pre-correction.md`（`533b0b23…`，14,473 B）、
gen 3（含 D129 更正、无 §10）`…/current-state.gen3-pre-loss-note.md`（`008105d5…`，16,716 B）、
gen 4（有 §10、但清单身份说法有误）`…/current-state.gen4-pre-anchor-note.md`（`fda6a40c…`，18,674 B）、
gen 5（更正 D131 表述、未含本 §10(c)）`…/current-state.gen5-pre-generator-defects.md`（`d53d279d…`，19,496 B）。

**结论：P2 可被有界接受（SCOPED APPROVE，限固定 Golden F32 VADD 的 Late→A5 Binary 编译范围）；
P3 的设计与 Binary/ABI/Launch 交接已完成，但 P3 实机执行尚未授权，本轮停在 P3 门禁。**

## 1. 最终 verdict

Challenger 正式 verdict 原文（verbatim，由 Lead 代为落盘；Challenger 为只读审查者，不写文件）：
`EV/resume-04/challenger/CHALLENGER-FINAL-VERDICT.txt`，sha256
`18af1997b5181388ddcde1b049b83d6a3d7669fc2ce84abab3394297766c3220`，12689 B，99 行。
源 transcript `subagents/agent-achallenger-final-f41fb199b9a8b05a.jsonl`（1197682 B，
14:53:20Z，事后静默监测确认 1197682 B 即为终代）。

裁决：**SCOPED APPROVE for the fixed-Golden F32 VADD Late→A5 Binary scope**。
先前 REVISE 的四项有界缺陷全部关闭，未发现阻塞性回归。
普通 RegBase SIMD 完整 ELF 仍为 **BASELINE_BLOCKED**；硬件数值正确性仍为 **NOT_RUN**。
该裁决解除 P2 技术审查阻塞，但本身不授权 P3 执行，也不主张硬件正确。

## 2. 仓库、工具与身份

HEAD：IR `82f3ad088ab3cea0e81fa6627e0487a93e5317ef`；TileLang `cc56643de04a49db6f50875e26d904b0c69e71a6`；
control 起点 `3cc7646b52a6fff9bfec141a784839c899db72b6`。IR 相对 staged-source `4495ab29b` 仅两个测试文件
+295，生产代码零差异（本轮独立复核：`EV/resume-04/lead/LEAD-SOURCE-SCOPE-AUDIT.txt`）。
IR 的 `origin` 查询超时，远端一致性 UNVERIFIED，未改远端配置。
served-model 审计仍 UNVERIFIED（未找到直接 Magpie 请求日志），见
`EV/resume-01/lead/model-audit-limitation.json`。

本轮修复后的生产客体（**新构建，非沿用旧 SHA**）：

| 客体 | SHA-256 | bytes |
|---|---|---:|
| `LateInputVerifier.cpp`（live，本轮唯一生产改动） | e19080ab1bd772fac2c5b790b22d4c62d1b62857f3be4ce941e044fc07b387bf | 62538 |
| 新 `bishengir-opt` | 9ea6f21b61d620e3752339ff66fd6ff5d382fd58bcb86ad1fd32dccf44b07e70 | 214636848 |
| 新 `bishengir-compile` | 2206d8c54e1b3c3e4689f92efdcee26baedade398711b0d9d9a2242dc27852eb | 205494152 |

相对 HEAD `82f3ad088` 的 diff：`LateInputVerifier.cpp` 491 insertions / 68 deletions，
另 `CMakeLists.txt` 6 insertions（补 `MLIRAVEDialect` 链接边）。
两处被取代的中间构建（`27fc3feb…` 首建、`df060073…`）已登记为 superseded，不得引用。

**该修复留在工作区、未提交**（IR 工作区恰有两处 `M`：`regbase/LateInputVerifier.cpp`、`regbase/CMakeLists.txt`；
HEAD 仍为 `82f3ad088`）。这是**刻意**的：本记录以**内容哈希** `e19080ab…` 绑定修复，而非以 commit 绑定，
这样 goal §1 钉住的 IR HEAD `82f3ad088` 保持有效、后续复核不会因 HEAD 位移而误报。本轮未 push、未 rebase。

冻结客体（未因本轮审查而失效，全部原样）：

| 客体（相对 `OLD=/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline`） | SHA-256 | bytes |
|---|---|---:|
| `impl/stage-p2c-r1/bin/bishengir-compile` | 812c5cb1d29cb3b8879593ea77d08ebdae274cc943d41523c7c4bab739cbf455 | 205469200 |
| `impl/stage-p2c-r1/lib/meta_op.aiv.c310.O2.bc` | 3807eb3d907236b17855db57a475ea120c37a74905c92bb4b971b415da7d6f79 | 31206992 |
| `impl/stage-p2c-r1/lib/meta_op.aiv.c310.O0.bc` | b8c68dcb1ef506464c2a27d83178772386e9cf65a834b9a8c9a71c8a41f27078 | 27695840 |
| `impl/stage-p2c-r1/SHA256SUMS.txt` | 6aa3c1ef0b818418e641f7713a0e0a3be991b609ad787c968279abff81819cf4 | 377 |
| `impl/build-head-20261015/bishengir-opt.3514f5fe`（修复前工具） | 3514f5fee810ce59795f6bb47387c451195e14550a81746a9d0fa321c58d00ae | 214611896 |
| `verify/tests/negatives/vadd_late_p1v2.mlir`（冻结 fixture） | d1023ac1fdfd22c8d7f2aed5c71241fdd83039744b383e43b19255d10a31f36b | 18659 |
| `verify/work/run/A1/kernel.o`（P3 唯一设备程序） | 2c4247c3b2ce11f79ad15415eb06d4ce1d5523a0c19c91ad3743bc08e7b8c4c9 | 4448 |

manifest 从自身目录校验 4/4 OK、rc=0，且已绑定 manifest SHA（`EV/lead/lead-manifest-check.txt`）。
本轮 20 项身份清单 `EV/resume-04/lead/P2-FINAL-IDENTITY-INVENTORY.json` 全部可复算；
Challenger 独立重算 13 项一致，并独立复现 manifest 4/4。

## 3. 本轮修复与最小反例（§4.3）

四个有界缺陷，全部落在 `LateInputVerifier.cpp`（+ 构建接线）：

1. **V5 载体**：由"任意属性文本含 `pb32` 子串"改为消费侧同构的类型化谓词
   `getAttrOfType<hivmave::FunctionDistTypeAttr>("functionType")` 且枚举为 `PB32`，
   并限定在受支持 VF 的实际 pge 上（非模块级布尔）。
2. **V6(c) GM 链**：由"遍历 `arith.muli` 检查常量"改为沿 load/store 的 GM 操作数回溯
   subview → 入口参数，要求冻结偏移链（`index_cast(muli(get_block_idx,8192))`、size 8192、stride 1）。
   **该修复的第一版有真实回归**：固定取 `operand 0`，而 load 的 GM 在 operand 0、store 的 GM 在
   operand 1，故误拒冻结基线。**发现顺序须如实登记**：Implementor 自己的 runner 先看到基线被拒
   （14:30:13Z，"the baseline regressed — it must accept"）并在 14:30:23Z 独立得出同一根因、
   14:30:24Z 改为按地址空间选择 GM 操作数；Lead 在 14:29:59Z 用**另一件仪器**（直接调用，非其 runner）
   对当时在场的构建 `27fc3feb…` 独立复现同一缺陷，14:45:28Z 才把报告送达——**Lead 是独立复现者，
   不是首先发现者**（`LEAD-FOUND-REGRESSION-store-operand.txt`，其标题"LEAD-FOUND"为当时用语，
   已由本行限定）。两件仪器在约 14 秒内各自命中同一缺陷，这才是该行的证据价值。
   这是 §4.3"若失败由 P2 修改引入，则必须修复，不得豁免"的实例，且**未进入任何被接受的二进制**。
   另有 `x4_store_gm_arg_swapped.mlir`（`555bbcca2a360355956c4060e1d4072af982ce0328ac6defe1c226a6a5ce90a1`）
   由 Implementor 预先登记预期后运行，把"store 绑定到 C（argument #2）"从**阅读**变成**测量**
   （诊断字面量 `addresses entry argument #0, expected argument #2 (C)` 只能由 `argNo != i` 合取式产生）；
   基线接受本身只证明"解析成功"，不证明绑定到哪个参数。
3. **函数体保护**：bodyless/external 入口在 V6/V7 解引用前诊断，不再触发断言（原 rc=134、stdout 0 字节）。
4. **V6 同步顺序**：由"计数并集合比较"改为固定 VADD 的顺序关系
   （两次 load → MTE2/V set → 匹配 wait → VF call → V/MTE3 set → 匹配 wait → store → 终结 PIPE_ALL）与精确 pipe/event 元组。

三个新增反例（Verifier 编写，预期先于实现冻结）：

| 反例 | SHA-256 | 修复前 | 修复后 |
|---|---|---|---|
| `new01_v5_functiontype_wrong_type.mlir` | dd32abe8f919c3d29dc36bc62c652f900c8d03b37ad516d15e0ec6fc29fedc53 | 接受（洞） | 拒绝，V5 诊断 |
| `new02_v6_c_only_offset.mlir` | db730e165f3d4cd730b2ac63758ee5587ba23d627200a6e727c31f45269f260d | 接受（洞） | 拒绝，V6 诊断 |
| `new03_v6_vmte3_pair_after_store.mlir` | af26e888444abd333cdc3f1e3041eb4e556fd3858cdd55276845ac91fe02bcce | 接受（洞） | 拒绝，V6 诊断 |

**计数诚实性（"一行不等于一次独立测量"）**：Implementor 另有一组 `x1/x2/x3`（`impl/inputs/`，ctime
22:11:37），与上表**是同样三个变异**，不是另外三个：`x1` 与 `new01` **逐字节相同**（`dd32abe8…`）；
`x2`/`new02` 同为"C 的偏移置零"、`x3`/`new03` 同为"V/MTE3 对移到 store 之后"，仅局部变量名与注释不同
（`27d0b2f2…` vs `db730e16…`；`28f92b6c…` vs `af26e888…`）。故：
(a) 两套共 **3 个不同变异**，不是 6 个——`new02`/`new03` 与 `x2`/`x3` **不是彼此独立的证据**；
(b) 但它们由**两名不同作者、在不知会对方的情况下**从同一冻结 fixture 独立构造出**同一语义**，
这是**跨作者的收敛证据**（价值在于两人独立命中同一变形，而非行数）；
(c) `x1` 与 `new01` 逐字节相同，**无法区分"独立收敛"与"复制"**，故这一条只作弱证据；
(d) 两套的 ctime 相差 40 秒且 `x*` 略早，两者都源自同一份 WORK-ORDER 的闭包实验清单，
故"独立"的准确含义是**未互相复制**，不是**未受同一指令驱动**。

修复前臂原始流：`EV/resume-04/impl/pre-fix-new/`（三条 sem_rc=0）。
Lead 独立复跑全部 9 个变异体（4 个原反例 + 3 个新反例 + 2 个对照）于新二进制上，
9/9 符合预期且诊断为正确理由：`EV/resume-04/lead/LEAD-POSTFIX-RECHECK.txt` 与 `postfix-recheck/`。
冻结基线在新二进制上接受（rc=0，两流空）：`guard.stdout`/`guard.stderr`/`guard2.*`。
另确认 **Contract Gate 对 GM 地址链零覆盖**（14 个标签，`LEAD-CLOSURE-ACCEPTANCE.txt` 旁
`CONTRACT-GATE-GM-COVERAGE.txt`），即 §4.3 第 2 项必须由 verifier 自身覆盖。

**§6.4 不变量**：验证器为只读 preflight，冻结 fixture 本就全过，故修复不得改变其 A5 产物。
实测：pre-fix 编译两次（确定性对照）+ post-fix 两次，四个 `kernel.o` 全部逐字节相同
（`2c4247c3…`/4448 B）；stderr 在 stage 路径归一化后一致；compile dump 中存在 LateInputVerifierPass。
证据：`EV/resume-04/lead/LEAD-A5-INVARIANT-CHECK.txt`、`impl/a5x-{pre,post}-{1,2}/`。

## 4. P2 验收状态矩阵

run-of-record 取代规则（D244）：选择"最新一次真正执行过该行的运行"。suite-v6 判为 VOID
（46 行全 NOT_RUN，且其 RETIRED 行实现的是被 追加 252 §4 反转的指令），不计测量。
有效计数：**46 行 = 45 PASS / 1 NOT_RUN（C3 non950 完整 binary）**。
机器可读：`EV/resume-04/lead/p2-acceptance-matrix.json`（sha256 `9932fea2c68b94d3f4d8bfadadfe0365dbdc07cc70026d4a23a76bb436f0432c`）；
Lead 独立重数：`P2-ACCEPTANCE-MATRIX-VERIFY.txt`；Challenger 独立重建同一计数。

**该矩阵是历史仪器的混合体**（历史执行 + 定向复跑 + 本轮新二进制检查），不是"46 项新二进制检查"。
Challenger 明确要求如此登记。

| 验收项 | 状态 / 限界 | 定位 |
|---|---|---|
| 固定 Late VADD → A5 binary | PASS；冻结 kernel 身份本轮 MATCH | `OLD/verify/logs/suite-v3/20261014T172238Z-190190/A1.*` |
| 路由、绕过 ordinary/inference/retry、单次 A5 | PASS（执行证据；不得用统计构造数冒充执行数） | A1/A12 定向复跑；`docs/p2-pass-trace.md` 旧 NOT_RUN 为历史 |
| 尾部合法化与单次 ConvertHIVMToLLVM | PASS；成功次数 0/1/2 聚焦批准 | D122–D125；`OLD/impl/build-head-20261015/R5-CLOSURE-2.txt` |
| 模板选择/存在/身份/attach | PASS；manifest 4/4 OK；O2 选中 | A1.stderr；B12a/B12b v4；D126 |
| D012 cache4/2、absent/0、non950 ABI、DMA-source negative | PASS；仓内 7/7 lit；R2 聚焦 APPROVE | D125；两份 `store-l2-cache-mode*.mlir` |
| 链接侧 cache4 | CLOSED BY OBSERVATION（ABI/编码证据） | D127；冻结 ELF descriptor bits 60–63=4，load 侧对应字段 0 |
| ABI/UB/grid/GM/sync/b32、末区合法化 | PASS（观测）；P3 硬件行为未执行 | D126，C2.stderr 最后 AppendUsePrintDebugData 区 |
| Kernel/FunctionInfo 一致性 | name 与参数个数/24-byte packing 一致；tags NOT_OBSERVABLE | `OLD/lead/functioninfo-consistency-check/…txt`（862 行世代） |
| 普通 RegBase SIMD 完整 ELF | **BASELINE_BLOCKED**，不得报 PASS | D121/D126；本轮新证据 `resume-04/challenger-execution-_zf5ok9g/ordinary.{stderr,o,rc}` |
| non950 完整 binary | **NOT_RUN**（C3）；三操作数 lowering 测试不等于 binary 成功 | suite C3 / F.2 |
| 新反例 1/2/3（§4.3） | PASS（修复后拒绝；修复前接受） | `EV/resume-04/verify/new-negatives/`、`impl/pre-fix-new/` |
| §6.4 冻结 fixture A5 不变量 | PASS（6 个 kernel.o 逐字节相同） | `EV/resume-04/lead/LEAD-A5-INVARIANT-CHECK.txt` |
| P3 loader/lookup/launch/全量数值 | NOT_RUN | 仅设计交接，未运行 ACL |
| P4 runtime E2E / P5–P6 CodeGen E2E | NOT_RUN，后续阶段 | 本轮禁止提前实施 |

历史时序（不得自行合并局部 PASS）：D121 REVISE → D122/R2/R5 修复执行 → D124 测试修复 →
D125 聚焦 APPROVE → D126 汇总 NOT ACCEPTED → D127 更正链接侧 cache4 → 本轮 resume-03 REVISE（4 项）
→ resume-04 修复 → 本轮 SCOPED APPROVE。v3 首次 37 PASS/6 FAIL/3 NOT_RUN 原样保留；
v4/v5 选中重跑取代相关行；v6 VOID；R1 为 A1 late 半的重复覆盖。

## 5. BASELINE_BLOCKED 的隔离依据

普通 RegBase SIMD 完整 ELF 失败在基线与新分支上同因同形，因此保留 BASELINE_BLOCKED：

- 两条历史臂与**本轮修复后的** `bishengir-compile` 均 rc=1，未定义符号
  `_mlir_ciface_vadd_1d_float`，且产出**相同的 9072 字节 prelink 对象**
  `7200a267a4caafeaf159a0f40b9f710990d601603bda22d7d6168115d8165ec9`。
- 该路由上 Late verifier 未参与；本轮生产改动不触及 ordinary 路由、合法化尾部、HIVMC lowering、
  D012 生产逻辑与仓内测试（`LEAD-SOURCE-SCOPE-AUDIT.txt`）。
- 因此该失败不是 P2 修改引入，属既有非 P2 故障，按 §5 保留 BASELINE_BLOCKED，
  不谎报完整 SIMD ELF PASS，也不豁免为 PASS。

## 6. 受限能力登记（本轮不修，不开新项目）

按 §4.3 只修有直接验收价值的缺陷，以下登记为受限能力：

- 验证器只支持固定结构 VADD 形态，不构成任意程序等价性、通用 CFG 同步或所有 UB/VF 数据流变异的证明。
- 等价的替代地址表达式、多 pge 形态可能仍不受支持。
- 动态 UB 运行期分配、ACL 加载、HostArgs 执行、FunctionInfo 传播、数值正确性均为下游义务。
- 三处记录性更正（Challenger 提出，Lead 独立复核，裁定 RECORD / DO NOT REPAIR，因为改注释会重建并改变
  已被封存裁决所判定的二进制身份 `9ea6f21b…`）：
  1. `LateInputVerifier.cpp:703-708` 注释高估了旧检查对"无关算子上文本"的接受能力；实测旧规则只遍历
     `ave.hir.pge`，`ctl_other_op_pb32.mlir` 在修复前即被拒绝（semantic rc=1），原因是载体被删除而非无关算子规则。
  2. `singleEntryBlock` 对多块函数返回 `front()`，只守了 `getBody().empty()`；由 V6 单独拒绝多块入口缓解。
  3. 旧稿中的"468 insertions"只存在于 Lead 的工作小结，未进入任何工件；`fix.diffstat.txt` 本就为 497/68。
  证据：`EV/resume-04/lead/LEAD-CHALLENGER-RECORD-CORRECTIONS.txt`。

## 7. P3 交接

设计见 [独立 ACL Harness 设计](p3-acl-harness-design.md)。唯一设备程序为上表 `kernel.o`：
ELF64 little-endian、ET_EXEC、machine `0x1029`、flags `0x990000`、导出 `main_kernel`（GLOBAL FUNC）；
HostArgs = A/B/C 三个 64-bit GM 地址、24 bytes；grid=64；dynamic UB=196608 bytes；N=524288；
UB 基址 0/65536/131072。

CANN 9.2 头文件与正式 `x86_64-linux/lib64/libascendcl.so` 的五个所需 `aclrt*` 导出已核对，
**真实 loader 尚未执行**。P3 首个子门禁必须是：加载同一份已验 SHA 字节并成功 lookup `main_kernel`，
记录两个真实 ACL rc；失败即 `LOADER_BLOCKED` 并停止 launch，不得换用 AscendC/PTO 或旧 ELF 伪装成功。
P4 必须经 `ExtractFuncInfo`/`AscendModuleCreate`/`launch_param_tags`/`aclrtLaunchKernelWithHostArgs`
自动传播 launch 配置；不得假定运行时消费 `hacc.block_dim`。

## 8. 无需重建原 Binary 的证据

本轮唯一的 P2 新缺陷都在 Late 输入验证器（只读 preflight）内部，且 §6.4 已实测：
冻结 fixture 经 pre-fix 与 post-fix 编译器产出的 A5 产物逐字节相同（`2c4247c3…`）。
因此 **P2 原设备 Binary 无需重建**，P3 继续使用冻结的 `kernel.o`（`2c4247c3…`/4448 B）作为交接客体。
新二进制 `9ea6f21b…`/`2206d8c5…` 只承担"验证器能拒绝四类非法输入"的新证据，不改变冻结产物。

## 9. 待办与门禁

- P2：有界接受（SCOPED APPROVE）。不再有待修的 P2 缺陷。
- P3：设计与交接完成；**实机执行未授权**。本轮停在 P3 门禁。
- 未决的外部核对：served-model 审计 UNVERIFIED；IR 远端一致性 UNVERIFIED。
- 历史记录（`status.md`/`decisions.md`）只追加，不覆盖。

## 10. 本轮记录性事件（不是验收缺陷，但读者应知）

**(a) 一次证据字节丢失（D130 / 追加 260、261）。** 收尾工件 `P2-FINAL-GATE-DELIVERABLES.txt` 被就地改写多次，
其中 **rev 1 / rev 2 / rev 3-pre 三代字节未先快照、已不可恢复**（`3e2b68f7…`/10,041 B、`dba9e7e1…`/13,359 B、
`af77bdbe…`/14,617 B）。检索范围：`os.walk` 遍历 `hivm-poc-evidence/`、`hivm-poc-control/`、`/tmp/claude-1071/`，
按精确大小分别取候选（各 0 个）再逐个 sha256 比对 → 均未找到（范围限三根 + 精确大小，故诚实表述为"该检索未找到"）。
**同一轮里 `current-state.md` 被改两次、两次都先做了快照**——纪律对一件工件做了、对另一件没做，这才是缺陷所在。
（(d) 会把"丢失"的判据收紧：真正不可恢复的是**既未快照、改写内容又未被逐字记录**的世代；本文件被改的两次
都留下了快照，索引的 rev 1/2/3-pre 则两样都没有。）
自 rev 3 起该索引每次改写前先 `cp -p` 快照（`.rev3.snapshot.txt`、`.rev4.snapshot.txt` 可复算）。
完整清单另记于 `EV/resume-04/lead/LOST-GENERATION-deliverables-index-rev1.txt`（含 addendum）。
**无任何 verdict、计数或二进制身份依赖这些字节。**

**(b) 取消"记录钉清单哈希"的做法（D131 / 追加 262、263 / D132）。** 身份复核清单是**每轮重生成**的活工件，
记录里转录它的 sha256 必然在下一次写作时失效（本轮已发生**四次**：`cd51ae1a…`、`14aef64b…`、
`87f81305…`、以及锚文件首版的 `8f123205…`）。故绑定改为**单向链、无环**：

```
IDENTITY-SWEEP-FINAL-gen3.sha256.txt   ← 锚：只写清单的 sha256 + bytes + 复算命令
        ↓
IDENTITY-SWEEP-FINAL-gen3.txt          ← 清单：**不含**自身哈希
        ↓
P2-FINAL-GATE-DELIVERABLES.txt         ← 索引：被清单以 sha256 绑定；不打印清单/锚的哈希，只按路径指向锚
```

**清单自身身份写在清单文件内部是不可能的**（文件无法包含自己的 sha256）；D131 曾如此表述，已由 D132 更正。
锚与清单由**同一个脚本** `EV/resume-04/lead/regenerate-identity-sweep.py` 一步生成：该脚本在全部行复现之前
**拒绝写出任何东西**，故不会产生半截清单。读者若需现行清单身份，请读
`EV/resume-04/lead/IDENTITY-SWEEP-FINAL-gen3.sha256.txt`（**锚**），或直接复跑该脚本——**不要从记录里引用任何哈希**。
同理，索引内不再声明自己的版本号（"rev N 为现行"每笔即过时）。

**(c) 清单生成器自身的三处缺陷（D133 / 追加 265）。** 把清单身份收进脚本后，脚本暴露了三个缺陷，
均为**先测量、再修**：旁置命名把后缀追加到整个文件名（`…sha256.txt.superseded-XXXX.txt`）；
幂等重跑会把活文件按自身 sha 旁置成**逐字节相同的双胞胎**；普查副本被旁置进**普查自己所枚举的目录**，
既破坏幂等性又被误分类为 `sweep` 世代。结构性修复：统一 `<kind>.superseded-<old-sha8>.txt`、
字节相同则不旁置、**派生文件（普查）不做旁置**、无法归类即抛错。普查改由生成器同一步写出。
`SUPERSEDED-SWEEP-CENSUS.txt` 统计的是**盘上文件**，不是历史代数，且**非单调**（去重合并曾使其减少）；
早于 23:19 的世代从未旁置、刻意不列入。以上均不改任何 verdict、计数或二进制身份。

**(d) 一次"未快照就地改写"的字节恢复，以及它收紧的丢失判据（D134 / 追加 266）。** 收尾索引
`P2-FINAL-GATE-DELIVERABLES.txt` 在本轮**又被就地改写一次**（为撤回"清单身份写在清单内部"这句已由 D132 更正的
表述，并修正两处过期的行数），**改写前未快照**——即 (a) 记录的那个缺陷在同一份文件上重演了一次，这一点如实登记。

但这一代**已按字节恢复**。改写 #1 的三处替换在记录中逐字保留，把同样的替换重新施加于改写前的快照
`P2-FINAL-GATE-DELIVERABLES.rev10.snapshot.txt`（`6db60dae…`/20,030 B），得到 `1b16d7d1…`/20,584 B，
与改写 #1 之后当场实测的值**逐字节相符**（sha256 与 byte size 均相同），已存为
`EV/resume-04/lead/P2-FINAL-GATE-DELIVERABLES.rev11-recovered.snapshot.txt`。

**可恢复性不取决于是否快照，而取决于改写是否被逐字记录。** rev 1/2/3-pre 的替换内容未被完整记下，故不可重建；
这一代的替换内容在记录里逐字存在，故可重建——尽管它同样"未先快照"。故 (a) 的表述应收紧为：
真正丢失的是**既未快照、改写内容又未被逐字记录**的世代；"未快照"本身并不足以断定丢失。
这次恢复**不改变** rev 1/2/3-pre 仍然丢失的结论（那三代的替换内容确实没有被记下）。

**(e) 清单的"行数"曾把一次测量算成两次（D135 / 追加 267）。** 复核清单枚举索引快照时，`rev7` 与 `rev8`
两个旁置文件**逐字节相同**——那是两次 `cp -p` 之间没有发生任何编辑，属于**同一世代的两次快照**，不是两个世代。
清单当时按行数报 `28/28`，把一次测量计成了两次。这与 D083/D084（"一行不等于一次独立测量"）是同一类错误，
本轮第二次犯。已把重复行移除，并在生成器内加入**孪生守卫**：两条被枚举的行若哈希相同即**拒绝写出**
（实测：故意插入一条重复行，rc=1，`REFUSING TO WRITE: 1 enumerated identities are byte-identical twins`）。
同时把清单自报的数改为"行数 + 不同哈希数"两个数、且**同属一个全集**——否则"31 行 / 30 个不同哈希"
会被读成"仍有一对孪生"，而那只是两个数覆盖的范围不同。现为 `31/31`，`31 DISTINCT`。

**(f) 我转录了清单哈希，并做了一次边界划错的"部分撤回"（D136 / 追加 268、269）。** 追加 267 结尾把复核清单的
sha256 逐字写进了记录——**这正是 D131 要终止、D132 已更正的做法**，且是在同一份记录里写下"这正是缺陷所在"
之后又犯的。追加 268 撤回该值，但同时写"锚与普查的哈希不作废"：**边界划错了**。锚、普查与清单由生成器
**同一步**产出，清单重生成时三者一起重生成，故 267 转录的四个哈希（清单/锚/普查/索引）**全部**过时
（索引只是因为期间未再编辑而侥幸相符）。追加 269 更正。

**规则（D136）**：撤回一个值前先问"这个值是谁在何时产出的"——**同一步产出的多个值是一个原子身份组，要么全撤要么全不撤**。
部分撤回**比不撤回更危险**，因为它读起来像已经查清，实际留下过时值被读者当作仍然有效。
现记录（含本文件）经程序扫描，**不含任何活工件的哈希**（清单/锚/普查/索引/各旁置世代）。

**(g) 复核清单的表头声明它自己做不到的事（D137 / 追加 270）。** 清单表头一直写着"绝对路径已打印，
故可按原文复跑该检查"，但**行内只打印标签、sha256 与字节数，从不打印路径**——该声明从未被验证过，
即一条**永远为假的字面量**（"检查里的字面量是检查的一部分"）。已修复：每行现追加绝对路径，
并按**读者视角**独立复跑一遍（不借助生成器内部）：解析出 35 行，逐行重算，`35/35` 复现、`35` 个不同哈希、无孪生。
顺带记下一次**我自己的复核脚本**的同类错误：第一版正则把两行**静默丢弃**（只解析到 33 行），
因为我把"未解析行"当成不存在；修正后按"打印出所有未解析行"的方式复核，才看到 35 行。
