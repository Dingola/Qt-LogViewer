<!-- page-ownership: automation-managed -->

# Project Structure

## Overview
The Qt-LogViewer repository is organized into distinct directories and files to separate concerns and facilitate development and testing.

## Main Directories
- `Configs`: Contains configuration files such as `.clang-format`, `.clang-tidy`, and `Doxyfile.in`.
- `QT_Project`: Main project directory containing source code, headers, forms, resources, and CMake configuration files.
  - `CMake`: CMake configuration files for building the project.
  - `Forms`: UI forms for various application components.
  - `Headers`: Header files for controllers, models, services, and views.
  - `Resources`: Icons, images, translations, and stylesheets.
  - `Sources`: Implementation files for controllers, models, services, and views.
  - `ThirdParty`: CMake scripts for integrating third-party libraries such as `QtWidgetsCommonLib` and `SimpleQtLogger`.
- `QT_Project_Tests`: Test project directory containing test headers, sources, and CMake configuration files.

## Notes
The repository is structured to separate the main project and test project. By default, only the main project is built. The test project can be enabled using the CMake variable `<PROJECT_NAME>_BUILD_TEST_PROJECT`.

