# 当前状态：P2 Final Gate（已关闭）/ P3（已关闭）/ P4（已关闭，P5 待授权）

2026-10-17（gen 12）。本文件提供当前入口，历史原文保留在 `status.md` / `decisions.md`。
前十代均逐字节保留、均可复算（权威清单 `EV/resume-04/lead/CURRENT-STATE-MANIFEST.json`）：
gen 1–9 快照在 `EV/resume-04/lead/current-state.gen<N>-pre-*.md`
（`4307866895…`/6,112 B、`533b0b23…`/14,473 B、`008105d5…`/16,716 B、`fda6a40c…`/18,674 B、
`d53d279d…`/19,496 B、`5a60ef88…`/20,532 B、`a8538bec…`/22,158 B、`d3a3c37c…`/23,122 B、`854069e7…`/24,222 B），
gen 10 快照在 P3 证据树 `EV/p3-20261016/lead/current-state.gen10-pre-p3-update.md`（`e2a877be…`/25,050 B）；
gen 11 快照在 P4 证据树 `EV/p4-20261016/lead/current-state.gen11-pre-p4-update.md`（`2549eba5…`/35,696 B）。

**一处历史缺陷（本轮登记，不改写历史）**：本文件表头自 gen 6 起一直写着"gen 6"，而清单已在 gen 7–10
逐代前移——表头是**手写标签**，从未随代际更新，连续四代与清单不符（正文 §10(g) 却已在记 D137）。
表头自 gen 11 起改为随代际更新（gen 11 → gen 12 本轮已同步）。清单与快照本身自始正确，故这不是身份缺陷，而是**标签漂移**。

**结论：P2 有界接受（SCOPED APPROVE，限固定 Golden F32 VADD 的 Late→A5 Binary 编译范围）；
P3-A/B/C/D 全部门禁通过，P3 独立 Binary 的实机 E2E 已由 Verifier 独立验收、Challenger APPROVE。
P4 已授权并**完成**（§12）；P5 门 READY 但未授权，本轮停在 P5 门禁。**

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

> **本节为 P2 轮次的历史记录，原文不改。** 其中"硬件数值正确性仍为 NOT_RUN"一句**已被 P3 取代**：
> 截至 gen 11，固定 Golden F32 VADD 的设备数值正确性为 **PASS**（§7）。
> 上句的适用范围是 **P2 轮次结束时的状态**，不是本文件的当前结论。

## 2. 仓库、工具与身份

HEAD：IR `82f3ad088ab3cea0e81fa6627e0487a93e5317ef`；TileLang `cc56643de04a49db6f50875e26d904b0c69e71a6`；
control 起点 `3cc7646b52a6fff9bfec141a784839c899db72b6`。IR 相对 staged-source `4495ab29b` 仅两个测试文件
+295，生产代码零差异（本轮独立复核：`EV/resume-04/lead/LEAD-SOURCE-SCOPE-AUDIT.txt`）。
IR 的 `origin` 查询超时，远端一致性 UNVERIFIED，未改远端配置。
served-model 审计仍 UNVERIFIED（未找到直接 Magpie 请求日志），见
`EV/resume-01/lead/model-audit-limitation.json`。

**P3 轮次（gen 11）新增/前移的身份**：

