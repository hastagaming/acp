#!/data/data/com.termux/files/usr/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "ACP Installer"
echo "============="

if ! command -v gcc >/dev/null 2>&1 && ! command -v clang >/dev/null 2>&1; then
    echo "Error: no C compiler found (gcc or clang)."
    echo "Install with: pkg install clang"
    exit 1
fi

if ! command -v git >/dev/null 2>&1; then
    echo "Error: git not found."
    echo "Install with: pkg install git"
    exit 1
fi

echo "Building and installing ACP..."
cd "$SCRIPT_DIR"

# Delegates to the Makefile's own "install" target instead of duplicating
# the build/copy steps here, so there is only one place (the Makefile)
# that needs to be kept in sync with the actual list of source files.
make clean
make install

echo ""
echo "Installation complete."
echo "Try running: acp --help"