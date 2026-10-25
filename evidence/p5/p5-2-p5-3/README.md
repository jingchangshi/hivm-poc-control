# P5-2 / P5-3 Machine Evidence

Round: P5 (authorized P5-0..P5-4). Generated 2026-10-25.
Sandbox: sandboxDegraded=true (no sudo in this harness; see alignment OWNER_CONFIRMED
SANDBOX-DEGRADED choice b). Every receipt in this directory carries that limitation.

## What P5-2 delivered (text)

`src/ascend/codegen/hivm_codegen.cc` is a real TIR visitor + MLIR text emitter.
Input is the prepared TIR (`_prepare_device_codegen_mod` output), NOT the P1 MLIR
and NOT any frozen string. It refuses any node outside its supported subset with a
diagnostic naming the node type and the enclosing function; there is no silent skip
and no partial output.

## What P5-3 delivered (binary)

`target.build.tilelang_hivm` now compiles the emitted MLIR through the P2 Late
pipeline (`bishengir-compile --input-ir-stage=late-hivm`) via the
`tilelang_callback_hivm_compile` hook implemented in `tilelang/contrib/hivm.py`.
On any failure it RAISES; it never returns the frozen PREBUILT binary.

## Files

| file | meaning |
|---|---|
| `p5-codegen-tests-with-compiler.txt` | 21 passed. Full set INCLUDING the 4 real-compile tests. |
| `p5-codegen-tests.txt` | 17 passed, 4 skipped. Same set with HIVM_COMPILER unset; the 4 compile tests skip honestly rather than passing on a weaker check. |
| `p5-ascend-suite-baseline.txt` | whole `testing/ascend` suite WITH the P5 changes: 45 failed, 1145 passed. |
| `p5-ascend-suite-clean-baseline.txt` | same suite with the P5 changes stashed (clean tree, rebuilt): 45 failed, 1128 passed. |

The two suite runs differ by exactly the 17 new P5 tests and share the same 45
failures, so the P5 change introduces no regression. The 45 failures are pre-existing
Bisheng codegen failures (`tilelang/contrib/bisheng.py:129 RuntimeError`), unrelated to
this work.

## Decisive result

A binary generated end to end from the frozen Golden TIR -- TIR -> Late MLIR -> P2 Late
pipeline -- is BYTE-IDENTICAL to the frozen P3 AIV binary:

    sha256 2c4247c3b2ce11f79ad15415eb06d4ce1d5523a0c19c91ad3743bc08e7b8c4c9
    size   4448 B

That digest is asserted AGAINST as an expected value in the test. It is never used as a
lookup key anywhere in the producer, and no MLIR text is copied from the P1 artefacts.

## Not covered here

Hardware execution and numerical correctness of the generated binary are NOT_RUN in this
round (P5 is a codegen phase). Device work remains governed by the separate device rules.
