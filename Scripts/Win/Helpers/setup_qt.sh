#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -W)"
source "$SCRIPT_DIR/utils.sh"
source "$SCRIPT_DIR/setup_python.sh"

# Function to list Qt versions in a directory
list_qt_versions() {
    local dir=$1
    find "$dir" -maxdepth 1 -type d -name '6.*' -exec basename {} \;
}

# Function to setup Qt
setup_qt() {
    local QT_VERSION=$1
    local QT_COMPILER=$2
    local QT_COMPILER_DIR=$3
    local MIN_REQUIRED_PYTHON_VERSION=$4
    local MIN_REQUIRED_PYTHON_VERSION_SUFFIX=$5

    # Loop to get the correct Qt installation path or install Qt
    while true; do
        if [ -z "$QT_DIR" ]; then
            echo "The required Qt version is $QT_VERSION with compiler $QT_COMPILER."
            read -p "QT_DIR is not set. Do you want to specify a path to Qt with the required version (p) or install Qt components (i)? (p/i): " choice
            case "$choice" in
              p|P )
                read -p "Please enter the Qt installation directory containing version $QT_VERSION: " QT_DIR
                ;;
              i|I )
                # Call the function to install or update Python
                setup_python $MIN_REQUIRED_PYTHON_VERSION $MIN_REQUIRED_PYTHON_VERSION_SUFFIX
                if [ $? -ne 0 ]; then
                    echo "An error occurred during the setup of Python."
                    exit 1
                fi

                # Check if Python is installed
                if ! command -v python &> /dev/null
                then
                    echo "Python could not be found. Please install Python to continue."
                    exit 1
                fi

                # Check if pip is installed
                if ! command -v pip &> /dev/null
                then
                    read -p "pip could not be found. Do you want to install it? (y/n): " install_choice
                    case "$install_choice" in
                      y|Y ) echo "Installing pip..."; python -m ensurepip;;
                      n|N ) echo "pip installation aborted."; exit 1;;
                      * ) echo "Invalid choice. pip installation aborted."; exit 1;;
                    esac
                fi

                # Install aqtinstall if not already installed
                if ! python -m pip show aqtinstall &> /dev/null
                then
                    echo "Installing aqtinstall..."; python -m pip install aqtinstall
                fi

                read -p "Please enter the directory where Qt should be installed (leave empty to use default): " QT_INSTALL_DIR
                if [ -z "$QT_INSTALL_DIR" ]; then
                    echo "Installing Qt to the default directory..."
                    python -m aqt install-qt windows desktop $QT_VERSION $QT_COMPILER --outputdir "Qt"
                    QT_DIR="$(pwd -W)/Qt"
                else
                    echo "Installing Qt to $QT_INSTALL_DIR..."
                    python -m aqt install-qt windows desktop $QT_VERSION $QT_COMPILER --outputdir $QT_INSTALL_DIR
                    QT_DIR=$QT_INSTALL_DIR
                fi
                QT_INSTALLED=true
                ;;
              * )
                echo "Invalid choice. Please enter 'p' to specify a path or 'i' to install Qt components."
                ;;
            esac
        fi

        if [ -d "$QT_DIR/$QT_VERSION/$QT_COMPILER_DIR" ]; then
            if [ "$QT_INSTALLED" = true ]; then
                echo "Successfully installed Qt at $QT_DIR/$QT_VERSION/$QT_COMPILER_DIR"
                setx QT_DIR "$QT_DIR"
            else
                echo "Found existing Qt installation at $QT_DIR/$QT_VERSION/$QT_COMPILER_DIR"
                setx QT_DIR "$QT_DIR"
            fi
            break
        else
            echo "Qt version $QT_VERSION with compiler $QT_COMPILER_DIR not found in $QT_DIR."
            available_versions=$(list_qt_versions "$QT_DIR")
            if [ -n "$available_versions" ]; then
                echo "Available Qt versions in $QT_DIR:"
                echo "$available_versions"
            fi
            QT_DIR=""
        fi
    done
}
