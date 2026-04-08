<!-- page-ownership: automation-managed -->
# Build and Run

## Overview
Qt-LogViewer provides a streamlined process for building and running the application across multiple platforms, including Linux, macOS, and Windows. The project uses CMake for configuration and supports Docker for deployment.

## Prerequisites
- CMake
- Qt framework
- Compiler supporting C++17 or later
- Git

## Configure and Build
1. Clone the repository:
   ```
git clone https://github.com/Dingola/Qt-LogViewer.git
cd Qt-LogViewer
   ```
2. Create a build directory and navigate to it:
   ```
mkdir build && cd build
   ```
3. Configure the project using CMake:
   ```
cmake ..
   ```
4. Build the project:
   ```
cmake --build .
   ```

## Run
After building, execute the application binary located in the build directory.

## Deployment and Docker Notes
The project supports Docker for deployment. Refer to the provided Docker configuration files for setup and usage instructions.

