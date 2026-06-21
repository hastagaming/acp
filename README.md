# ACP (Add Commit Push)

A simple CLI tool that turns:

\`\`\`
git add . && git commit -m "message" && git push
\`\`\`

into:

\`\`\`
acp "message"
\`\`\`

Written in pure C, with no external dependency besides `git` itself (which is what actually runs behind the scenes). Built for Termux (Android ARM64) and Linux in general.

## License

Apache License 2.0. See [LICENSE](./LICENSE).

## Installation

Make sure `gcc` (or `clang`) and `git` are installed:

\`\`\`
pkg install clang git
\`\`\`

Then run:

\`\`\`
chmod +x install.sh
./install.sh
\`\`\`

The binary will be installed at `$PREFIX/bin/acp`, so it can be called from anywhere.

### Manual build (without install.sh)

\`\`\`
make
./acp "commit message"
\`\`\`

### Install a precompiled binary (no compiler needed)

Every tagged release publishes precompiled binaries for `aarch64`, `arm`,
and `x86_64` (see "Continuous Integration and Releases" below). To install
one directly:

\`\`\`
chmod +x install-prebuilt.sh
./install-prebuilt.sh
\`\`\`

This downloads the binary matching your device's architecture from the
latest GitHub Release, verifies its SHA-256 checksum, and installs it to
`$PREFIX/bin/acp`.

## Usage

### Core command

\`\`\`
acp "fix login bug"
\`\`\`

This runs:
1. `git add .`
2. `git commit -m "fix login bug"`
3. `git push origin <current-branch>` (branch is detected automatically)

### Configure a remote per repository

\`\`\`
acp --remote "https://github.com/user/repo.git"
\`\`\`

The remote is saved to `.git/acp.conf` and applied directly via `git remote`.

### Show mode

\`\`\`
acp -s "update readme"
\`\`\`

Prints the git commands that will run before they are actually executed.

### Safe mode

\`\`\`
acp --safe "important change"
\`\`\`

Blocks force push and warns if the active branch is `main` or `master`.

### Security check

\`\`\`
acp --check
\`\`\`

Scans the project folder for:
- `.env` files
- Secret/token/API key patterns inside text/source files
- Suspiciously large files (configurable, default 50MB)

### Check installed version

\`\`\`
acp --version
\`\`\`

## Configuration

`config/default.conf` holds global default values, separate from the
per-repository remote stored in `.git/acp.conf`. After installation it is
copied to `$PREFIX/etc/acp/default.conf`, and `acp` reads it on every run.
If the file is missing or a key is absent, ACP falls back to its built-in
defaults, so the file is optional.

\`\`\`
DEFAULT_BRANCH_FALLBACK=main
LARGE_FILE_WARNING_MB=50
\`\`\`

- `DEFAULT_BRANCH_FALLBACK`: branch name used when the current branch
  cannot be detected from git (for example, a detached HEAD), instead of
  the hardcoded value `main`.
- `LARGE_FILE_WARNING_MB`: file size threshold, in megabytes, above which
  `acp --check` flags a file as suspiciously large.

To change a default permanently, edit `$PREFIX/etc/acp/default.conf`
directly; no wizard or interactive setup is involved.

## .gitignore handling

- If normal files are mixed with ignored files, the ignored files are skipped automatically and ACP still proceeds to commit the normal files, printing a warning listing what was skipped.
- If every detected file is ignored, the commit is cancelled with:
  `Nothing to commit (all files are ignored).`

## Auto-init repository

If run inside a folder that is not yet a git repository, ACP asks:

\`\`\`
Not a git repository. Initialize now? (y/n)
\`\`\`

Answer `y` to run `git init`, or `n` to cancel.

## Error handling

Every git failure is reported with a clear format: stage (add/commit/push), exit code, current folder, the raw git output, and a plain-language hint about the likely cause and fix.

## Project structure

\`\`\`
ACP/
├── src/
│   ├── main.c       Entry point and flow orchestration
│   ├── git.c        Git operation wrappers (add, commit, push, branch)
│   ├── parser.c     CLI argument parsing
│   ├── remote.c     Save/load remote from .git/acp.conf
│   ├── safety.c     Safe mode and security scanning (.env, secrets, large files)
│   ├── ignore.c     Analysis of git-ignored files
│   ├── error.c      Error message formatting
│   └── config.c     Reads $PREFIX/etc/acp/default.conf
├── include/
│   └── common.h     Shared header (structs, function declarations)
├── config/
│   └── default.conf Global default values (copied to $PREFIX/etc/acp/)
├── docs/
│   └── README.md
├── .github/
│   └── workflows/
│       ├── ci.yml             Compile check on every push/PR
│       └── build-release.yml  Native build per arch + GitHub Release
├── Makefile
├── install.sh
├── install-prebuilt.sh
├── LICENSE
├── CHANGELOG.md
└── README.md
\`\`\`

## Notes

- No AI features, no setup wizard, no verbose mode.
- Every git operation is run through the system's own `git` binary.
- Safe for file names with spaces and special characters, since the internal command always uses `git add .` rather than iterating over individual file names.

## Continuous Integration and Releases

This repository ships two GitHub Actions workflows under `.github/workflows/`:

- **`ci.yml`** — runs on every push/PR to `main` that touches `src/`, `include/`,
  or `Makefile`. Compiles with both `gcc` and `clang` using `-Wall -Wextra -Werror`
  and runs a basic smoke test (`acp --version`, `acp --help`). This is the fast
  feedback loop while developing.

- **`build-release.yml`** — runs on `workflow_dispatch`, on PRs to `main`, and
  on any pushed tag matching `v*.*.*`. For each Termux architecture
  (`aarch64`, `arm`, `x86_64`) it pulls the official `termux/termux-docker`
  image, installs `clang`/`make` inside it, and compiles ACP **natively**
  inside that Termux environment (using QEMU for the non-native architectures
  on the GitHub-hosted runner). The resulting binaries are uploaded as
  workflow artifacts, and when triggered by a tag push, they are also
  attached to a GitHub Release automatically, alongside a `.sha256` checksum
  file for each binary.

To cut a new release:

\`\`\`
git tag v1.0.0
git push origin v1.0.0
\`\`\`

The workflow will build `acp-aarch64`, `acp-arm`, and `acp-x86_64`, then
publish them under the `v1.0.0` GitHub Release.

Note: cross-architecture builds run under QEMU emulation on the GitHub-hosted
x86_64 runner, so `aarch64` and `arm` jobs are slower than the native
`x86_64` job — this is expected and not a failure.

## Publishing to TUR (Termux User Repository)

This repository contains the ACP source code and CI/release automation.
The Termux packaging recipe (`build.sh`) lives in a separate repository — the
[TUR](https://github.com/termux-user-repository/tur) fork — under
`packages/acp/build.sh`. Per TUR/termux-packages conventions, the recipe
builds from the source tarball of a tagged release (not from a raw binary),
since `TERMUX_PKG_SRCURL` must point to an extractable archive. To publish
a new version:

1. Tag and push a release in this repository:
   \`\`\`
   git tag v1.0.0
   git push origin v1.0.0
   \`\`\`
2. Compute the SHA-256 of the generated GitHub source tarball:
   \`\`\`
   curl -sL https://github.com/<user>/acp/archive/refs/tags/v1.0.0.tar.gz | sha256sum
   \`\`\`
3. Update `TERMUX_PKG_SRCURL`, `TERMUX_PKG_SHA256`, and `TERMUX_PKG_VERSION`
   in `packages/acp/build.sh` inside your TUR fork.
4. Open a pull request against `termux-user-repository/tur`.

The precompiled binaries produced by `build-release.yml` are a separate,
faster install path for users outside of TUR (via `install-prebuilt.sh`) —
they are not used by the TUR recipe itself.