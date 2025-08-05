#!/usr/bin/env bash

set -euo pipefail

echo "[*] Building lame.ko..."
make -j$(nproc) M=arch/x86/kernel clean modules

echo "[*] Installing lame.ko..."
sudo make M=arch/x86/kernel modules_install 

echo "[*] Installing sanitized UAPI headers..."
make headers_install INSTALL_HDR_PATH=/tmp/lame-headers

echo "[*] Done!"