| 客体 | SHA-256 | bytes |
|---|---|---:|
| IR HEAD（P3-0 提交后，当前） | `bfefe18011152cca671e7e6ddf03793afbacaa2a`（commit id，非内容哈希） | — |
| control 仓 HEAD（P3 收官时，未再提交） | `b8ec55eef28d805b6845e590db571fd1b8f27e52`（commit id；其父 `3cc7646`，提交信息为 "update P2 docs"——**不是** P3 提交） | — |
| `p3/` 交付目录 | **未跟踪**（`git ls-files p3/` 为空）→ 无 git 身份，故 harness 以**内容哈希**绑定 | — |
| P3 Harness 源码 `p3/p3_harness.cc`（**未跟踪**，故无 git 身份，以内容哈希绑定） | 26b01e381bb5b2bdd595cb67c772868b1218ef9d191d3f17f2a2cba2e689fd4c | 103817 |
| P3 Harness 二进制（r9，已 staged） | e2298095b5f51d576165ab62010f57dc44e90c1328119fc2a0495c767995e24d | 192752 |
| `p3/p3_frozen_constants.h` | bab59c83dab56975e94e29b3e332dbf70b7eb24ad62d468adbee5c1581eebf7f | 3264 |
| 冻结验收标准 `P3-ACCEPTANCE-CRITERIA.txt`（Verifier 冻结，未改） | 6a321d7150b43ab6f18727c82c0d6c8d0abcc99afe067255b9aa446e8290d656 | 26678 |
| 独立 Oracle `p3_oracle.py`（Verifier，harness 只消费不重算） | 32d48852b1c69a31e479652809d19aad0ef59dac72b04242a6fa8134b0008a06 | 16168 |
| 正式 `libascendcl.so`（实机链接客体） | 4b270e626aad5d6a1e2b5444c60203185cf4684e06843573afb78e539b385fb2 | 259976 |
| Verifier P3-D 独立报告（现行） | 5044021d0e7a81bb698b697c1e0f0883199dd850613afc4a2fccb9f48c35df3e | 23741 |
| └ 其 gen1（4 行外观差异，已取代） | b63857c2e54561def89b9437e24473b8a92f66c2281d9772472e90db95117c59 | 23741 |
| Challenger P3-D 终审 verdict（verbatim 落盘） | e95bd0a1da4a3df174beeb414828f606b889cb26b521148a0903a40f186f7b47 | 9281 |
| 运行目录 `report.jsonl` / `acl.log` | d49158328b37172991ec3376d051f53725845e8a3687bfca62ff1017e4cebf06 / e47b481bec5da4075d3c82a5b3246934f927d4d868bf3b2de84f5c2b43aa6eb4 | 150522 / 29582 |

上表全部为**实测值**（本次由 Python 逐字节计算），非转录。P3 轮次未 push、未改远端、未改冻结工件。

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

> **§3 为 P2 轮次的历史记录，原文不改。** 其编号 3/4 项即 D128 审查范围的最终内容；
> P3-0 已把该修复**提交持久化**为 `bfefe18011152cca671e7e6ddf03793afbacaa2a`（§7 表第 1 行），
> 提交后内容哈希不变。§3 中"留在工作区、未提交"的表述属于 P2 轮次当时的状态。

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

## 7. P3 独立 ACL 实机 E2E — 结果

设计见 [独立 ACL Harness 设计](p3-acl-harness-design.md)。唯一设备程序为上表 `kernel.o`：
ELF64 little-endian、ET_EXEC、machine `0x1029`、flags `0x990000`、导出 `main_kernel`（GLOBAL FUNC）；
HostArgs = A/B/C 三个 64-bit GM 地址、24 bytes；grid=64；dynamic UB=196608 bytes；N=524288；
UB 基址 0/65536/131072。**该身份在本轮实机运行中再次实测 MATCH（goal §4 钉值）。**

| 子门禁 | 状态 | 判据（实测） |
|---|---|---|
| P3-0 源码封存 | 完成 | `bfefe18011152cca671e7e6ddf03793afbacaa2a`；提交仅两文件；提交后内容哈希不变 |
| P3-A Loader/Lookup | **PASS** | `aclrtBinaryLoadFromData`(4448 B, sha==kernel.o) rc=0；`aclrtBinaryGetFunction("main_kernel")` rc=0；释放链可解释 |
| P3-B Launch/Sync | **PASS（E1 配置）** | launch_rc=0、sync_rc=0；grid=64、DYN_UBUF_SIZE=196608 生效；**E1 = 仅 `--loader-magic=vector-core`**，非 E3 |
| P3-C 全量数值 | **PASS** | 15/15 行（5 case × 3 次）逐行 PASS；每行 compared=524288、mismatches=0 |
| P3-D 独立终审 | **APPROVE** | Verifier 独立报告 + Challenger 终审，两者均落盘（见下） |

