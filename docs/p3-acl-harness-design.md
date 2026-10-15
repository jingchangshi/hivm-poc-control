# P3 独立 ACL Harness 设计与交接

日期：2026-10-15。任务边界：本文件是可实施设计；本轮没有实现、构建或执行 P3。P3 的准入结论以 `current-state.md` 和本轮独立 Challenger verdict 为准。P3 成功仍须实机数值证据。

## 1. 固定输入与身份

设备程序唯一来源：
`/home/s00653124/workspace/hivm-poc-evidence/p2-20261013-late-pipeline/verify/work/run/A1/kernel.o`。
完整 SHA-256：`2c4247c3b2ce11f79ad15415eb06d4ce1d5523a0c19c91ad3743bc08e7b8c4c9`；4448 bytes。
每次运行先读取全部字节、计算 SHA-256，并把同一份内存交给 loader，避免检查后重新打开路径。禁止生成或替换设备程序。

| 项目 | 交接值及依据 |
|---|---|
| ELF | ELF64 little endian，ET_EXEC，machine `0x1029`，flags `0x990000`；本轮 `lead/kernel-readelf.txt` |
| 目标 | Ascend950PR_9579 / dav-c310 / AIV，P2 IR 与目标编码证据 |
| 导出入口 | `main_kernel`，GLOBAL FUNC；不能用 `$local` 或 VF helper 替代 |
| 参数 | A、B、C 三个 GM 指针，按此顺序，64-bit 地址，偏移 0/8/16，总计 24 bytes；P2 最终 IR 与 `__CCE_KernelArgSize` |
| 数据 | 每个数组 524288 个 float32，即 2097152 bytes |
| launch | numBlocks=64；dynamic UB=196608 bytes；每 block 8192 元素 |
| UB 几何 | 0/65536/131072 byte 基址；每段 DMA 32768 bytes；不是紧邻的三个 32768-byte buffer |
| 设备容量 | UB_SIZE=2031616 **bits**=253952 bytes；不得作为 launch 配置 |

## 2. CANN 9.2 Loader 兼容性边界

本轮读取实际容器头文件：`/data/pri/cann-9.2.0/cann/include/acl/acl_rt.h`，解析到 `cann-9.2.0-beta.2/x86_64-linux/include/acl/acl_rt.h`。
实际声明、类型与头文件身份已在本次恢复执行中重新测量，见 `p2-final-20261015/resume-01/lead/acl-design-recheck.json` 和 `acl-types-recheck.json`。
`aclrtBinaryLoadFromData` 接收 binary bytes、长度和可空 options；`aclrtBinaryGetFunction` 用导出名取 handle；`aclrtLaunchKernelWithHostArgs` 接收连续 host 参数区。
正式 `x86_64-linux/lib64/libascendcl.so` 导出 loader、lookup、launch、timeout sync 和 unload 五个所需 `aclrt*` 接口；同目录 `libruntime.so` 不导出这五个同名接口。以 `p2-final-20261015/resume-01/lead/acl-library-recheck.json` 的实际动态符号表及库身份为准。链接和运行必须使用该 **lib64**，不能选 devlib stub 或 simulator。

**已确认 API/符号可用，尚未执行该 ELF 的 loader。** 本地公开头文件没有承诺接受任意 machine=0x1029 的 ET_EXEC，因此 ELF 格式和符号检查不能冒充 loader 兼容性 PASS。P3 第一子门禁必须是：在选定 Ascend950PR 上加载上述同一份字节，并成功 lookup `main_kernel`；记录两个调用的真实 ACL rc。此步骤失败即记 `LOADER_BLOCKED`，停止 launch，保存完整错误、ELF 信息及日志，复核格式/导出/当前 runtime 支持。不得改用 AscendC/PTO 或旧 ELF。

初次调用使用 `options=nullptr`，与当前本地 TileLang `ascend_module.cc:408` 的接口用法一致；Harness 自身不链接或调用 TileLang。头文件提供 VECTOR_CORE magic option，但本轮没有证据要求它，不能把任意 magic 猜测作为成功前提；需要时另做可追溯的小实验，仍使用同一 ELF。

## 3. C++ 实现结构与准确 launch 参数

单独可执行程序，依赖 C++ 标准库、ACL 正式头文件和 `libascendcl`，使用独立 SHA-256 实现或系统加密库；不得依赖 TileLang CodeGen/Runtime、torch 或生成设备代码。使用 `<acl/acl.h>` 及实际安装的 `<acl/acl_rt.h>` 类型，避免复制 ABI struct。

