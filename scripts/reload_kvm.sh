#!/bin/bash

# Script to reload kvm and kvm_intel modules
# Non-verbose: only outputs errors if they occur

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root or with sudo" >&2
    exit 1
fi

# Module names (unload in reverse dependency order)
UNLOAD_MODULES=("kvm_intel" "kvm")
LOAD_MODULES=("kvm" "kvm_intel")

# Function to check if a module is loaded
is_module_loaded() {
    local module="$1"
    lsmod | grep -q "^${module} "
}

# Unload modules
for module in "${UNLOAD_MODULES[@]}"; do
    # Attempt to unload
    if ! modprobe -r "$module" >/dev/null 2>&1; then
        echo "Error: Failed to unload module $module (is it in use?)" >&2
        exit 1
    fi

    # Verify it is unloaded
    if is_module_loaded "$module"; then
        echo "Error: Module $module is still loaded after removal" >&2
        exit 1
    fi
done

# Load modules
for module in "${LOAD_MODULES[@]}"; do
    # Attempt to load
    if ! modprobe "$module" >/dev/null 2>&1; then
        echo "Error: Failed to load module $module" >&2
        exit 1
    fi

    # Verify it is loaded
    if ! is_module_loaded "$module"; then
        echo "Error: Module $module failed to load" >&2
        exit 1
    fi
done

# Exit silently if successful
exit 0
