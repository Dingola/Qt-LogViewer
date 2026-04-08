<!-- page-ownership: automation-managed -->
# Project Structure

## Overview
The Qt-LogViewer repository is organized into multiple directories for source code, headers, resources, configurations, and tests.

## Main Directories
- `Configs/`: Configuration files for tools like `.clang-format`, `.clang-tidy`, and `Doxygen`.
- `QT_Project/`: Main project directory containing source code, headers, forms, resources, and CMake files.
  - `CMake/`: CMake configuration files for building and installing the project.
  - `Forms/`: UI form files for the application.
  - `Headers/`: Header files for controllers, models, services, and views.
  - `Sources/`: Source files for controllers, models, services, and views.
  - `Resources/`: Icons, images, translations, and style resources.
  - `ThirdParty/`: Third-party dependencies and related CMake files.
- `QT_Project_Tests/`: Test project directory containing test headers, sources, and CMake files.

## Notes
- The project uses CMake for build configuration and supports modular components for controllers, models, services, and views.
- The test project is optional and can be enabled via a CMake boolean variable.


