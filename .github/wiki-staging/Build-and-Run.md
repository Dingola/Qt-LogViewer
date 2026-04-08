<!-- page-ownership: automation-managed -->

# Build and Run

## Overview
Qt-LogViewer can be built and run on Linux, macOS, and Windows. The project includes a main application and an optional test project. The test project can be enabled using the CMake option `<PROJECT_NAME>_BUILD_TEST_PROJECT`.

## Prerequisites
- CMake
- Qt framework
- SimpleQtLogger library
- QtWidgetsCommonLib library

## Configure and Build
1. Clone the repository.
2. Install the required dependencies, including SimpleQtLogger and QtWidgetsCommonLib.
3. Configure the project using CMake:
   ```
   cmake -S . -B build -D<PROJECT_NAME>_BUILD_TEST_PROJECT=ON
   ```
4. Build the project:
   ```
   cmake --build build
   ```

## Run
1. Navigate to the build directory.
2. Execute the built application:
   ```
   ./Qt-LogViewer
   ```

## Deployment and Docker Notes
The project supports deployment and can be run using Docker. Refer to the repository documentation for specific Docker setup instructions.

