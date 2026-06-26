# ACP — Add Commit Push


[![CI](https://img.shields.io/github/actions/workflow/status/hastagaming/acp/ci.yml?branch=main&label=CI)](https://github.com/hastagaming/acp/actions/workflows/ci.yml) [![Release](https://img.shields.io/github/v/release/hastagaming/acp?label=release)](https://github.com/hastagaming/acp/releases)

[![License: Apache-2.0](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](./LICENSE) [![Termux](https://img.shields.io/badge/platform-Termux%20%7C%20Linux-success)](#installation)

A tiny CLI tool that turns this:

```bash
git add . && git commit -m "message" && git push
```

into this:

```acp
acp "message"
```

Written in pure C. No external dependency besides `git` itself, which is
what actually runs behind the scenes. Built for Termux on Android and for
Linux in general.

<p align="center">
  <img src="assets/acp_demo.gif" alt="ACP terminal demo: core command, show mode, and security check" width="700">
</p>

<p align="center"><sub>Real terminal output — core command, <code>-s</code> show mode, and <code>--check</code> security scan.</sub></p>

---

## Contents

- [Why](#why)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Behavior details](#behavior-details)
- [Project structure](#project-structure)
- [Continuous integration and releases](#continuous-integration-and-releases)
- [Publishing to TUR](#publishing-to-tur-termux-user-repository)

## Why

Typing `git add . && git commit -m "..." && git push` dozens of times a day
gets old fast, especially on a phone keyboard in Termux. ACP collapses that
into one short command, while staying out of the way:

- No setup wizard, no interactive prompts beyond what's strictly needed.
- No AI features, no telemetry, no extra dependencies.
- Detects your current branch automatically — no need to type it.
- Understands `.gitignore` and skips ignored files instead of failing.
- Catches accidental commits of `.env` files, secrets, or huge binaries
  before they reach your remote.

## Installation

### Option A — build from source

Requires `gcc` (or `clang`) and `git`:

```bash
pkg install clang git
```
or:
```bash
sudo apt install gcc git
```
(Linux desktop)

Then:

```bash
chmod +x install.sh
./install.sh
```

This compiles ACP and installs the binary to `$PREFIX/bin/acp`, so it can
be called from anywhere.

<details>
<summary>Manual build, without install.sh</summary>

```bash
make
./acp "commit message"
```
</details>

### Option B — install a precompiled binary

Every tagged release publishes ready-to-run binaries for two separate,
**non-interchangeable** targets:

| Target | Architectures | Linked against | Runs on |
|---|---|---|---|
| Termux | `aarch64`, `arm`, `x86_64` | Bionic libc | Real Termux/Android installs |
| Linux desktop | `x86_64`, `aarch64` | glibc | Ubuntu, Debian, and similar |

```bash
chmod +x install-prebuilt.sh
./install-prebuilt.sh
```

The script detects whether it's running inside Termux or a regular Linux
environment, downloads the binary matching your architecture from the
latest GitHub Release, verifies its SHA-256 checksum, and installs it.

## Usage

### Core command

```bash
acp "fix login bug"
```

Runs, in order:

1. `git add .`
2. `git commit -m "fix login bug"`
3. `git push origin <current-branch>` — branch detected automatically

### Show what will run, before running it

```bash
acp -s "update readme"
```

### Set the remote for this repository

```bash
acp --remote "https://github.com/user/repo.git"
```

Saved to `.git/acp.conf` (per-repository) and applied via `git remote`.

### Safe mode

```bash
acp --safe "important change"
```

Blocks force push, and warns if the active branch is `main` or `master`.

### Scan for sensitive files before committing

```bash
acp --check
```

Flags:
- `.env` files
- likely secrets/tokens/API keys inside source or text files
- suspiciously large files (configurable, default 50MB)

### Check the installed version

```bash
acp --version
```

## Configuration

`config/default.conf` holds global defaults, separate from the
per-repository remote in `.git/acp.conf`. It's installed to
`$PREFIX/etc/acp/default.conf` and read on every run — if it's missing or a
key is absent, ACP just falls back to its built-in defaults, so the file is
entirely optional.

```ini
DEFAULT_BRANCH_FALLBACK=main
LARGE_FILE_WARNING_MB=50
```

| Key | Used for |
|---|---|
| `DEFAULT_BRANCH_FALLBACK` | Branch name used when the current branch can't be detected (e.g. detached HEAD), instead of a hardcoded value |
| `LARGE_FILE_WARNING_MB` | Size threshold, in MB, above which `acp --check` flags a file |

Edit `$PREFIX/etc/acp/default.conf` directly to change a default
permanently — no wizard involved.

## Behavior details

**`.gitignore` handling**
- Ignored files mixed with normal files: ignored ones are skipped
  automatically, the rest still gets committed, and a warning lists what
  was skipped.
- Every detected file is ignored: the commit is cancelled with
  `Nothing to commit (all files are ignored).`

**Auto-init**
If run outside a git repository, ACP asks before doing anything:
```text
Not a git repository. Initialize now? (y/n)
```

**Error reporting**
Every git failure is reported with: the stage that failed (add / commit /
push), the exit code, the current folder, the raw git output, and a
plain-language hint about the likely cause and fix.

**Safety**
File names with spaces or special characters are handled safely, since the
internal command always uses `git add .` rather than iterating over
individual file names.

## Project structure

```project structure
ACP/
├── src/
│   ├── main.c       Entry point and flow orchestration
│   ├── git.c        Git operation wrappers (add, commit, push, branch)
│   ├── parser.c     CLI argument parsing
│   ├── remote.c     Save/load remote from .git/acp.conf
│   ├── safety.c     Safe mode and security scanning
│   ├── ignore.c     Analysis of git-ignored files
│   ├── error.c      Error message formatting
│   └── config.c     Reads $PREFIX/etc/acp/default.conf
├── include/
│   └── common.h     Shared header (structs, function declarations)
├── config/
│   └── default.conf Global default values
├── docs/
│   ├── README.md
│   └── assets/      Demo GIF and other docs media
├── .github/workflows/
│   ├── ci.yml             Compile check on every push/PR
│   └── build-release.yml  Native build per target + GitHub Release
├── Makefile
├── install.sh
├── install-prebuilt.sh
├── build.sh             TUR/termux-packages recipe
├── LICENSE
├── CHANGELOG.md
└── README.md
```

## Continuous integration and releases

Two GitHub Actions workflows live under `.github/workflows/`, both running
on GitHub-hosted runners:

- **`ci.yml`** — every push/PR to `main` touching `src/`, `include/`, or
  `Makefile`. Compiles with both `gcc` and `clang` (`-Wall -Wextra
  -Werror`) and smoke-tests the binary. Fast feedback while developing.

- **`build-release.yml`** — on `workflow_dispatch`, on PRs to `main`, and
  on any pushed tag matching `v*.*.*`. Builds two non-interchangeable sets
  of binaries in parallel:

  - `build-termux`: builds natively inside the official
    `termux/termux-docker` image for `aarch64`, `arm`, `x86_64` (QEMU for
    the non-x86_64 architectures). Output linked against Bionic libc.
  - `build-linux-desktop`: builds natively on `ubuntu-latest` (x86_64) and
    `ubuntu-24.04-arm` (native arm64, no emulation). Output linked against
    glibc.

  Both are uploaded as workflow artifacts, and on a tag push, attached
  automatically to a GitHub Release with a `.sha256` checksum each.

To cut a release:

```bash
git tag v1.0.0
git push origin v1.0.0
```

This builds all four binaries (`acp-termux-aarch64`,
`acp-termux-x86_64`, `acp-linux-x86_64`, `acp-linux-aarch64`) and publishes
them under the `v1.0.0` GitHub Release.

> The Termux `aarch64`/`arm` jobs run under QEMU emulation, so they take
> noticeably longer than the others — that's expected, not a failure.

## License

Apache License 2.0 — see [LICENSE](./LICENSE).