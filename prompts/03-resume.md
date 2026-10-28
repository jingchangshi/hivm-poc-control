# Resume after quotas/session loss

> **历史文件（Historical）。** “Claude Code implicit Team 目录 / Magpie 路由”的表述已废弃；
> **从文件与提交重建当前状态、不 reset 用户改动、不重复无关基线**这一实质要求**仍适用**。
> 当前恢复入口：根 `README.md`、`docs/current-state.md`，并用 AgentGates `doctor`/`report` 现场核验。

Read CLAUDE.md, docs/goal.md, docs/environment.md, docs/acceptance.md, docs/status.md, and docs/decisions.md. Check actual `git status --short`, `git log -3` and current branches in BOTH local repos. Do not reset user changes. Find existing proof under `/home/s00653124/workspace/hivm-poc-evidence`.

Rebuild the CURRENT state from files and commits, not from assumptions about persistent agent sessions. Claude Code implicit Team directories may be removed when the session exits. If needed, spawn named teammates with approved roles and models. Confirm actual Magpie routing. Finish the earliest unblocked stage, independently re-run only the tests affected by new changes, and persist new status before exit. Do not repeat E0 broad baselines unless source/driver/runtime changed.