**运行目录（只写一次）**：`EV/p3-20261016/verify/p3c-run-20261015T190259Z/`
（`EV` = `/home/s00653124/workspace/hivm-poc-evidence`）。墙钟 18 s（`19:18:29Z`→`19:18:47Z`），
容器 rc=0，`stderr.txt` 0 字节。E1 是该轮**唯一**的设备调用（`command.txt` 逐字记录，含为何不经 `run_stage.sh`）。

**数值与内存保护结果**：524288 元素/行 × 15 行；mismatches=0、sentinel 残留=0、NaN/Inf=0、
max_abs_err=0、64 个 block 失配计数全 0；A/B 回读未修改；六个 guard 区未破坏；
1,920 个持久化边界点（15×128）设备位 == 冻结参考位。`report.jsonl` 15 行运行记录的
`preservation_measured`/`guards_measured`/`sentinel_residue_measured` **全部为 true**（未测量的字段写 null，不写 0）。

**资源释放**：`acl.log` 139 条记录 rc 全 0，含完整释放链
（3× Free、BinaryUnLoad、DestroyStream、DestroyContext、ResetDevice、aclFinalize），
故冻结标准 `:279-284` 的"释放可解释"是**测量**而非断言。

**设备卫生**：仅用 device 0（Ascend950PR，前后 health OK、无进程）；device 1 是他人作业，全程未触碰。
device 0 的 HBM 前后有 5 MB 漂移——**本文件不对其作因果归因**（见 §11）。

**未独立测得的 7 项（Verifier 显式列出，不得读作已证）**：其中最要紧的两项是
(1)"NPU 确实执行了 kernel"（launch_rc=0 + sync_rc=0 + C 与参考一致，**无法**排除一个在回拷时伪造 C 的运行时；
信任边界在 CANN 与硅片）与 (2)"每行全部 524288 个元素在设备上一致"（设备 C 字节未落盘，
进程内比较后只留下失配计数与 128 个边界样本；其余元素依赖 harness 的比较代码，不可由保留字节重推）。
其余为：A/B 设备侧未修改（仅回读比较）、窗口内无并发干扰（仅前后采样）、运行中无 ECC 错误（仅前后采样）、
E1 相对 E0/E2/E4/E5 的判别（由 Implementor 的 experiment-r4 测得，本轮未重跑反例臂）、
运行内核处的 ABI HostArgs 布局（24/0/8/16 为自回显行）。

**P4 前提**：P4 必须经 `ExtractFuncInfo`/`AscendModuleCreate`/`launch_param_tags`/`aclrtLaunchKernelWithHostArgs`
自动传播 launch 配置；不得假定运行时消费 `hacc.block_dim`。
**P3 只证明独立 Binary 在真实硬件上按可观测契约正确运行；不证明 TileLang Runtime 已接通，也不证明 TIR → HIVM DeviceCodeGen 已实现。**

## 8. 无需重建原 Binary 的证据

本轮唯一的 P2 新缺陷都在 Late 输入验证器（只读 preflight）内部，且 §6.4 已实测：
冻结 fixture 经 pre-fix 与 post-fix 编译器产出的 A5 产物逐字节相同（`2c4247c3…`）。
因此 **P2 原设备 Binary 无需重建**，P3 继续使用冻结的 `kernel.o`（`2c4247c3…`/4448 B）作为交接客体。
新二进制 `9ea6f21b…`/`2206d8c5…` 只承担"验证器能拒绝四类非法输入"的新证据，不改变冻结产物。

## 9. 待办与门禁

