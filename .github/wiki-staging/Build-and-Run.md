# Build and Run

## Prerequisites

To build and run the Qt-LogViewer application, ensure the following dependencies are installed:

- **Qt Framework**: Version 5.15 or higher.
- **CMake**: Version 3.15 or higher.
- **Compiler**: A C++ compiler compatible with Qt (e.g., GCC, Clang, or MSVC).

## Build Instructions

1. **Clone the Repository**:
   Clone the Qt-LogViewer repository to your local machine:
   ```
   git clone https://github.com/<username>/Qt-LogViewer.git
   cd Qt-LogViewer
   ```

2. **Create a Build Directory**:
   It is recommended to use a separate directory for building the project:
   ```
   mkdir build
   cd build
   ```

3. **Run CMake**:
   Configure the project using CMake:
   ```
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```
   Replace `Release` with `Debug` if you need a debug build.

4. **Build the Project**:
   Compile the project using the following command:
   ```
   cmake --build .
   ```

## Run Instructions

After successfully building the project, the executable file will be located in the `build` directory. To run the application:

1. Navigate to the directory containing the executable:
   ```
   cd build
   ```

2. Execute the application:
   ```
   ./Qt-LogViewer
   ```
   On Windows, the executable will have a `.exe` extension:
   ```
   Qt-LogViewer.exe
   ```

## Notes

- Ensure all required libraries and dependencies are correctly installed and accessible during runtime.
- If you encounter issues during the build or run process, verify that your environment matches the prerequisites listed above.

