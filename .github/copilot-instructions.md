# Project Overview

Qt-LogViewer is a **Qt-based desktop application** for viewing, searching and filtering log files with a user-friendly interface.  
It supports cross-platform builds (Windows, Linux, macOS) and provides optional Docker integration.  
The project is structured as a main application (`QT_Project`) and an optional test project (`QT_Project_Tests`), both managed with **CMake**.

## Folder Structure

- `/QT_Project`
  - `/Headers/Qt-LogViewer`
    - `/Views` – UI-related classes
    - `/Services` – service layer (e.g., logging, file I/O)
    - `/Models` – data models
    - `/Controllers` – application logic
  - `/Sources` – implementation files (mirroring `Headers`)
  - `/Forms` – Qt Designer `.ui` forms
  - `/Resources` – icons, translations, images, etc.
- `/QT_Project_Tests` – test project (headers, sources, third-party test dependencies)
- `/CMake` – shared CMake modules
- `/Configs` – clang-tidy, clang-format, Doxygen, etc.
- `/Scripts` – platform-specific build & deployment scripts
- `/Dockerfile` – Docker build configuration

## Libraries and Frameworks

- **Qt 6.8** (UI & application framework)
- **C++20**
- **CMake** (minimum version 3.19)
- Optional:
  - **clang-format** & **clang-tidy** for linting
  - **Doxygen** for documentation
  - **Docker** for containerized builds

## Coding Standards

- Use **snake_case** for variable and function names.
- Use **trailing return types** for all functions (e.g., `auto foo() -> int`).
- Only **one return per method**, at the end of the method.  
  - Exception: `void` methods must not use `return`.
- **Avoid** using `break` and `continue`.
- Use `[[nodiscard]]` on functions where ignoring the return value would be a logical error or make the function call meaningless.  
- **Follow the defined folder structure:**  
  - `/Views` for UI-related classes  
  - `/Services` for service layer code (logging, file I/O, etc.)  
  - `/Models` for data models  
  - `/Controllers` for application logic  
- **Getter and setter naming convention:**  
  - Getter methods must always start with `get` (e.g., `get_value()`).  
  - Setter methods must always start with `set` (e.g., `set_value(int new_value)`).  

## Documentation Rules

- Always generate **Doxygen documentation** for both headers (`.h`) and implementation files (`.cpp`).  
- All documentation must be written **in English**.  
- **Any code change or addition (e.g., new class, method, parameter, or option) must always be accompanied by updated Doxygen documentation.**  
- When making changes, always show **exactly where documentation has been added, updated, or removed**, so that code and docs remain fully synchronized.  

## Build and Test Guidelines

- CMake options configure build type, test inclusion, sanitizers, and documentation generation.
- Tests live under `QT_Project_Tests` and mirror the structure of the main project.
- Coverage reports are uploaded via Codecov if `CODECOV_TOKEN` is configured.

## UI Guidelines

- The app should maintain a **clean, native Qt look and feel**.
- Keep layouts responsive and user-friendly.
- Resources such as icons and translations are loaded from the `/Resources` folder.
- Translations should be managed with `.ts` files and compiled via CMake targets (`_translations_update`, `_translations_compile`).