- P2：有界接受（SCOPED APPROVE）。不再有待修的 P2 缺陷。
- P3：**全部门禁通过（A/B/C PASS，D APPROVE），P3 标记为 CLOSED。** 详见 §7。
- **P4：已授权并 CLOSED（§12）。** 设备验收 P4-C 17/17 PASS（device 0）；终审唯一 REVISE 经 R1/R2
  修复、独立验证（PASS-WITH-CAVEATS）、独立复评**已解除**（RESOLVED-WITH-CAVEATS）。
- **P5：READY，但未授权。** 正式 TIR → HIVM DeviceCodeGen 本轮**禁止**；授权前不得开始。
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

## 11. P3 轮次新登记的事件（含本文件自身的一处越界）

**(a) 本文件的 gen 11 改写使 P2 的身份普查清单失效（登记，不修复）。** P2 的
`EV/resume-04/lead/IDENTITY-SWEEP-FINAL-gen3.txt` 把 `docs/current-state.md` 列为**一行**
（gen 10，`e2a877be…`/25,050 B）。本文件被就地改写后该行**必然 FAIL**——这是 P2 清单里
**唯一一行活工件**，其余 36 行都是冻结客体。两条纪律在此冲突：
P2 清单要求"任何被枚举工件在清单之后再被写入，清单即 STALE，必须重生成"，
而 goal §9 明确要求"不要重新生成 P2 的复杂历史清单，所有冻结工件保持不变"。
**本轮的裁定：服从 goal §9，不重跑 P2 生成器**；清单按设计陈旧（"living sidecar sealed by hash
fails the previous manifest by design"），已在 P3 证据树留锚，读者不会把它读成篡改：
`EV/p3-20261016/lead/P3-IDENTITY-SUPERSESSION.txt`（绑定 gen 10 快照与 gen 11 现行件的 sha256+bytes）。
**未改动**：gen 1–9 快照、P2 索引、锚、普查——P2 的 37 行中只有 current-state 这一行随设计失效。

**(b) 我**没有**重跑 P2 的生成器，也就**没有**用它去复核 gen 10 快照——该快照的复算是**另做**的、
只用了 `sha256sum` 与 `cmp`（不借助任何 P2 脚本）：与改写前实测值逐字节相同，`cmp` 退出 0，
且两者 inode 不同（`stat -c '%h %i'` 分别为 1/77207846 与 1/108901177），故不是同一 inode 的两个名字。
"P2 生成器认可它"这句话本轮**不成立、也未写出**。

**(c) 一处未做因果归因的观测。** device 0 的 HBM 前后为 6005→6010 MB。**本文件不对这 5 MB 作因果说明**，
也不声称"无泄漏"：18 s 窗口内的采样点只有前后两个，中间态未采样；同一窗口内 device 1（他人作业）
的 HBM 由 6745 升到 39445 MB——把 device 0 的位移归因于本进程或归因于他人都是**超出证据的推断**。
可断言者仅：前后 health 均为 OK，且本进程的释放链 rc 全 0。

**(d) "行数即测量数"在 P3 轮次核对：15 行是 15 次测量，但**并非** 15 份互异数据。**
P3-C 的 15 行 = 5 个 case × 3 次运行，5 个 case 的 (A,B,C_ref) 三元组**两两不同**（实测 5 个不同三元组），
故 15 行取 15 个不同的 (case, run) 组合、无孪生行，**行数即测量数**。
但把"互异"说成 15 份**载荷**是错的：15 个载荷文件实测只有 **11 个不同 sha256**，因为
`constants.0` 是 A=B=C_ref=**全零**、`constants.1` 是 A=0 而 B=C_ref=3.25——
其中 A==C_ref、B==C_ref 这两处相等是 **VADD 的正确语义**（0+0=0；0+3.25=3.25），不是缺陷。
**本条是我本轮自己犯的错误**：初稿写成"15 个 oracle 载荷两两不同"，该句**已删**（见 (e)）。
故本文件此前的"15 行 = 15 次不同测量"表述应收紧为：
**5 个互异 case 各 3 次运行 = 15 次测量；载荷层面只有 11 个不同摘要。**
P3 证据树内**不存在** P2 那种"同一世代 `cp -p` 两次"的孪生。

