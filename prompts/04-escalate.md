# Architecture blocker prompt

> **历史文件（Historical）：角色名 `challenger-opus` 属旧命名。** 当前对应角色为 reviewer / challenger
> （见根 `README.md`）。**升级协议本身仍适用**：停止受影响的工作流、要求有源码依据的报告、
> 至少两个竞争假设、一个最小判别实验、以及 APPROVE/REVISE/BLOCK 裁决。

STOP only the affected workstream. Send the problem to challenger-opus.

Request a source-grounded report with: expected vs observed; exact local file/function; git SHA; minimal reproduce command; at least two competing hypotheses; one or two small discriminating experiments; result; necessary scope of change; verdict APPROVE/REVISE/BLOCK.

If a decision would change Golden semantics, target ABI, synchronization, UB layout, source pipeline boundaries, or user-approved scope, prepare a short user-facing decision question with evidence and alternatives. Keep independent workstreams unblocked if they do not depend on the choice. Never enlarge timeout or silently relax the acceptance oracle.
