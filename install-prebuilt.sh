#!/usr/bin/env bash

set -e

REPO="hastagaming/acp"
VERSION="${1:-latest}"

echo "ACP Prebuilt Installer"
echo "======================"

if ! command -v curl >/dev/null 2>&1; then
    echo "Error: curl not found."
    if [ -n "$PREFIX" ] && [ -d "$PREFIX" ]; then
        echo "Install with: pkg install curl"
    else
        echo "Install with your distro's package manager, e.g.: apt install curl"
    fi
    exit 1
fi

# Detect whether this is running inside Termux or a regular Linux desktop,
# since the two release tracks are built against different C libraries
# (Termux uses Bionic libc, Linux desktops use glibc) and are not
# interchangeable.
if [ -d "/data/data/com.termux" ] && [ -n "$PREFIX" ]; then
    PLATFORM="termux"
    INSTALL_PREFIX="$PREFIX"
else
    PLATFORM="linux"
    INSTALL_PREFIX="${PREFIX:-/usr/local}"
fi

TARGET_BIN="$INSTALL_PREFIX/bin/acp"

ARCH="$(uname -m)"
case "$ARCH" in
    aarch64) NORMALIZED_ARCH="aarch64" ;;
    armv7l|armv8l|arm) NORMALIZED_ARCH="arm" ;;
    x86_64) NORMALIZED_ARCH="x86_64" ;;
    *)
        echo "Error: unsupported architecture '$ARCH'."
        exit 1
        ;;
esac

if [ "$PLATFORM" = "termux" ]; then
    if [ "$NORMALIZED_ARCH" = "arm" ] || [ "$NORMALIZED_ARCH" = "aarch64" ] || [ "$NORMALIZED_ARCH" = "x86_64" ]; then
        ASSET="acp-termux-${NORMALIZED_ARCH}"
    else
        echo "Error: unsupported Termux architecture '$ARCH'."
        exit 1
    fi
else
    if [ "$NORMALIZED_ARCH" = "aarch64" ] || [ "$NORMALIZED_ARCH" = "x86_64" ]; then
        ASSET="acp-linux-${NORMALIZED_ARCH}"
    else
        echo "Error: unsupported Linux desktop architecture '$ARCH' (only x86_64 and aarch64 are published)."
        exit 1
    fi
fi

echo "Detected platform: $PLATFORM ($ARCH)"
echo "Selected asset: $ASSET"

if [ "$VERSION" = "latest" ]; then
    DOWNLOAD_URL="https://github.com/$REPO/releases/latest/download/$ASSET"
else
    DOWNLOAD_URL="https://github.com/$REPO/releases/download/$VERSION/$ASSET"
fi

echo "Downloading $ASSET ($VERSION) from GitHub Releases..."
TMP_FILE="$(mktemp)"
curl -fsSL "$DOWNLOAD_URL" -o "$TMP_FILE"

echo "Verifying checksum..."
CHECKSUM_URL="$DOWNLOAD_URL.sha256"
if curl -fsSL "$CHECKSUM_URL" -o "$TMP_FILE.sha256" 2>/dev/null; then
    EXPECTED_SUM="$(awk '{print $1}' "$TMP_FILE.sha256")"
    ACTUAL_SUM="$(sha256sum "$TMP_FILE" | awk '{print $1}')"
    if [ "$EXPECTED_SUM" != "$ACTUAL_SUM" ]; then
        echo "Error: checksum mismatch. Aborting installation."
        rm -f "$TMP_FILE" "$TMP_FILE.sha256"
        exit 1
    fi
    rm -f "$TMP_FILE.sha256"
    echo "Checksum verified."
else
    echo "Warning: could not fetch checksum file, skipping verification."
fi

echo "Installing to $TARGET_BIN..."
mkdir -p "$(dirname "$TARGET_BIN")"
cp "$TMP_FILE" "$TARGET_BIN"
chmod +x "$TARGET_BIN"
rm -f "$TMP_FILE"

echo ""
echo "Installation complete."
"$TARGET_BIN" --version