<!-- page-ownership: automation-managed -->

# Build and Run

## Overview

Qt-LogViewer is built with CMake and Qt. The repository separates the main application from the test project.

## Main Build Inputs

- `QT_Project`
- `QT_Project_Tests`
- `Configs`
- repository workflows in `.github/workflows/`

## Typical Flow

1. Configure the project with CMake.
2. Build the application target.
3. Build and run the test target when enabled.
4. Use the repository workflows as the authoritative automated build reference.

## Notes

Exact platform-specific automation details should be taken from the repository workflows and build configuration files.
