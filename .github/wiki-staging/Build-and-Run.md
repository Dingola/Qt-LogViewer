<!-- page-ownership: automation-managed -->
# Build and Run

## Overview
Qt-LogViewer is built using CMake and supports multiple platforms including Linux, macOS, and Windows. The project includes a main application and an optional test project that can be enabled via a CMake option.

## Prerequisites
- CMake
- A C++ compiler compatible with C++17
- Qt framework
- SimpleQtLogger library

## Configure and Build
1. Clone the repository.
2. Configure the project using CMake:
   ```
   cmake -S . -B build -D<PROJECT_NAME>_BUILD_TEST_PROJECT=ON
   ```
3. Build the project:
   ```
   cmake --build build
   ```

## Run
1. Navigate to the build directory.
2. Execute the compiled binary:
   ```
   ./Qt-LogViewer
   ```

## Deployment and Docker Notes
- Deployment configurations are managed using `QT_Project/CMake/install.cmake`.
- Docker support is mentioned in the README but specific details are not provided in the input.


