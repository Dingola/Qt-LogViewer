<!-- page-ownership: automation-managed -->
# Project Structure

## Overview
The repository is organized into distinct directories for source code, headers, resources, forms, tests, and configuration files. This structure ensures a clear separation of concerns and facilitates maintainability.

## Main Directories
- `Configs/`: Contains configuration files such as `.clang-format`, `.clang-tidy`, and `Doxyfile.in`.
- `QT_Project/`: Main project directory containing source code, headers, forms, and resources.
  - `CMake/`: CMake configuration files.
  - `Forms/`: UI forms for the application.
  - `Headers/`: Header files for controllers, models, services, and views.
  - `Resources/`: Icons, images, translations, and style files.
  - `Sources/`: Source files for controllers, models, services, and views.
  - `ThirdParty/`: Integration files for third-party libraries such as `SimpleQtLogger` and `QtWidgetsCommonLib`.
- `QT_Project_Tests/`: Test project directory with headers and source files for unit tests.

## Notes
The project uses a modular structure with clear separation between controllers, models, services, and views. The test project mirrors the production structure for comprehensive testing.

