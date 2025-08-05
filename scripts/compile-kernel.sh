#!/usr/bin/env bash

set -euo pipefail

echo "[*] Building the kernel..."
make -j$(nproc)

echo "[*] Installing modules and kernel..."
sudo make modules_install 
sudo make install

echo "[*] Installing sanitized UAPI headers..."
make headers_install INSTALL_HDR_PATH=/tmp/lame-headers

echo "[*] Done!"
