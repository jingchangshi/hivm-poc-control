# Architecture blocker prompt

STOP only the affected workstream. Send the problem to challenger-opus.

Request a source-grounded report with: expected vs observed; exact local file/function; git SHA; minimal reproduce command; at least two competing hypotheses; one or two small discriminating experiments; result; necessary scope of change; verdict APPROVE/REVISE/BLOCK.

If a decision would change Golden semantics, target ABI, synchronization, UB layout, source pipeline boundaries, or user-approved scope, prepare a short user-facing decision question with evidence and alternatives. Keep independent workstreams unblocked if they do not depend on the choice. Never enlarge timeout or silently relax the acceptance oracle.
