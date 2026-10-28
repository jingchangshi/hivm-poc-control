# P5 终审记录（2026-10-28）

本文件记录 P5-FINAL-A 这一轮的实测结果与边界。**只陈述本轮实际执行过的命令与观测到的值**；
未执行的项目明确写 NOT_RUN。本轮结论是 TileLang 代码 **READY_FOR_P5_GATE**，不是 P5 CLOSED；
P5 关闭需要下一轮机器门禁和独立裁决，P6 未启动。

## 1. 源码身份

| 仓库 | HEAD | 工作区 |
|---|---|---|
| TileLang | `658e4cb762a492722771fd97cdb29a1e809f879d`（`feat/hivm-backend-poc`） | clean |
| AscendNPU-IR | `bfefe18011152cca671e7e6ddf03793afbacaa2a` | clean（0 改动） |
| 控制仓 | 本文件所在提交 | 见 `git log` |

P5 这一轮在 `6fe6f091` 之上产生 5 个提交，改动 8 个文件、+1672/−130：

```text
658e4cb7 [Ascend] Test backend routing, target agreement and TIR mutations
51f109fa [Ascend] Resolve one kernel symbol and state the provenance limits
7ca0cc13 [Ascend] Refuse TIR the HIVM emitter was silently reinterpreting
12b9c711 [Ascend] Make the HIVM compile hook check the target and the artifact it returns
78a778bd [Ascend] Bind the HIVM backend to the generated DeviceCodeGen
```

构建产物：`build/lib/libtilelang.so` sha256 `d240c0fd675ac4db9d1160956e5b8415beb5b3ed9010b89723a4e51624530f0c`。

## 2. R1–R4 的处置

| 项 | 处置 |
|---|---|
| R1 正式 Backend 绑定 | `HIVM_CODEGEN.build` 指向 `target.build.tilelang_hivm`；新增 source-only Module 回调（返回 Module，不是 String），`compile_device=False` 走它。 |
| R2 Hook 与 Target 一致性 | hook 使用 Target；arch/mcpu 都读且必须一致；`HIVM_TARGET` 是断言不是覆盖；能力探测要求 `--help` 成功且声明 late-hivm；输出必须是实测 AIV 形态 ELF。 |
| R3 TIR 语义变异 | 三个静默误编译已关闭（VF 常量地址、vadd 掩码、VF step），合法近邻保留；见 §4。 |
| R4 产物来源 | 单一符号解析供 emitter/FunctionInfo/label；GENERATED token 独立于 PREBUILT；标签明写不记录编译器身份。 |

另修一个只有真实编译器才能发现的缺陷：`Var::defined()` 被当作“已发射”标志，导致
`get_block_idx` 定义行被跳过而 3 处使用照写，真实编译器报 undeclared SSA value。

## 3. 测试结果

命令（容器 `s00653124-tilelang-hivm`，conda `s00653124_tilelang`，Python 3.11.17）：

```sh
export HIVM_COMPILER=/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline/impl/stage-p2c-r1/bin/bishengir-compile
python -m pytest testing/ascend/codegen/ -q
```

| 环境 | 结果 |
|---|---|
| 有真实 P2 编译器 | **53 passed, 0 skipped, 0 failed**（rc=0） |
| 无 `HIVM_COMPILER` | 48 passed, 5 skipped（skip 全是需编译器的 built fixture） |
| `testing/ascend/target/` | 16 passed |

日志：`/home/s00653124/workspace/tmp/logs/p5-regression-final.log`。

身份（实测）：

```text
compiler     812c5cb1d29cb3b8879593ea77d08ebdae274cc943d41523c7c4bab739cbf455  (205,469,200 B)
O2 template  3807eb3d907236b17855db57a475ea120c37a74905c92bb4b971b415da7d6f79
libtilelang  d240c0fd675ac4db9d1160956e5b8415beb5b3ed9010b89723a4e51624530f0c
```

skip 不计入 P5 PASS。

## 4. TIR 变异结果（source-only，最终构建上两轮复跑一致）

每例先构造、再经 `save_json/load_json` 往返并 `assert_structural_equal`，之后才计 codegen 结果。

