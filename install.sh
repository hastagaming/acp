#!/data/data/com.termux/files/usr/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_PREFIX="${PREFIX:-/usr/local}"
TARGET_BIN="$INSTALL_PREFIX/bin/acp"
CONFIG_DIR="$INSTALL_PREFIX/etc/acp"

echo "ACP Installer"
echo "============="

if ! command -v gcc >/dev/null 2>&1; then
    echo "Error: gcc not found."
    echo "Install with: pkg install clang"
    exit 1
fi

if ! command -v git >/dev/null 2>&1; then
    echo "Error: git not found."
    echo "Install with: pkg install git"
    exit 1
fi

echo "Compiling ACP..."
cd "$SCRIPT_DIR"
gcc -Wall -Wextra -O2 -std=c11 -o acp \
    src/main.c src/git.c src/parser.c src/remote.c \
    src/safety.c src/ignore.c src/error.c src/util.c src/config.c

if [ ! -f "$SCRIPT_DIR/acp" ]; then
    echo "Error: compile failed."
    exit 1
fi

echo "Installing binary to $TARGET_BIN..."
cp "$SCRIPT_DIR/acp" "$TARGET_BIN"
chmod +x "$TARGET_BIN"

echo "Setting up config at $CONFIG_DIR..."
mkdir -p "$CONFIG_DIR"

if [ -f "$SCRIPT_DIR/config/default.conf" ]; then
    cp "$SCRIPT_DIR/config/default.conf" "$CONFIG_DIR/default.conf"
fi

echo ""
echo "Installation complete."
echo "Try running: acp --help"