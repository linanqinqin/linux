#!/bin/bash

# Script to manually install specific kernel modules
# Usage: sudo ./scripts/insmod.sh

# Array of relative paths to the modules you want to install
MODULES=(
    "arch/x86/kvm/kvm.ko"
    "arch/x86/kvm/kvm-intel.ko"
    # Add more modules here as needed
)

# Kernel version (e.g., "6.8.0")
KERNEL_VERSION=$(uname -r)

# Directory where modules will be installed
MODULES_DIR="/lib/modules/$KERNEL_VERSION/kernel"

# Signing tool and keys (if module signing is required)
SIGN_TOOL="scripts/sign-file"
SIGN_KEY_PEM="certs/signing_key.pem"
SIGN_KEY_X509="certs/signing_key.x509"

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root or use sudo."
    exit 1
fi

# Function to sign a module (if required)
sign_module() {
    local module=$1
    if [ -f "$SIGN_TOOL" ] && [ -f "$SIGN_KEY_PEM" ] && [ -f "$SIGN_KEY_X509" ]; then
        echo "Signing module: $module"
        $SIGN_TOOL sha512 "$SIGN_KEY_PEM" "$SIGN_KEY_X509" "$module"
    else
        echo "Skipping signing (signing tools or keys not found)."
    fi
}

# Function to compress a module
compress_module() {
    local module=$1
    echo "Compressing module: $module"
    zstd -f "$module" -o "$module.zst"
}

# Function to install a module
install_module() {
    local module=$1
    local dest_dir="$MODULES_DIR/$(dirname "$module")"

    echo "Installing module: $module.zst to $dest_dir"
    mkdir -p "$dest_dir"
    cp "$module.zst" "$dest_dir/"
}

# Main script logic
for module in "${MODULES[@]}"; do
    if [ -f "$module" ]; then
        # echo "Processing module: $module"
        SIGN "$module"
        ZSTD "$module"
        INSTALL "$module"
    else
        echo "Module not found: $module"
    fi
done

# Update module dependencies
echo "Updating module dependencies..."
depmod

echo "Module installation complete!"
