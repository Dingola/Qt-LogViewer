<!-- page-ownership: automation-managed -->
# Testing

## Overview
Qt-LogViewer includes a dedicated test project to ensure the reliability and correctness of its components. The test project is optional and can be enabled via a CMake variable.

## Test Project
- Located in `QT_Project_Tests/`.
- Contains tests for controllers, models, services, and views.
- Test files include both headers and source files, such as `FileCatalogControllerTest.cpp` and `LogModelTest.cpp`.

## CI and Coverage
- Continuous integration is implemented with workflows for Linux, macOS, and Windows.
- Code coverage is tracked using Codecov.

## Notes
- The test project uses Google Test as a dependency, configured via `QT_Project_Tests/ThirdParty/GoogleTest.cmake`.
- Tests cover core functionalities, including log parsing, filtering, and session management.


