#!/usr/bin/env bash

# Wrapper to create disk image and install kernel.
# Uses create_disk.sh when present; otherwise falls back to tests/Makefile + mcopy.

set -euo pipefail

OS_BIN="${OS_BIN:-build/os.bin}"
DISK_IMG="${DISK_IMG:-disk.img}"
DISK_SIZE_MB="${DISK_SIZE_MB:-100}"

echo "Wrapper: OS_BIN=$OS_BIN, DISK_IMG=$DISK_IMG, DISK_SIZE_MB=$DISK_SIZE_MB"

if [ ! -f "$OS_BIN" ]; then
  echo "Error: $OS_BIN not found. Build the OS first with 'make os'"
  exit 1
fi

if [ -x ./scripts/create_disk.sh ]; then
  echo "==> Using scripts/create_disk.sh to create disk image"
  ./scripts/create_disk.sh || { echo "scripts/create_disk.sh failed"; exit 1; }
  mv -f disk.img "$DISK_IMG" || true
  echo "Kernel copied to disk via create_disk.sh"
  exit 0
fi

# Fallback: call tests/Makefile to create a test disk image
echo "==> Falling back to tests/Makefile disk creation"
make -C tests disk || true

if [ -f tests/test_disk.img ]; then
  cp tests/test_disk.img "$DISK_IMG" && echo "Copied tests/test_disk.img -> $DISK_IMG"
else
  echo "WARNING: tests/test_disk.img not found; disk creation in tests may have failed."
fi

if command -v mcopy > /dev/null 2>&1; then
  echo "==> Copying kernel to disk (KERNEL.BIN)"
  mcopy -i "$DISK_IMG"@@1M -o "$OS_BIN" ::KERNEL.BIN && echo "Kernel copied to disk"
else
  echo "WARNING: mcopy not found. Cannot copy kernel to disk."
fi