建议函数拆分：`ReadAndVerifyBinary`、`CheckDevice`、`LoadKernel`、`MakeDataset`、`AllocateGuardedBuffers`、`RunCase`、`CompareAll`、`WriteReport`。每个 ACL 调用包装记录 operation、rc、即时 `aclGetRecentErrMsg()` 文本、UTC 和单调时间；保留首次失败，cleanup 错误不能覆盖它。

调用顺序：`aclInit(nullptr)` → `aclrtSetDevice(device_id)` → 创建专有 context、stream → loader/lookup → 分配并运行各输入集 → 完成同步 → 释放 → unload → destroy stream/context → 与本进程 set-device 配对的 reset → `aclFinalize`。只管理本进程创建的资源；不得操作其他用户的 context、任务或容器。具体设备编号在 P3 实施时按设备现场分配，记录产品信息，拒绝与 Ascend950PR 契约不符的设备。

以下为设计片段，未编译、未执行：

```cpp
struct HostArgs { void* a; void* b; void* c; };
static_assert(sizeof(void*) == 8);
static_assert(sizeof(HostArgs) == 24);
static_assert(offsetof(HostArgs, a) == 0);
static_assert(offsetof(HostArgs, b) == 8);
static_assert(offsetof(HostArgs, c) == 16);
HostArgs args{a_payload, b_payload, c_payload};
aclrtLaunchKernelAttr attrs[3]{};
attrs[0].id = ACL_RT_LAUNCH_KERNEL_ATTR_DYN_UBUF_SIZE;
attrs[0].value.dynUBufSize = 196608;
attrs[1].id = ACL_RT_LAUNCH_KERNEL_ATTR_ENGINE_TYPE;
attrs[1].value.engineType = ACL_RT_ENGINE_TYPE_AIV;
attrs[2].id = ACL_RT_LAUNCH_KERNEL_ATTR_TIMEOUT;
attrs[2].value.timeout = 10;  // seconds, local acl_rt.h:555
aclrtLaunchKernelCfg cfg{attrs, 3};
// Check every return code; packed args and binary bytes live through sync.
aclrtLaunchKernelWithHostArgs(fn, 64, stream, &cfg,
                              &args, sizeof(args), nullptr, 0);
aclrtSynchronizeStreamWithTimeout(stream, 30000);  // milliseconds
```

HostArgs 中存设备 payload 地址本身，不是 host 数组地址或 `void**` 参数指针数组。placeholder 数量为 0。保持 `args`、cfg、属性和 binary 字节存活到同步结束。显式 AIV engine 来自本地枚举，不改 ELF。若当前设备拒绝某 launch 属性，报告具体 rc，不能静默去掉 dynamic UB 或改 grid。

实现时使用环境已有 host C++ 编译器；先检查容器内路径和实际头文件。所有构建/执行均经 `scripts/in-container.sh`，独立 P3 输出目录，链接正式 CANN `x86_64-linux/lib64` 的 `ascendcl` 并记录最终动态依赖。建议开启 `-std=c++17 -Wall -Wextra -Werror`，禁止 fast-math。以上是未来构建要求，本轮没有声称已存在 Harness target 或通过构建。

## 4. 独立预期、数据与 guard

先冻结数据公式和比较规则，再写 Harness。i 为全局元素下标，b=i/8192，j=i%8192。所有公式均以整数及精确二进制分数构造 float32；不依赖生产 lowering。

| 输入集 | 公式 | 验证目的 |
|---|---|---|
| constants | 分别运行 A=0,B=0；A=0,B=3.25；A=-2.5,B=7.75 | 基本计算、非零写入与全零结果 |
| mixed | A=((i%511)-255)×2^e，e 依 i%3 取 -8/0/8；B=((i%509)-254)×2^f，f 依 (i/3)%3 取 8/0/-8；取模结果先转有符号整数再相减 | 正负、大小量级和抵消 |
| per-block | A=32×b+(j%17)/16；B=-b+(j%31)/32 | 64 个 block 各异，非零 block 索引、地址及边界 |