| 例 | 结果 |
|---|---|
| A1 GM 因子换自由变量（防御探针，不计入有效计数） | REJECT：block factor 不是 thread_extent 绑定的 bx |
| A2 GM 因子换已绑定变量（for 包裹） | REJECT：thread_extent body 必须是 SeqStmt（结构性拒绝） |
| B1a/B1b vld、vsts 常量地址 | REJECT：constant address，必须携带 `i * 64` |
| B2 stride `i*64 → i*32` | REJECT：一次迭代移动 64 元素 |
| B3 VF `step=2` | REJECT：子集只支持 1 |
| C1 `vadd(a,b) → vadd(a,a)` | ACCEPT，MLIR 操作数变为 `%v0, %v0` |
| D1 vsts 值 `result → a_reg` | ACCEPT，masked_store 操作数变为 `%v0` |
| D2 vsts mask → 全 false | REJECT：期望已绑定向量值 |

边界：A1 引入自由变量，不作为有效语义案例；A2 在结构检查处被拒绝，**没有**证明更深的
变量身份分支被动态覆盖。

## 5. 真实 P2 链路（stage-p2c-r1 原位执行，未改动 IR 安装目录）

| 输入 | parse/verify + LateInputVerifier | 产物 |
|---|---|---|
| BASE | OK | 4448 B, `2c4247c3b2ce11f79ad15415eb06d4ce1d5523a0c19c91ad3743bc08e7b8c4c9`（与冻结 P3 逐字节相同） |
| C1 | OK | 4432 B, `1194ba9de52cb123e518af2bac970aafe2fef2e0bddab60e3d9dc9192331ec70` |
| D1 | OK | 4432 B, `d8275b36b2b42fa49dd7ce703a2b731675977b3a69aef347bc7793b8200d19fa` |

C1 与 D1 互为不同且均不等于冻结值：语义变化进入真实编译，没有身份短路。C1/D1 是合法但
语义不同的程序，下游接受属预期。

## 6. 未关闭项与已知边界

1. **P5 未 CLOSED**：本轮只做源码终审；机器门禁（AgentGates `tilelang-hivm` 的 build /
   regression）与最终独立裁决尚未在本轮执行。
2. **设备与数值正确性 NOT_RUN**：未使用 NPU；生成二进制未在硬件上执行。
3. **安装目录模板漂移**：`AscendNPU-IR-1/build/install` 中 O2 模板 sha256 为
   `fbc87c2557…`，而编译器内置期望 `3807eb3d…`，门禁按设计拒绝。本轮未覆盖该目录，
   改用证据树中 `stage-p2c-r1`（`sha256sum -c` 4/4 OK）原位执行。这是需要单独处理的
   环境问题。
4. **ELF 头不是设备识别**：只筛查实测 Ascend/AIV 形态（ET_EXEC、machine `0x1029`、
   flags `0x990000`）；现有源码没有把这些字段映射到具体型号，设备主张依赖编译器身份、
   argv 与 MLIR/binary 身份链。
5. **编译器身份不随产物记录**：Module 不保存编译器身份，标签明写
   `Compiler identity: NOT recorded here`；本轮身份由独立测量记录。
6. **source-only Module 不能经设备 loader 恢复**：`CSourceModule` 序列化源码流，而
   `AscendModuleLoadFromBytes` 读三段结构，普通 AscendC 的 `asc` 同样失败，属既有边界。
7. **恢复后的 GENERATED Module source map 为空**，默认 `inspect_source()` 返回空串；
   token 证据取自对恢复模块再次序列化的字节。
8. **A2 竞态**：A2 的 for-wrap 形态不宣称整体程序无竞态；它只用于证明该结构被明确拒绝。
9. **旧后端覆盖口径**：AscendC/PTO/PREBUILT 三项回归是 direct FFI 入口执行，不是
   resolver→context 的完整调用链；HIVM 的 context/engine 路径由专门测试覆盖。
10. **未执行**：45 项既有 BiSheng 失败未在本轮重跑，仅按已存基线记录，未修复无关测试。

## 7. 下一轮建议

- 用 AgentGates 对当前 HEAD 跑 `tilelang-hivm-build` 与 `tilelang-hivm-regression`，
  以 `verify_requirements` 明确这些 gate 是否满足当前源码状态。
- 单独处理 `build/install` 的模板身份漂移，避免下次真实编译再次依赖历史 bundle。
- 若要把编译器身份纳入产物，需要在 build 边界显式传递并记录，而不是在 codegen 内推断。
