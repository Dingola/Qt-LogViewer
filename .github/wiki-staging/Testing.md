<!-- page-ownership: automation-managed -->
# Testing

## Overview
Testing in Qt-LogViewer ensures the reliability and correctness of its components. The repository includes a dedicated test project that mirrors the production structure.

## Test Project
- **Service Tests**: The `NumberFormatUtilsTest` validates the functionality of the `NumberFormatUtils` service, including number formatting methods.
- **Test Files**:
  - `QT_Project_Tests/Headers/Qt-LogViewer/Services/NumberFormatUtilsTest.h`
  - `QT_Project_Tests/Sources/Qt-LogViewer/Services/NumberFormatUtilsTest.cpp`

## CI and Coverage
- Continuous Integration (CI) workflows are set up for Linux, macOS, and Windows.
- Code coverage is tracked using Codecov.

## Notes
The test project is optional and can be enabled using the CMake variable `<PROJECT_NAME>_BUILD_TEST_PROJECT`.

