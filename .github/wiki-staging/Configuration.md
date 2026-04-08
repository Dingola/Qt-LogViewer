<!-- page-ownership: automation-managed -->

# Configuration

## CMake Options
- `<PROJECT_NAME>_BUILD_TEST_PROJECT`: Boolean variable to enable or disable the test project during the build process.

## Environment Variables
- `PROJECT_NAME`: Specifies the name of the project.
- `PROJECT_VER`: Specifies the project version.
- `PROJECT_VER_MAJOR`, `PROJECT_VER_MINOR`, `PROJECT_VER_PATCH`: Specify the major, minor, and patch versions of the project.

## Tooling
- `.clang-format` and `.clang-tidy` for code style and linting.
- `Doxyfile.in` for generating documentation using Doxygen.
- Integration with third-party libraries such as `SimpleQtLogger` and `QtWidgetsCommonLib` via CMake scripts.

