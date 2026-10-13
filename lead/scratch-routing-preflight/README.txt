Lead routing-pair preflight, 2026-10-14 (control commit 3366e0d window).
Read-only: no build, no compiler invocation, no runner or binding edit.
routing-preflight.py  - AST-extracts LATE_MARKER_RE / ORD_MARKER_RES from the
  runner, extracts the r1 binaries' own [P2-BOUNDARY] strings with strings(1),
  exercises the patterns against those lines, then calls the runner's OWN
  routing_scan/routing_detail on the archived routing-pair streams and on the
  223 B markerless stub as a negative control.  Nothing is retyped.
routing-preflight.txt - output of record.
Subject runner sha256: 904a0d706288b32bdeb39d3949dc9ff2a131c065809bb10d9cd42309285f2df3
  (v3 was being edited by the verifier during this window; the sha above is the
  one under test and is printed inside the output as well.)
Subject binaries: stage-p2b-r1/bin/bishengir-compile ea217b8d423b2e46...,
  stage-p2b-r1/bin/bishengir-opt 4c31167bfad5243e...
