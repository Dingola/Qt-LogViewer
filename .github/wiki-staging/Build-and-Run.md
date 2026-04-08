# Build and Run

This page provides instructions for building and running the `Qt-LogViewer` application.

## Prerequisites

Before building and running the application, ensure the following prerequisites are met:

- **Qt Framework**: Install the appropriate version of the Qt framework required for this project. Refer to the `README.md` for specific version details.
- **C++ Compiler**: Ensure a compatible C++ compiler is installed on your system.
- **CMake**: Install CMake to configure and generate the build system.
- **Git**: Clone the repository using Git.

## Build Instructions

Follow these steps to build the `Qt-LogViewer` application:

1. **Clone the Repository**:
   ```bash
   git clone <repository-url>
   cd Qt-LogViewer
   ```

2. **Create a Build Directory**:
   It is recommended to use an out-of-source build. Create a separate directory for build files:
   ```bash
   mkdir build
   cd build
   ```

3. **Run CMake**:
   Configure the project using CMake:
   ```bash
   cmake ..
   ```

4. **Build the Project**:
   Compile the application using the following command:
   ```bash
   cmake --build .
   ```

## Run Instructions

After successfully building the application, you can run it as follows:

1. **Locate the Executable**:
   The compiled executable can be found in the `build` directory.

2. **Run the Application**:
   Execute the application from the command line:
   ```bash
   ./Qt-LogViewer
   ```

## Notes

- Ensure all dependencies are correctly installed before building the project.
- For troubleshooting or additional details, refer to the `README.md` file in the repository.

