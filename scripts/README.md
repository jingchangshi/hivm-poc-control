# scripts/ 说明（2026-10-28）

本目录的脚本**按原样保留**。其中 `in-container.sh` 的内容哈希
（`2ed26e9b9fc66f2376797ecbe4d21f5074cb162ab3f2a1cc6e55c74f9fead34f`）
已被历史记录与证据引用，**因此本轮不修改其内容**——改一个注释字节也会让那些引用变成陈旧身份。
（该哈希在历史证据树中确实被引用；本次未对全部引用位置作穷尽核验，故本文不给出计数。）

## `in-container.sh` — 历史执行包装器（仍可读，但不要当作当前环境说明）

- 它把 `docker exec` 包装成"进入容器执行一条命令"。
- 容器名**写死**为 `s00653124_tilelang`，该容器**现已 exited**（2026-10-28 实测）。
- 它只向容器转发 `HOME` 与 `HIVM_CONDA_ENV` 两个变量；其它宿主机环境变量**不会**进入容器
  （这是 D044 记录的、会导致"守卫无法触发"的真实缺陷，属已知边界，不是本轮要修的问题）。
- 用法形态（仅供理解历史命令）：

  ```sh
  HIVM_CONDA_ENV=<env> ./scripts/in-container.sh bash -c '<command>'
  ```

**当前替代做法**：先用 `docker ps` 与 AgentGates `doctor`（workspace `tilelang`，
adapter `tilelang-hivm` / `tilelang-hivm-npu`）确认当前运行中的容器与工具链，
再用直接的 `docker exec`（显式 `-e` 转发所需的每个变量）执行命令。
不要把容器名、conda 环境名写死到新的脚本或文档里。

## `check-golden-readonly.sh` — P0 只读校验（历史）

- 同时检查旧路径 `tests/tests/golden/hivm/vadd_f32_single_tile` 与规范路径
  `tests/golden/hivm/vadd_f32_single_tile`；旧路径**已不存在**，脚本对不存在的目录会跳过
  （`[ -e "$DIR" ] || continue`），因此仍可在当前树上运行。
- 它断言的冻结身份仍然有效：
  JSON `576fa7a37bcaf1748e387efbd26508652e2e4b18790fa21aea874a7b756c0fe5`、
  TIR `4194733b8412484c9bb8caf74ba3285348345f92ad61684784fe8c3a79e12a18`、
  manifest `ub_offsets_f32 = {a:0, b:16384, c:32768}`（元素偏移）。
- 末尾还会打印 TileLang 的 `git status --short` 与 `git diff --cached --name-status`，
  用于确认没有意外改动 Golden。
