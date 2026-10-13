# Verified execution environment (2026-10-12)

## Repositories (host and Docker share /home)

- TileLang: `/home/s00653124/workspace/tilelang`; `main` at `0e687c17bb3bd8a92bfeaa2bc8a5bdb37f6b3ca4`. The working tree now includes staged NEW Golden files and a changed freeze script; do not assume clean.
- AscendNPU-IR: `/home/s00653124/workspace/AscendNPU-IR-1`; `master` at `e878f34927fb5b77fd8c7257fda3a95a3560f2ec`. Origin is `https://gitcode.com/Ascend/AscendNPU-IR.git`, not the old private GitHub fork.
- Reference: `/home/s00653124/workspace/tilelang-mlir-ascend`; `main` at `417dae9908ad2ea5ffb74294a7075b6cb64b4f2b`. Read only.

## Runtime, toolchains and test baselines

- All work happens inside existing `s00653124_tilelang` Docker container (Ubuntu 24.04). Host Claude Code modifies bind-mounted repositories.
- Container user `s00653124`, HOME=`/home/s00653124`, source `source /data/pri/cann-9.2.0/cann/set_env.sh` and `source /data/miniconda3/etc/profile.d/conda.sh`.
- Primary conda env: `s00653124_tilelang`; Triton NPU E2E env: `s00653124_ta36`.
- Python 3.11.17; torch 2.10.0+cpu with functional torch_npu 2.10.0.post5.dev20260815; two accessible Ascend950PR NPUs; target `Ascend950PR_9579`.
- LLVM/CMake build: `/home/s00653124/workspace/AscendNPU-IR-1/build`; Release, clang++ 15.0.6; build.ninja exists.
- NPU-IR compile artifacts: `build/bin/bishengir-compile`, `build/bin/bishengir-opt`, and `build/install/bin/` copies/links.
- Existing `bishengir-toolkits/dev/bin/bishengir-compile` is a link to the installed compiler; verify installed output refreshed after each incremental build.
- Bisheng: `/home/s00653124/workspace/ccec-toolkits/default/ccec_compiler/bin/bisheng`.
- Host ripgrep executable: `/home/s00653124/Softwares/ripgrep/rg` (not on PATH inside container by default).

## Baseline results: PASS

- TileLang: `cd ~/workspace/tilelang/examples/ascend && python -m pytest -sv test_vecadd.py -k simd`; 1 PASS, 1 deselected.
- NPU-IR compile: `cd ~/workspace/PlayGround/test_add && bash compile.sh dev triton_add.bcmlir`; exit 0, output `triton_add.o` and `pipeline_triton_add_dev.log`.
- Triton NPU E2E: in env `s00653124_ta36`, `cd ~/workspace/PlayGround/test_add_triton && bash run_triton.sh dev test_add.py python3`; exit 0 and NPU output. Review `test_add.py` to confirm whether it contains numerical assertions; tensor printing alone is insufficient.
- Evidence: `/home/s00653124/workspace/hivm-poc-evidence/e0-20261012-013931`.
- SSBUFFER fallback warning is explicitly OUT OF SCOPE for this project.

## Safe invocation

From this control directory:

```sh
./scripts/in-container.sh bash -c 'cd /home/s00653124/workspace/tilelang/examples/ascend && python -m pytest -sv test_vecadd.py -k simd'
HIVM_CONDA_ENV=s00653124_ta36 ./scripts/in-container.sh bash -c 'cd /home/s00653124/workspace/PlayGround/test_add_triton && bash run_triton.sh dev test_add.py python3'
```

Never assume the test script invokes the compiler just built; check `readlink -f` and binary hashes when necessary.