**(e) 上述错误发生在**本文件已写成之后**，故按"改写须逐字记录"处理（D134）。**
(e) 与 (d) 的后半段是**同一次改写**加入的：我把 gen 11 的初稿
（快照 `EV/p3-20261016/lead/current-state.gen11-pre-oracle-count-fix.md`，`e47fb4aa…`/33,326 B）
在 §11(d) 处**就地改写**，替换为上面这段。改写是**局部替换**，其余字节不变；
被替换的旧句逐字为：
  "15 行取 15 个不同 `case`×`run` 组合，无孪生行；本轮 15 个 oracle 载荷 sha256 两两不同（已实测），
   故 15 不是把一次测量算成两次。"
**该句中的"两两不同（已实测）"是假的**，且更糟的是它**自称已实测**——实际是**未测量就断言**，
正是本项目反复登记的那一类错误（"检查里的字面量是检查的一部分"）。改写后不再声称载荷两两不同。

**改写可复算（已实测）**：把上面逐字记录的旧句施加于快照，得到 `snap[:i] + NEW`，
其 sha256 与**当前文件**逐字节相同（`94fb8a98418bbea5…`），且替换点之前的 20,263 字节
在快照与现行件之间**逐字节一致**。故这一代虽为"未先快照即改写"的**反向**情形
（快照已做、替换亦已逐字记录），其可恢复性由**替换内容被逐字记录**保证，与 (a)/(d) 的判据一致。
（`constants.0` 的 A=B=C_ref=全零与 `constants.1` 的 A=0、B=C_ref=3.25 都是 VADD 的正确语义，
不是缺陷；此处登记的是**我自己的计数/措辞错误**，不是产物缺陷。）

## 12. P4 轮次：正常 TileLang Host → HIVM Backend → TVM-FFI Runtime → 冻结 P3 Binary → 真机 device 0

**授权**：`docs/p4-goal-1016.md`（13148 B，pin `bbf7902bacfa0e68a35be5af8544887f1deef6afd94e25424307044fdffaac0a`）。
**目标（§8 完成标准）**：正常 TileLang 编译入口选择 HIVM Backend，经 Host CodeGen、FunctionInfo 和
TVM-FFI Runtime，使用冻结的 P3 AIV Binary 在真实 NPU 上得到完整正确结果；**不存在** AscendC/PTO 或
人工 ACL Launch 回退。**P5/P6（正式 TIR→HIVM DeviceCodeGen）本轮禁止。**

**设备程序（冻结，goal §3）**：`kernel.o` `2c4247c3…`/4448 B、符号 `main_kernel`、A/B/C 三个 GM 指针、
grid=64、dynamic UB=196608 bytes。**任何 Binary SHA 不匹配必须拒绝。**

### 12.1 四个子阶段与独立裁决

| 子阶段 | 内容 | 独立验证 | 裁决 |
|---|---|---|---|
| P4-A | HIVM Backend 路由（与 AscendC/PTO 互斥） | RUN-A/B + CR-RT-14 | 路由半 **YES**（CR-RT-14 对 R01..R05 的 `candidates` 字段豁免；其余 MEAS 字段逐字节相同） |
| P4-B | PREBUILT 注入 + Runtime 集成（CR-ID/REJ/PB/CACHE/FI/LD/SER） | verify-4（verify-3 死于上下文溢出） | **PASS-WITH-CAVEATS**，七组全 PASS，**无生产缺陷** |
| P4-C | 真机 E2E 验收（5 族 × 3 run = 15 次，device 0） | verify-5 | **OVERALL PASS**，17 行 PASS / 0 FAIL / 0 NOT_RUN，harness exit 0 |
| P4-D | 负例 N-1..N-9 + 路由负例 + 旧 Backend 回归 | verify-6 | **PASS-WITH-BLOCKED-REGRESSION-ROWS**，276 执行行 PASS / 0 FAIL，3 行 BLOCKED |

