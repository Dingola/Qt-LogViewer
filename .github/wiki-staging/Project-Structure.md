# Project Structure

The `Qt-LogViewer` repository is organized into a clear structure to facilitate development, testing, and maintenance. Below is an overview of the key directories and files:

## Root Directory
- **README.md**: Provides an overview of the project, including its purpose and usage.
- **.gitignore**: Specifies intentionally untracked files to ignore.
- **CMakeLists.txt**: Defines the build configuration for the project.

## Source Code
Located in the `src` directory, this contains the main application logic and components.

- **src/main.cpp**: Entry point of the application.
- **src/core/**: Contains core application logic and utilities.
- **src/ui/**: Includes user interface components and related resources.

### Core
The `src/core` directory includes:
- **LogParser**: Handles parsing of log files.
- **LogModel**: Manages the data model for logs.
- **FileHandler**: Provides file I/O operations for log files.

### UI
The `src/ui` directory includes:
- **MainWindow**: Implements the main application window.
- **LogViewerWidget**: Displays parsed log data in a user-friendly format.
- **Dialogs**: Contains additional UI dialogs for user interactions.

## Resources
- **resources/**: Contains application assets such as icons, stylesheets, and other static files.

## Tests
- **tests/**: Includes unit tests and integration tests for the application.
  - **tests/core/**: Tests for core logic components.
  - **tests/ui/**: Tests for user interface components.

## Build
- **build/**: Directory for build artifacts, typically excluded from version control.

This structure ensures a modular and maintainable codebase, with a clear separation of concerns between core logic, user interface, and testing components.

