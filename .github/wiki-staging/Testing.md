<!-- page-ownership: automation-managed -->
# Testing

## Overview
Qt-LogViewer includes a dedicated test project that mirrors the production structure. Tests cover controllers, models, services, and views to ensure functionality and reliability.

## Test Project
- Located in `QT_Project_Tests/`.
- Includes unit tests for key components such as `FileCatalogControllerTest`, `LogModelTest`, and `LogLoaderTest`.
- Uses GoogleTest for test execution.

## CI and Coverage
- Continuous Integration (CI) pipelines are set up for Linux, macOS, and Windows.
- Code coverage is tracked using Codecov, with a badge available in the README.

## Notes
The test project is optional and can be enabled via the CMake variable `<PROJECT_NAME>_BUILD_TEST_PROJECT`.

