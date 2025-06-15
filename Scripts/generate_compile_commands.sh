#!/bin/bash

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check if cmake is available
if ! command_exists cmake; then
    echo "Error: cmake is not installed or not in the PATH."
    read -p "Press enter to continue"
    exit 1
fi

# Check if ninja is available
if ! command_exists ninja; then
    echo "Error: ninja is not installed or not in the PATH."
    read -p "Press enter to continue"
    exit 1
fi

# Set the solution directory
SOLUTION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && cd .. && pwd)"

# Set the build directory
BUILD_DIR="${SOLUTION_DIR}/_build"

# Output the solution and build directories
echo "SOLUTION_DIR: ${SOLUTION_DIR}"
echo "BUILD_DIR: ${BUILD_DIR}"

# Create the build directory if it doesn't exist
mkdir -p "${BUILD_DIR}"

# Define the main project name
MAIN_PROJECT_NAME=$(basename "$SOLUTION_DIR")

# Run CMake with Ninja to generate compile_commands.json
cd "${BUILD_DIR}"
cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -D${MAIN_PROJECT_NAME}_BUILD_TARGET_TYPE=static_library -D${MAIN_PROJECT_NAME}_BUILD_TEST_PROJECT=ON -DTHIRD_PARTY_INCLUDE_DIR="${SOLUTION_DIR}/ThirdPartyDir" "${SOLUTION_DIR}"

if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed."
    read -p "Press enter to continue"
    exit 1
fi

echo "compile_commands.json for project created in ${BUILD_DIR}"

read -p "Press enter to continue"