**P4-C 关键实测**（`verify/RUN-C/P4-C-VERDICT.txt`，`f0bee8aa…`/10980 B）：
字面调用 `tilelang.compile(vector_add(524288, mode="simd"), target="hivm", out_idx=-1)`；
解析到 `hivm` backend（`cache_reusable=False`）；`target.build.tilelang_ascend*`/`tilelang_pto*` 计数**全 0**；
import payload sha == 冻结 kernel.o；`aclrtBinaryLoadFromData` **恰 1 次**（`size=4448 numOpt=1 type=2
magic=0x41415246`，VECTOR_CORE）；15 次 launch 全 `numBlocks=64 argsSize=24 dynUBuf=196608`；
`FunctionInfo.name=main_kernel`、3 指针参数、`launch_param_tags=['blockIdx.x','tirx.use_dyn_shared_memory']`；
15/15 `compared=524288 mismatch=0 dev_nonfinite=0 ok=True`；oracle `sha(pre)==(post)==pin`。

**Lead 逐字节独立复核**（`lead/P4C-LEAD-CORROBORATION.txt` `9f91a40a…`/7913 B；
`lead/P4D-LEAD-CORROBORATION.txt` `b0bb0577…`/3473 B + ADDENDUM-1 `7cbcc4ca…`/2336 B）：
脚本绑定复现、冻结身份重测、5/5 oracle ref 自报相符、行首锚定轨迹 `^LOAD=1 ^LAUNCH=15 ^UNLOAD=1`、
末行 `TRACER_EXIT n_load=1 n_launch=15 n_real_fail=0 real_ok=1`、**device 1 前后皆空**（从未触碰）。

**P4-D 关键实测**（`verify/RUN-D/P4-D-VERDICT.txt`，`f85b4398…`/31864 B）：九条负例全部按名拒绝或
由 Stub 观测；错误 magic/故障类负例**全部**经 `acl_mock.so`（身份 `p4b-acl-mock-v1`）拦截，**未**在共享设备
制造故障；CR-REG-1 证明 `rt_mod_ascend.cc` 唯一被删行是注册续行 `BuildTileLangAscendWithoutCompile);`，
三个旧入口函数体逐字节等于 HEAD；CR-REG-3 的 13 个冻结 subject 全部复现、P3 树 ctime 普查零 offender。

### 12.2 生产源码修改（仅 TileLang；AscendNPU-IR 不变）

TileLang HEAD `cc56643de04a49db6f50875e26d904b0c69e71a6`，工作区 16 个改动文件（11 M + 5 新），
`434 insertions(+), 43 deletions(-)`（终审前为 `403/43`；R1 守卫方法 + R2 注释块计 +31 行）。
生产面：`src/ascend/codegen/rt_mod_ascend.cc`（PREBUILT 入口）、
`src/ascend/runtime/ascend_module.cc`（VECTOR_CORE 加载配置 + ABI 静态断言）、
`tilelang/ascend/{target,backend,codegen,execution_backend}.py`、`tilelang/backend/module.py`、
`tilelang/cache/{__init__,kernel_cache}.py`、`tilelang/jit/__init__.py`；
新增头文件 `src/ascend/codegen/{hivm_prebuilt_contract.h,sha256.h}`、
`src/ascend/hivm_prebuilt_identity.h`、`src/ascend/runtime/acl_rt_types.h`。
**AscendNPU-IR HEAD `bfefe18011152cca671e7e6ddf03793afbacaa2a`，worktree clean，本轮零改动。**
`tilelang-mlir-ascend` 保持只读。

### 12.3 门禁

