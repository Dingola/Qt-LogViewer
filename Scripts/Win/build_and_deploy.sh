#!/bin/bash

source Helpers/utils.sh
source Helpers/build_script.sh

# Set variables
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -W)"
PROJECT_NAME=$(basename "$PROJECT_DIR")
BUILD_TYPE="Release"
BUILD_TARGET_TYPE="executable"
BUILD_TEST_PROJECT=false
THIRD_PARTY_INCLUDE_DIR="$(pwd -W)/ThirdPartyDir"
QT_VERSION="6.8.0"
QT_COMPILER="win64_msvc2022_64"
QT_COMPILER_DIR="msvc2022_64"
MIN_REQUIRED_PYTHON_VERSION="3.14.0"
MIN_REQUIRED_PYTHON_VERSION_SUFFIX="a1"
BUILD_DIR_NAME="_build_release"
DEPLOY_DIR_NAME="_deploy"
BUILD_ZIP_ARCHIVE=false
PACKAGE_NAME="${PROJECT_NAME}_deploy.zip"
BUILD_NSIS_INSTALLER=false
NSIS_SCRIPT="installer.nsi"

# Ensure the script is running as admin
check_admin

# Setup and build the project
setup_and_build $PROJECT_DIR $PROJECT_NAME $BUILD_TYPE $BUILD_TARGET_TYPE $BUILD_TEST_PROJECT $THIRD_PARTY_INCLUDE_DIR $QT_VERSION $QT_COMPILER $QT_COMPILER_DIR $BUILD_DIR_NAME

# Check if setup_and_build was successful
if [ $? -ne 0 ]; then
    echo "Error: setup_and_build failed."
    exit 1
fi

# Install the project
echo "Installing project..."
cmake --install "${BUILD_DIR_NAME}" --prefix "${DEPLOY_DIR_NAME}"

# Check if QT_DIR is set
if [ -z "$QT_DIR" ]; then
    echo "Error: QT_DIR environment variable is not set."
    read -p "Press enter to continue"
    exit 1
fi

# Deploy the project using windeployqt
echo "Running windeployqt..."
"$QT_DIR/$QT_VERSION/$QT_COMPILER_DIR/bin/windeployqt.exe" --release "${DEPLOY_DIR_NAME}/bin/${PROJECT_NAME}.exe"

# Check if windeployqt was successful
if [ $? -ne 0 ]; then
    echo "Error: windeployqt failed."
    read -p "Press enter to continue"
    exit 1
fi

# Copy all .qm files to the translations directory if they exist
TRANSLATIONS_DIR="${PROJECT_DIR}/${PROJECT_NAME}/Resources/Translations"
if [ -d "$TRANSLATIONS_DIR" ] && [ "$(ls -A $TRANSLATIONS_DIR/*.qm 2>/dev/null)" ]; then
    echo "Copying .qm files to translations directory..."
    mkdir -p "${DEPLOY_DIR_NAME}/bin/translations"
    cp "${TRANSLATIONS_DIR}"/*.qm "${DEPLOY_DIR_NAME}/bin/translations/"
    if [ $? -ne 0 ]; then
        echo "Error: Failed to copy .qm files."
        read -p "Press enter to continue"
        exit 1
    fi
else
    echo "No .qm files to copy."
fi

# Package the deployment directory into a ZIP file if BUILD_ZIP_ARCHIVE is true
if [ "$BUILD_ZIP_ARCHIVE" = true ]; then
    # Check if 7-Zip is installed
    if command -v 7z &> /dev/null; then
        echo "Packaging deployment directory into a ZIP file..."
        7z a "${PACKAGE_NAME}" "${DEPLOY_DIR_NAME}"

        # Check if the packaging was successful
        if [ $? -ne 0 ]; then
            echo "Error: Failed to package the deployment directory."
            read -p "Press enter to continue"
            exit 1
        fi
    else
        echo "7-Zip is not installed. Please install 7-Zip to create a ZIP archive."
        read -p "Press enter to continue"
        exit 1
    fi
else
    echo "Skipping packaging of deployment directory."
fi

# Create NSIS installer if BUILD_NSIS_INSTALLER is true
if [ "$BUILD_NSIS_INSTALLER" = true ]; then
    # Check if NSIS is installed
    if command -v makensis &> /dev/null; then
        echo "Creating installer using NSIS..."
        makensis -DPROJECT_NAME="$PROJECT_NAME" -DDEPLOY_DIR="$DEPLOY_DIR_NAME" "$NSIS_SCRIPT"

        # Check if the installer creation was successful
        if [ $? -ne 0 ]; then
            echo "Error: Failed to create the installer."
            read -p "Press enter to continue"
            exit 1
        fi
    else
        echo "NSIS is not installed. Please install NSIS to create an installer."
        read -p "Press enter to continue"
        exit 1
    fi
else
    echo "Skipping creation of NSIS installer."
fi

echo "Deployment completed successfully."
read -p "Press enter to continue"
