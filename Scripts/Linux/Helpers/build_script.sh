#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/setup_qt.sh"

# Function to setup and build the project
setup_and_build() {
    local PROJECT_DIR=$1
    local PROJECT_NAME=$2
    local BUILD_TYPE=$3
    local BUILD_TARGET_TYPE=$4
    local BUILD_TEST_PROJECT=$5
    local THIRD_PARTY_INCLUDE_DIR=$6
    local QT_VERSION=$7
    local QT_COMPILER=$8
    local QT_COMPILER_DIR=$9
    local MIN_REQUIRED_PYTHON_VERSION=${10}
    local MIN_REQUIRED_PYTHON_VERSION_SUFFIX=${11}
    local BUILD_DIR_NAME=${12:-build}  # Default to "build" if not set

    # Prepare
    mkdir -p "$THIRD_PARTY_INCLUDE_DIR"

    # Setup Qt
    setup_qt $QT_VERSION $QT_COMPILER $QT_COMPILER_DIR $MIN_REQUIRED_PYTHON_VERSION $MIN_REQUIRED_PYTHON_VERSION_SUFFIX

    # Configure CMake
    BUILD_DIR="$(pwd)/$BUILD_DIR_NAME"
    cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -D${PROJECT_NAME}_BUILD_TARGET_TYPE:STRING=$BUILD_TARGET_TYPE -D${PROJECT_NAME}_BUILD_TEST_PROJECT:BOOL=$BUILD_TEST_PROJECT -DTHIRD_PARTY_INCLUDE_DIR:PATH=$THIRD_PARTY_INCLUDE_DIR -DCMAKE_PREFIX_PATH="$QT_DIR/$QT_VERSION/$QT_COMPILER_DIR"

    # Build
    cmake --build "$BUILD_DIR" --config $BUILD_TYPE
}
