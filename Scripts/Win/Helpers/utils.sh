#!/bin/bash

# Function to check if the script is running as admin
check_admin() {
    net session > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "This script must be run as an administrator."
        echo "Restarting script with administrative privileges..."
        # Get the name of the calling script
        CALLING_SCRIPT=$(basename "$0")
        powershell -Command "Start-Process powershell -ArgumentList '-NoProfile -ExecutionPolicy Bypass -Command \"cd $(pwd -W); & ./$CALLING_SCRIPT\"' -Verb RunAs"
        exit 1
    fi
}

# Function to compare Python versions
version_greater_equal() {
    local version1=$1
    local version2=$2
    [ "$(printf '%s\n' "$version1" "$version2" | sort -V | head -n1)" = "$version2" ]
}

# Function to check available disk space using wmic
check_disk_space() {
    local dir=$1
    local required_space_mb=$2
    local drive_letter=$(echo $dir | cut -d':' -f1)
    if command -v wmic &> /dev/null; then
        local available_space_bytes=$(wmic logicaldisk where "DeviceID='${drive_letter}:'" get FreeSpace | grep -Eo '[0-9]+' | tr -d '\r\n')
        if [ -z "$available_space_bytes" ]; then
            echo "Failed to retrieve available disk space for drive ${drive_letter}:"
            return 1
        fi
        local available_space_mb=$((available_space_bytes / 1024 / 1024))
        if [ "$available_space_mb" -lt "$required_space_mb" ]; then
            echo "Not enough disk space. Required: ${required_space_mb}MB, Available: ${available_space_mb}MB"
            return 1
        fi
    else
        echo "wmic is not available to check disk space."
        return 1
    fi
}
