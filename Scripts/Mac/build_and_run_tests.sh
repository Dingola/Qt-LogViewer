#!/bin/bash

source Helpers/build_script.sh

# Set variables
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PROJECT_NAME=$(basename "$PROJECT_DIR")
BUILD_TYPE="Release"
BUILD_TARGET_TYPE="static_library"
BUILD_TEST_PROJECT=true
THIRD_PARTY_INCLUDE_DIR="$(pwd)/ThirdPartyDir"
QT_VERSION="6.8.0"
QT_COMPILER="clang_64"
QT_COMPILER_DIR="macOS"
MIN_REQUIRED_PYTHON_VERSION="3.13"
MIN_REQUIRED_PYTHON_VERSION_SUFFIX="__EMPTY__"
BUILD_DIR_NAME="_build_tests"

# Setup and build the project
setup_and_build $PROJECT_DIR $PROJECT_NAME $BUILD_TYPE $BUILD_TARGET_TYPE $BUILD_TEST_PROJECT $THIRD_PARTY_INCLUDE_DIR $QT_VERSION $QT_COMPILER $QT_COMPILER_DIR $MIN_REQUIRED_PYTHON_VERSION $MIN_REQUIRED_PYTHON_VERSION_SUFFIX $BUILD_DIR_NAME

# Run tests
BUILD_DIR="$(pwd)/$BUILD_DIR_NAME"
if [ "$BUILD_TEST_PROJECT" = true ]; then
    TEST_APP_PATH="$BUILD_DIR/QT_Project_Tests/${PROJECT_NAME}_Tests.app/Contents/MacOS/${PROJECT_NAME}_Tests"
    if [ -f "$TEST_APP_PATH" ]; then
        echo "Running tests..."
        "$TEST_APP_PATH"
    else
        echo "Test application not found: $TEST_APP_PATH"
    fi
fi

read -p "Press enter to continue"
