#!/usr/bin/env bash
# flash.sh — Build and upload OneKey firmware via PlatformIO
# Usage:
#   ./flash.sh              Build and upload (ESP32-S3)
#   ./flash.sh monitor      Build, upload, then open serial monitor
#   ./flash.sh build        Build only (no upload)
#   ./flash.sh env esp32    Use a different PlatformIO environment

set -euo pipefail

ENV="esp32s3"
ACTION="upload"
MONITOR=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        monitor)  MONITOR=true; shift ;;
        build)    ACTION=""; shift ;;
        env)      ENV="$2"; shift 2 ;;
        *)        echo "Unknown option: $1"; exit 1 ;;
    esac
done

# Check for PlatformIO
if ! command -v pio &> /dev/null; then
    echo "PlatformIO CLI (pio) not found."
    echo ""
    echo "Install it with:"
    echo "  pip install platformio"
    echo ""
    echo "Or see: https://platformio.org/install/cli"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Copy firmware into PlatformIO's expected src/ directory
mkdir -p src
cp firmware/config.h src/config.h
cp firmware/one_key_keyboard.ino src/main.cpp

echo "=== OneKey BLE Keyboard ==="
echo "Environment: $ENV"
echo ""

if [[ -n "$ACTION" ]]; then
    echo "Building and uploading..."
    pio run -e "$ENV" -t upload
else
    echo "Building..."
    pio run -e "$ENV"
fi

if $MONITOR; then
    echo ""
    echo "Opening serial monitor (Ctrl+C to exit)..."
    pio device monitor -e "$ENV"
fi
