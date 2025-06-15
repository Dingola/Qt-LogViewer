#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Function to install or update Python
setup_python() {
    local MIN_REQUIRED_PYTHON_VERSION=${1:-"3.14.0"}
    local MIN_REQUIRED_PYTHON_VERSION_SUFFIX=${2:-""}
    local FULL_PYTHON_VERSION="${MIN_REQUIRED_PYTHON_VERSION}${MIN_REQUIRED_PYTHON_VERSION_SUFFIX}"

    # Function to compare Python versions
    version_greater_equal() {
        local version1=$1
        local version2=$2
        [ "$(printf '%s\n' "$version1" "$version2" | sort -V | head -n1)" = "$version2" ]
    }

    # Function to install Python using apt
    install_python() {
        local python_version=$1
        echo "Installing Python $python_version..."
        sudo apt-get update
        sudo apt-get install -y python${python_version%.*} python${python_version%.*}-dev python3-pip
        if [ $? -ne 0 ]; then
            echo "Python installation failed. Please install Python manually and try again."
            return 1
        fi
    }

    # Function to install Python from source
    install_python_from_source() {
        local python_version=$1
        local python_version_suffix=$2
        local full_python_version="${python_version}${python_version_suffix}"
        echo "Installing Python $full_python_version from source..."
        sudo apt-get update
        sudo apt-get install -y build-essential libssl-dev zlib1g-dev libncurses5-dev libncursesw5-dev libreadline-dev libsqlite3-dev libgdbm-dev libdb5.3-dev libbz2-dev libexpat1-dev liblzma-dev tk-dev
        if [ $? -ne 0 ]; then
            echo "Failed to install build dependencies. Please install them manually and try again."
            return 1
        fi
        wget "https://www.python.org/ftp/python/$python_version/Python-${full_python_version}.tgz"
        tar -xzf "Python-${full_python_version}.tgz"
        cd "Python-${full_python_version}"
        ./configure --enable-optimizations
        make -j$(nproc)
        sudo make altinstall
        if [ $? -ne 0 ]; then
            echo "Python installation from source failed. Please install Python manually and try again."
            return 1
        fi
        cd ..
        rm -rf "Python-${full_python_version}" "Python-${full_python_version}.tgz"
    }

    # Check if the specific Python version is installed
    check_specific_python_version() {
        local python_version=$1
        if command -v python${python_version%.*} &> /dev/null
        then
            PYTHON_VERSION=$(python${python_version%.*} --version 2>&1 | awk '{print $2}')
            echo "Found Python version: $PYTHON_VERSION"
            if version_greater_equal $PYTHON_VERSION $FULL_PYTHON_VERSION
            then
                echo "Python version is sufficient: $PYTHON_VERSION"
                return 0
            else
                echo "Python version is older than $FULL_PYTHON_VERSION."
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
            if [ $? -ne 0 ]; then
                read -p "Quick installation failed. Do you want to install Python from source? This may take 5 or more minutes. (y/n): " source_choice
                if [[ "$source_choice" =~ ^[yY]$ ]]; then
                    install_python_from_source $MIN_REQUIRED_PYTHON_VERSION $MIN_REQUIRED_PYTHON_VERSION_SUFFIX
                else
                    echo "Python installation aborted."
                    return 1
                fi
            fi
        else
            echo "Python installation aborted."
            return 1
        fi
    fi

    # Check if pip is installed
    if ! command -v pip${MIN_REQUIRED_PYTHON_VERSION%.*} &> /dev/null
    then
        echo "pip could not be found. Installing pip..."
        sudo apt-get install -y python3-pip
        if [ $? -ne 0 ]; then
            echo "pip installation failed. Please install pip manually and try again."
            return 1
        fi
    fi
}
