#!/usr/bin/env bash

set -euo pipefail

echo "[*] Building the kernel..."
make -j$(nproc)

echo "[*] Installing modules and kernel..."
sudo make M=arch/x86/kernel modules_install 
sudo make install

echo "[*] Done!"
