#!/data/data/com.termux/files/usr/bin/bash

set -e

REPO="REPLACE_WITH_YOUR_USERNAME/acp"
VERSION="${1:-latest}"
INSTALL_PREFIX="${PREFIX:-/usr/local}"
TARGET_BIN="$INSTALL_PREFIX/bin/acp"

echo "ACP Prebuilt Installer"
echo "======================"

if ! command -v curl >/dev/null 2>&1; then
    echo "Error: curl not found."
    echo "Install with: pkg install curl"
    exit 1
fi

ARCH="$(uname -m)"
case "$ARCH" in
    aarch64) ASSET="acp-aarch64" ;;
    armv7l|armv8l|arm) ASSET="acp-arm" ;;
    x86_64) ASSET="acp-x86_64" ;;
    *)
        echo "Error: unsupported architecture '$ARCH'."
        exit 1
        ;;
esac

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
cp "$TMP_FILE" "$TARGET_BIN"
chmod +x "$TARGET_BIN"
rm -f "$TMP_FILE"

echo ""
echo "Installation complete."
"$TARGET_BIN" --version