闸门记录 `lead/P4-GATE-TRANSITIONS.txt`（G1..G49，append-only）。**P4-C 闸门 = G47**
（`1eec630a8564e0dab61f45aff8dc82788ddc76d3459b542f91191a47d0842c17`/183418 B）。
**P4-D 闸门 = G45**（`b53dda04d67e304fccdfe7acca987b4f9649b92ac1153724c89187c09216f64e`）+ **G46** 补录。
**Challenger 终审 = G48**（`c46f88b8a9540ca112b93f6b2d94495674c88e76929aa6e29d40fb855ac9637b`/185150 B，
裁决 **REVISE**）。**G49 已落闸**：R1 修复 + R2 再加固经独立验证（`bbd0da7d…`/17264 B）、
REVISE 经独立复评**已解除**（`bf9a5451…`/10994 B）；闸门文件 `c46f88b8…`/185150 B → **`e51aaef1e20ebfa0ad005b681ba454377aa7037522168d87715baee248ad9664`/191558 B**
（前缀逐字节保持；追加 293/294 记录）。

### 12.4 终审 REVISE 与处置（**已解除 → P4 关闭**）

Challenger 终审（`challenger/CHALLENGER-P4-FINAL-VERDICT.txt`，`5b8ef6c5…`/3939 B，由 Lead 逐字落盘）
九项确认 **1–7、9 = CONFIRMED**；**第 8 项 CONFIRMED 但独立的 cache-identity 义务失败**；
**OVERALL = REVISE**。唯一必修：`tilelang/jit/__init__.py:519` 的**默认 target JIT cache 决策**——
装饰器默认 `target=None` 时守卫调用 `create_backend_context(None)` 抛错、被裸 `except` 吞掉返回 `True`（fail-open），
而编译会从 `TILELANG_DEFAULT_TARGET` 解析 `None`；故 `TILELANG_DEFAULT_TARGET=hivm` 可选中 HIVM 而**外层 JIT cache
直接返回旧 kernel、不再走 binary 准入**。

**Lead 独立复现**（`lead/scratch/r1-guard/lead_r1_probe.py`，**不同仪器**：拦截 `create_backend_context` 记录
每次解析到的 backend）：默认装饰器 + `hivm` → 守卫 `True`（缺陷）；显式 `target="hivm"` → `False`；
并证明 **P4-C 验收路径不受影响**——`tilelang.compile` 是**模块级** `jit.compile`（`:94`），**不**经 JITImpl 守卫，
`None` 由 `_resolve_cache_dispatch`（`cache/__init__.py:53`）正确解析。故 **P4-C 裁决不受此缺陷影响**。

**R1 修复**（`impl-r1/`，仅 `tilelang/jit/__init__.py`）：守卫在问 backend **之前**先按编译同样方式解析 `None`
（`env.get_default_target()`）。复测：翻转成立，且**守卫解析与编译解析一致**（皆 → backend `hivm`，reuse=False）；
`auto`/未设仍 `True`。

**R2 再加固**（`impl-r2/`，同文件）：Lead 发现 R1 的 diff **新增**了一条**假注释**
（`no per-call cost is paid on reusable backends`；实测 reusable 路径 **64.17 µs/call**，HEAD 此路径为 **0**）并引入
逐调用开销；且实测 `determine_target('auto')` **依赖环境上下文**（`Target.current()`：无上下文→ascend，
`with Target('llvm')`→llvm），故**朴素 memo key 会重新引入 fail-open**。R2 更正注释并在安全前提下消除该开销。
见 `lead/FINDING-R1-COST-AND-FALSE-COMMENT.txt`、`lead/FINDING-guard-context-dependence.txt`。

