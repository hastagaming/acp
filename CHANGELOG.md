# Changelog

All notable changes to ACP will be documented in this file.

## [1.0.0] - 2026-06-21

### Added
- Core command: `acp "message"` runs `git add .`, `git commit -m`, and `git push origin <current-branch>` with automatic branch detection.
- `--remote "URL"`: saves a per-repository remote into `.git/acp.conf` and applies it via `git remote add/set-url origin`.
- `-s`: show mode, prints the git commands before they run.
- `--safe`: blocks force push and warns when the active branch is `main` or `master`.
- `--check`: scans the project for `.env` files, likely secret/token patterns in source files, and suspiciously large files.
- Automatic `.gitignore` handling: ignored files are skipped with a warning; if every changed file is ignored, the commit is cancelled with a clear message.
- Auto-init prompt when run outside a git repository.
- Detailed error reporting for add/commit/push failures: stage, exit code, current folder, raw git output, and a plain-language hint.
- `--version` / `-v` and `--help` / `-h`.
- Configurable defaults via `config/default.conf` (installed to `$PREFIX/etc/acp/default.conf`): branch fallback and large-file warning threshold.
