<!-- page-ownership: automation-managed -->
# Project Structure

## Overview
The Qt-LogViewer repository is organized into main directories for production and test code. The structure supports modular development and testing.

## Main Directories
- `QT_Project/Headers/Qt-LogViewer/Services`: Contains service headers, such as `NumberFormatUtils.h`.
- `QT_Project/Sources/Qt-LogViewer/Services`: Contains service implementations, such as `NumberFormatUtils.cpp`.
- `QT_Project/Sources/Qt-LogViewer/Views/App`: Contains view-related components, such as `LogLevelFilterItemWidget.cpp`.
- `QT_Project_Tests/Headers/Qt-LogViewer/Services`: Contains test headers, such as `NumberFormatUtilsTest.h`.
- `QT_Project_Tests/Sources/Qt-LogViewer/Services`: Contains test implementations, such as `NumberFormatUtilsTest.cpp`.

## Notes
The project is divided into a main application and a test project. By default, only the main application is built. The test project can be enabled using the CMake option `<PROJECT_NAME>_BUILD_TEST_PROJECT`.