**独立验证**：`verify/RUN-R1/`（工作单 `lead/VERIFIER-WORK-ORDER-R1R2.txt`）。VERDICT: PASS-WITH-CAVEATS (verify/RUN-R1/R1R2-VERDICT.txt bbd0da7d924d8e32c25222dedd03e8bc6089ea82cb3d25e375cdd805265026bb/17264 B; 6 items + 6 caveats).
**独立复评**（工作单 `lead/CHALLENGER-RE-REVIEW-WORK-ORDER.txt`，只读、不跑设备）：**必修项 RESOLVED**，
R1/R2 增量**未引入新的可执行缺陷**；最小判别实验为「在内存中只删去新增的 None→env 解析分支 → 旧 fail-open 复现」。
复评另**证伪**验证器 C4（环境上下文守卫翻转"不可构造"）：合法对象 `determine_target("hivm", return_object=True)`
下守卫**确实翻转** `True→False→True`（Lead 已独立复现）。RE-REVIEW VERDICT: RESOLVED-WITH-CAVEATS (challenger/CHALLENGER-P4-REREVIEW-VERDICT.txt bf9a5451ad7f934d4aef061d79fd1c166a426fa08fca2bc0d5b96153f22c480b/10994 B; the required revision is RESOLVED; C1..C4 caveats in the verdict).
**故 P4 = CLOSED**（设备验收 P4-C 自始不在该缺陷路径上：模块级 `tilelang.compile` 不经 JITImpl 守卫）。

### 12.5 P4 的诚实性边界（不得读作已证）

- **PREBUILT，非 DeviceCodeGen E2E**：本轮设备程序是**冻结的 P3 Binary**；**没有**运行 AscendC 源码
  CodeGen、PTO CodeGen 或 HIVM TIR→Late MLIR 生成。P4 证明的是**运行时能消费**一个预构建 binary，
  **不**证明 P5 的 TIR→Binary 生成。
- **15 行 ≠ 15 份互异载荷**：5 个 case × 3 run；每族内 3 次运行是**同一输入**（oracle 设计如此），
  15 行落在 **5 个互异载荷**上。
- **设备-对-参考的相等**由 harness 的**进程内**比较承载（设备字节**未落盘**），故**无**独立的设备 dump
  字节 diff；Lead 的 A+B 重算佐证的是 **oracle 自洽**，不是设备。
- **P4-D 一条设备 Runtime E2E 行 BLOCKED**（按 goal §5.3.8 只用 Mock）；**PTO 两条 BASELINE_BLOCKED**
  （缺 `ptodsl`，非 P4 回归：PTO 入口 `rt_mod_pto.cc` **不在** P4 差异内）。
- **R-4 更正（原记录为「已知缺口」，经实测为误）**：`tl.hivm_prebuilt_binary_path` 覆盖**经
  `pass_configs=` 贯通** `tilelang.compile()`（`compile(..., pass_configs={"tl.hivm_prebuilt_binary_path":
  <坏拷贝>})` 在 sha256 处**拒绝**，路径不存在则在 open 处**拒绝**）；只有**外层** `tvm` `PassContext`
  不携带该覆盖（外层 context 在编译中被替换）。故「损坏 binary 负例」**可以**经生产编译入口驱动；
  P4-D 的直接驱动偏保守，但建立在一条**错误前提**上。见 `ser-repro/R4-FINDING.txt`。
- **序列化覆盖边界**（本轮 host-only 实测，非生产缺陷）：冻结 binary 的 sha256+size pin 在 **PREBUILT 入口**；
  从序列化字节**恢复**的 Module **不会重新 pin**，其重建的诚实标签**仍**打印冻结 sha。复现：翻转内嵌
  device code 的**一个字节**在恢复时**被接受**；恢复后的 Module **持有**被污染字节、其 code sha256 **≠** pin，
  但其标签**仍**打印冻结 sha 并仍以 VECTOR_CORE 到达 loader。裁定为**覆盖边界**而非生产缺陷：无生产路径
  产生被污染字节（编译入口总是 pin；kernel cache 因 `cache_reusable=False` 既不服务也不持久化 HIVM 条目）。
  见 `ser-repro/FINDING.txt`。**结转**：若 HIVM 后端将来变为 cache-reusable，恢复路径需自带完整性检查。
- **P4-A Challenger 复核结转**：item D（R15 checker 的谓词接受一条合成的 NOT_ROUTED 记录而无显式拒绝证据）
  仍 **OUTSTANDING**；runner `:454-456` 的 `startswith` 放宽是**有界**的未来 runner 收紧项。
- **device 0 后置外部进程** `218304`（host 侧同名，非本 run 所有）；G31 记为共享状态。