Host reference 对**已经转为 float32 的输入**先以 double 精确相加，再 round 到 float32；本设计数据的指数跨度/有效位范围使 double 足以精确容纳和。固定 round-to-nearest，比较所有有限结果的数值精确相等（允许 +0/-0 数值相等），不使用放宽容差掩盖错误。任何 NaN/Inf 输出失败。统计逐元素 mismatch 总数、首 16 处明细、最大绝对误差，各 block mismatch 数。A+B 的交换律不能证明指针排列；参数次序证据来自 ABI，不从数值结果倒推。

每个 A/B/C 独立分配 `payload_bytes + 2×guard_bytes`，guard_bytes=256，payload 从 allocation_base+256 开始，记录基址和实际对齐，若不满足 DMA 所需对齐则先诊断并停止。设备地址分别打包为 A/B/C payload。整个 A/B 分配 host 镜像含 guard，C payload 初始化为固定 quiet-NaN bit pattern `0x7fc12345`，guard 使用逐字节 `0xa5`，通过 memcpy 写入，不用 float 转换重写位模式。C 的全部预期结果均有限，未写 sentinel 必须失败。同步完成后读取**完整分配**；检查 A/B payload 位级未变、六个 guard 区逐字节未变、C 全部输出正确。guard 能覆盖该布局邻近越界，不能证明任意远距离越界绝不存在。

每个输入集至少重复 3 次，每次重新写入 C sentinel 和 guard，顺序执行。所有 block 两端 i=b×8192 与 (b+1)×8192-1 单列结果，明确包含 8191/8192、16383/16384 及末元素 524287；全量比较仍是主门禁，边界抽查不能替代它。重复通过可发现部分非确定性症状，不能宣称穷尽无 race。

## 5. 完成、超时、故障与资源

Host→device 初始化使用受检的同步 copy；kernel 异步启动后必须先 `aclrtSynchronizeStreamWithTimeout` 成功，再进行 D2H 和数值比较。launch rc=0 不代表执行成功。每个 case 记录 launch rc、sync rc、copy rc、元素检查数、sentinel/guard/输入保持结果；任一失败则整 case FAIL，未开始的 case 为 NOT_RUN。

kernel timeout 为 10 秒；stream 同步上限 30000 毫秒。P3 外层进程使用独立 wall-clock watchdog（例如 120 秒及短 kill grace），防止初始化、loader 或 cleanup 卡住；watchdog 超时为 TIMEOUT，绝非 PASS。同步失败/超时后禁止读取或比较 C；不得在 pending kernel 仍可能访问时释放其 buffer。保留首次错误，先尝试有界终止本进程自有 context；无法确认停止时结束进程并登记未完成的 cleanup，不无限等待，不擅自重置共享设备或干扰其他任务。正常路径逆序清理，每个 rc 单独记录。

P3 每次运行隔离目录保留：Harness 源/可执行文件/动态依赖身份、冻结 binary 身份、数据公式版本、精确命令、容器/conda/CANN/设备型号、开始结束 UTC、stdout/stderr、JSON 每 case 结果及最终 rc。在进程启动前将其 ACL/应用日志定向到该运行的专属目录（按现场 CANN 支持配置核对变量），保留本用户 `~/ascend/log` 中本次时段的 plog/device 日志；不要采集其他用户日志或环境凭据。若设备日志无访问权限，明确记录路径/权限与 LOG_UNAVAILABLE，不能声称已收集。ACL 错误的 recent message 必须在其他 ACL 调用前复制。

## 6. P3 和后续门禁

P3 成功要求：固定 SHA loader+lookup 成功、所有 launch/sync/copy 成功、全部 case 和重复的每个 524288 元素正确、所有 sentinel 消失、guard 和 A/B 保持、日志无未解释设备故障。Loader、launch、数值任一项未执行均不得报 P3 PASS。本轮仅完成设计与输入交接。

P4 必须经现有 `ExtractFuncInfo` → `AscendModuleCreate` → `launch_param_tags` → `aclrtLaunchKernelWithHostArgs` 自动传 grid/dynamic UB。当前最终 IR 的 `hacc.block_dim=64` 不是该运行时消费的载体；不能把 P3 的手动配置当成 P4 传播证明。`tirx.use_dyn_shared_memory` 标签须最后。FunctionInfo 的 launch/extra tags 在本 P2 ELF 中不可观测。

P5 从 Golden Prepared TIR 正式生成 Late IR，承担元素到 byte 的 UB 地址、pb32、cache4、VF outline/call、set/wait 和终结 PIPE_ALL；P6 才能证明 DeviceCodeGen E2E。本轮停在 P3 门禁。
