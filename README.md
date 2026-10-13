# HIVM POC Agent Teams kit

This directory is the persistent control plane for a two-repository, real-NPU compiler POC.
Run Claude Code on the host; run all compiler builds and NPU tests in the existing Docker container.

## Usage

1. Unpack this directory as `~/workspace/hivm-poc-control`.
2. Keep your existing Magpie endpoint and credential exports; do not place secrets in this directory.
3. Verify tier mapping: sonnet = GPT-6.1 Sol, haiku = Haiku 5.5, opus = Opus 5.5, using the real IDs that passed your Magpie tests.
4. Run `bash scripts/check-golden-readonly.sh` before any file migration.
5. Start Claude Code with the commands shown in `prompts/01-start.md`.
6. Give the lead `docs/goal.md` and the task in `prompts/01-start.md`.
7. Run P0 separately from implementation. Use `prompts/02-implement.md` only after P0 and the architecture gate pass.
8. On a new session, use `prompts/03-resume.md`.

Do not directly run generators, compiler installers, or destructive git commands before P0 validates the existing staged Golden artifacts.
