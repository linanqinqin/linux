#!/usr/bin/env bash

set -euo pipefail

echo "[*] Building the kernel..."
make -j$(nproc)

echo "[*] Installing modules and kernel..."
sudo make modules_install install

echo "[*] Done!"
