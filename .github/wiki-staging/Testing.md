<!-- page-ownership: automation-managed -->

# Testing

## Overview
Qt-LogViewer includes a dedicated test project to ensure the reliability and correctness of its components. The test project mirrors the structure of the main project, providing comprehensive coverage for controllers, models, services, and views.

## Test Project
- Located in the `QT_Project_Tests` directory.
- Includes unit tests for controllers, models, services, and views.
- Examples of test files:
  - `FileCatalogControllerTest.cpp`
  - `LogModelTest.cpp`
  - `LogParserTest.cpp`
  - `SettingsTest.cpp`

## CI and Coverage
- Continuous integration is set up for Linux, macOS, and Windows using GitHub Actions.
- Code coverage is tracked via Codecov and displayed with a badge in the README.

## Notes
The test project is optional and can be enabled using the CMake option `<PROJECT_NAME>_BUILD_TEST_PROJECT`. This ensures that tests are only built when explicitly required.

