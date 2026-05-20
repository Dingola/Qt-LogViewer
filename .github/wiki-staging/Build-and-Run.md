<!-- page-ownership: automation-managed -->
# Build and Run

## Overview
Qt-LogViewer provides a streamlined process for building and running the application across supported platforms, including Linux, macOS, and Windows. It supports Docker for deployment and offers CMake options for configuration.

## Prerequisites
- CMake
- Qt framework
- Compiler toolchain compatible with your platform

## Configure and Build
1. Ensure all prerequisites are installed.
2. Clone the repository: `git clone https://github.com/Dingola/Qt-LogViewer.git`
3. Navigate to the project directory: `cd Qt-LogViewer`
4. Create a build directory: `mkdir build && cd build`
5. Configure the project using CMake: `cmake ..`
6. Build the project: `cmake --build .`

## Run
1. After building, navigate to the build directory.
2. Execute the application binary: `./Qt-LogViewer` (Linux/macOS) or `Qt-LogViewer.exe` (Windows).

## Deployment and Docker Notes
- Docker deployment is supported. Refer to the repository documentation for Docker-specific instructions.

