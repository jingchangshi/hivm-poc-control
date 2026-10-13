# Follow-up turn after approving P0

Read the approved docs/late-ir-contract.md, docs/pass-cut.md, docs/acceptance.md and docs/status.md. Continue using the existing named teammates if available. If the old team no longer exists after a restart, create a fresh implicit team with the same agent definitions and pinned model aliases; do not reconstruct ~/.claude/teams by hand.

Implement only the next unblocked phase from P1 to P6 in docs/goal.md. For each phase:
1. Ask verifier to prepare the independent acceptance tests and negative cases.
2. Ask challenger to approve semantic/ABI assertions and exact stage cut before implementation.
3. Assign bounded production code files to implementor; tests remain independent.
4. Run actual Docker compiler tests and when appropriate real NPU tests with the recorded conda env.
5. Challenger reviews the diff and the verifier's actual evidence.
6. Lead updates phase status, commits verified changes, records all logs and commits, and advances only on PASS.

Do not treat P4 prebuilt-runtime success as P6 CodeGen success. Stop and interact with the user only when architecture/contract must change or access/permission is missing. Otherwise continue until the next evidence-based gate without broad speculative refactors.
