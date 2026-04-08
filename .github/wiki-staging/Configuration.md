<!-- page-ownership: automation-managed -->

# Configuration

## CMake Options
- `<PROJECT_NAME>_BUILD_TEST_PROJECT`: Boolean option to enable or disable building the test project.

## Environment Variables
- `PROJECT_NAME`: Specifies the main project name.
- `PROJECT_VER`, `PROJECT_VER_MAJOR`, `PROJECT_VER_MINOR`, `PROJECT_VER_PATCH`: Define the project version and its components.

## Tooling
- **Code Style and Linting**:
  - `.clang-format` and `.clang-tidy` for code style and static analysis.
- **Documentation**:
  - `Doxyfile.in` for generating documentation using Doxygen.
- **Third-Party Libraries**:
  - `SimpleQtLogger` and `QtWidgetsCommonLib` are integrated via CMake scripts.

