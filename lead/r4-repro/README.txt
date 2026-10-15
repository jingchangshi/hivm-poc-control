Lead-side INDEPENDENT reproduction of review-4's two R2 findings (2026-10-15, lead).
Not part of the frozen evidence set; lead working note. Instrument:
  bishengir-opt sha256 3514f5fee810ce59795f6bb47387c451195e14550a81746a9d0fa321c58d00ae
  FileCheck /home/s00653124/workspace/AscendNPU-IR-1/build/bin/FileCheck
  run via scripts/in-container.sh (host glibc too old: GLIBCXX_3.4.29 missing)

FINDING 1 (false PASS, confirmed):
  out3.mlir  = the REAL emitted non-950 module (3-operand ABI)
  mutant3.mlir = same module with a 4th i32 appended to decl, call and type suffix
  chk3.txt = the CHECK lines as LANDED in ef56cd772
  RESULT: old chk3 vs mutant3 -> FileCheck rc=0  == FALSE PASS
          (cause: `%{{.*}}` is greedy; `%[[C0]]` binds to the SECOND %c0_i32)
  chk3-fixed.txt (capture the casts, bound the call arity) -> real rc=0, mutant rc=1

FINDING 2 (missing in-repo shape negative, confirmed):
  in_shape.mlir = plain memref (not pointer_cast) + cache4 on Ascend950
  L1 rc=0 (accepted); L2 rc=1 with the provably-DMA-only diagnostic
  No landed RUN in bishengir/test asserts that rejection.
