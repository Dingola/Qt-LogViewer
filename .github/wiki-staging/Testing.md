<!-- page-ownership: automation-managed -->

# Testing

## Overview
Qt-LogViewer includes a dedicated test project to ensure the reliability and correctness of its components. The test suite mirrors the structure of the main project and covers controllers, models, services, and views.

## Test Project
The test project is located in the `QT_Project_Tests` directory and includes:
- Unit tests for controllers, models, services, and views.
- Test headers and sources organized similarly to the main project.
- Integration with GoogleTest for test execution.

## CI and Coverage
Continuous integration workflows are set up for Linux, macOS, and Windows, as indicated by the build and test badges in the README. Code coverage is tracked using Codecov, with a badge displaying the current coverage percentage.

## Notes
The test project is optional and can be enabled using the CMake variable `<PROJECT_NAME>_BUILD_TEST_PROJECT`. This separation ensures that the main project remains lightweight while providing robust testing capabilities.

