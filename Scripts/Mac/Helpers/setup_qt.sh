#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
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

    # Install required dependencies
    echo "Installing required dependencies..."
    brew update
    brew install qt@6 p7zip

    # Check if QT_DIR is already set in the environment
    if [ -z "$QT_DIR" ]; then
        # Load QT_DIR from the user's shell configuration file if it exists
        SHELL_CONFIG_FILE="$HOME/.zshrc"  # Change this to .bash_profile or .profile if needed
        if [ -f "$SHELL_CONFIG_FILE" ]; then
            source "$SHELL_CONFIG_FILE"
        fi
    fi

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

                # Use the installed Python version that meets the requirements
                PYTHON_CMD="python${MIN_REQUIRED_PYTHON_VERSION%.*}"

                # Create a virtual environment
                VENV_DIR="$SCRIPT_DIR/venv"
                $PYTHON_CMD -m venv $VENV_DIR
                source $VENV_DIR/bin/activate

                # Check if pip is installed for the specific Python version
                if ! $PYTHON_CMD -m pip --version &> /dev/null
                then
                    echo "pip could not be found for $PYTHON_CMD. Installing pip..."
                    $PYTHON_CMD -m ensurepip
                    if [ $? -ne 0 ]; then
                        echo "pip installation failed. Please install pip manually and try again."
                        exit 1
                    fi
                fi

                # Install aqtinstall if not already installed
                if ! $PYTHON_CMD -m pip show aqtinstall &> /dev/null
                then
                    echo "Installing aqtinstall..."; $PYTHON_CMD -m pip install aqtinstall
                fi

                read -p "Please enter the directory where Qt should be installed (leave empty to use default): " QT_INSTALL_DIR
                if [ -z "$QT_INSTALL_DIR" ]; then
                    QT_INSTALL_DIR="$(pwd)/Qt"
                fi

                echo "Installing Qt to $QT_INSTALL_DIR..."
                $PYTHON_CMD -m aqt install-qt mac desktop $QT_VERSION $QT_COMPILER --outputdir $QT_INSTALL_DIR
                QT_DIR=$QT_INSTALL_DIR
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
                export QT_DIR="$QT_DIR"
                # Set QT_DIR permanently in the user's shell configuration file
                SHELL_CONFIG_FILE="$HOME/.zshrc"  # Change this to .bash_profile or .profile if needed
                if ! grep -q "export QT_DIR=" "$SHELL_CONFIG_FILE"; then
                    echo "export QT_DIR=\"$QT_DIR\"" >> "$SHELL_CONFIG_FILE"
                else
                    sed -i '' "s|export QT_DIR=.*|export QT_DIR=\"$QT_DIR\"|" "$SHELL_CONFIG_FILE"
                fi
            else
                echo "Found existing Qt installation at $QT_DIR/$QT_VERSION/$QT_COMPILER_DIR"
                export QT_DIR="$QT_DIR"
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
