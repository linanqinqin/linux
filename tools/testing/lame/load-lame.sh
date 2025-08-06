#!/bin/bash

# Load LAME kernel module and set device permissions
# This script unloads the module (if loaded), reloads it, and sets permissions

set -e

echo "this script is deprecated and will cause unexpected behavior."
echo "exiting without doing anything."
exit 0

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "This script needs to be run as root (use sudo)"
    echo "Usage: sudo $0"
    exit 1
fi

# unload and load the module
modprobe -r lame 2>/dev/null || true
modprobe lame

# Wait a moment for the device to be created
sleep 1

# set device permissions to allow non-root users to read/write (660)
chmod 660 /dev/lame
