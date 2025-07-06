#!/usr/bin/env bash

set -euo pipefail

# echo "[*] Building the kernel..."
# make -j$(nproc)

echo "[*] Installing modules and kernel..."
sudo make modules_install install

echo "[*] Locating the newly installed kernel..."
# Find the latest vmlinuz in /boot after installation
LATEST_KERNEL=$(ls -t /boot/vmlinuz-* | head -n1)

if [[ ! -f "$LATEST_KERNEL" ]]; then
    echo "[!] Failed to locate newly installed kernel in /boot. Exiting."
    exit 1
fi

# Extract the kernel version from the filename
KERNEL_VERSION=$(basename "$LATEST_KERNEL" | sed 's/vmlinuz-//')

echo "[*] Found installed kernel: $KERNEL_VERSION"

INITRD="/boot/initrd.img-$KERNEL_VERSION"
if [[ ! -f "$INITRD" ]]; then
    # Some distros use initramfs.img
    INITRD="/boot/initramfs-$KERNEL_VERSION.img"
    if [[ ! -f "$INITRD" ]]; then
        echo "[!] Could not find initrd for $KERNEL_VERSION at $INITRD"
        exit 1
    fi
fi

echo "[*] Loading kernel using kexec..."
sudo kexec -l "$LATEST_KERNEL" --initrd="$INITRD" --reuse-cmdline

echo ""
echo "[*] Kernel loaded with kexec."
echo "    You can now immediately reboot into the new kernel using:"
echo "        sudo kexec -e"
echo ""
read -p "Do you want to execute kexec reboot now? (y/N): " confirm

if [[ "$confirm" == "y" || "$confirm" == "Y" ]]; then
    echo "[*] Syncing disks before kexec..."
    sync
    echo "[*] Rebooting into new kernel..."
    sudo kexec -e
else
    echo "[*] New kernel is loaded and ready. You can reboot later with:"
    echo "    sudo kexec -e"
fi
