<!-- page-ownership: automation-managed -->

# Project Structure

## Overview
The Qt-LogViewer repository is organized into distinct modules, including controllers, models, services, views, forms, and tests. This modular structure ensures separation of concerns and maintainability.

## Main Directories
- `Configs`: Configuration files, including `.clang-format`, `.clang-tidy`, and `Doxyfile.in`.
- `QT_Project`: Main project source files, headers, forms, and resources.
  - `CMake`: CMake configuration files.
  - `Forms`: UI forms for various widgets and dialogs.
  - `Headers`: Header files for controllers, models, services, and views.
  - `Sources`: Implementation files for controllers, models, services, and views.
  - `Resources`: Icons, images, translations, and stylesheets.
  - `ThirdParty`: External dependencies like `QtWidgetsCommonLib` and `SimpleQtLogger`.
- `QT_Project_Tests`: Test project files, including headers and sources for unit tests.

## Notes
The repository includes a test project that mirrors the structure of the main project, ensuring comprehensive test coverage. The modular design facilitates easy navigation and development.

