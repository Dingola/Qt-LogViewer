#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Function to install or update Python
setup_python() {
    local MIN_REQUIRED_PYTHON_VERSION=${1:-"3.13"}
    local MIN_REQUIRED_PYTHON_VERSION_SUFFIX=${2:-""}
    if [ "$MIN_REQUIRED_PYTHON_VERSION_SUFFIX" = "__EMPTY__" ]; then
        MIN_REQUIRED_PYTHON_VERSION_SUFFIX=""
    fi
    local FULL_PYTHON_VERSION="${MIN_REQUIRED_PYTHON_VERSION}${MIN_REQUIRED_PYTHON_VERSION_SUFFIX}"

    # Function to compare Python versions
    version_greater_equal() {
        local version1=$1
        local version2=$2
        [ "$(printf '%s\n' "$version1" "$version2" | sort -V | head -n1)" = "$version2" ]
    }

    # Function to install Python using brew
    install_python() {
        local python_version=$1
        echo "Installing Python $python_version..."
        brew update
        brew install python@$python_version
        if [ $? -ne 0 ]; then
            echo "Python installation failed. Please install Python manually and try again."
            return 1
        fi
    }

    # Check if the specific Python version is installed
    check_specific_python_version() {
        local python_version=$1
        if command -v python${python_version%.*} &> /dev/null
        then
            PYTHON_VERSION=$(python${python_version%.*} --version 2>&1 | awk '{print $2}')
            echo "Found Python version: $PYTHON_VERSION"
            if version_greater_equal $PYTHON_VERSION $MIN_REQUIRED_PYTHON_VERSION
            then
                if [ -z "$MIN_REQUIRED_PYTHON_VERSION_SUFFIX" ] || version_greater_equal $PYTHON_VERSION $FULL_PYTHON_VERSION
                then
                    echo "Python version is sufficient: $PYTHON_VERSION"
                    return 0
                else
                    echo "Python version is older than $FULL_PYTHON_VERSION."
                    return 1
                fi
            else
                echo "Python version is older than $MIN_REQUIRED_PYTHON_VERSION."
                return 1
            fi
        else
            return 1
        fi
    }

    # Check if the specific Python version is installed
    if check_specific_python_version $MIN_REQUIRED_PYTHON_VERSION
    then
        echo "Python version is sufficient."
    else
        echo "Python could not be found or is older than $FULL_PYTHON_VERSION."
        read -p "Do you want to install Python $FULL_PYTHON_VERSION? (y/n): " choice
        if [[ "$choice" =~ ^[yY]$ ]]; then
            install_python $MIN_REQUIRED_PYTHON_VERSION
        else
            echo "Python installation aborted."
            return 1
        fi
    fi

    # Check if pip is installed
    if ! command -v pip${MIN_REQUIRED_PYTHON_VERSION%.*} &> /dev/null
    then
        echo "pip could not be found. Installing pip..."
        brew install pip
        if [ $? -ne 0 ]; then
            echo "pip installation failed. Please install pip manually and try again."
            return 1
        fi
    fi
}
