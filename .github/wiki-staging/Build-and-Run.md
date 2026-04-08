# Build and Run

## Prerequisites

Before building and running the Qt-LogViewer application, ensure the following dependencies are installed:

- **Qt Framework**: Version 5.15 or later.
- **CMake**: Version 3.15 or later.
- **Compiler**: A C++ compiler compatible with Qt and CMake (e.g., GCC, Clang, or MSVC).
- **Git**: For cloning the repository.

## Clone the Repository

To obtain the source code, clone the Qt-LogViewer repository:

```bash
git clone https://github.com/<username>/Qt-LogViewer.git
cd Qt-LogViewer
```

Replace `<username>` with the appropriate repository owner.

## Build Instructions

1. **Create a Build Directory**  
   It is recommended to use a separate directory for building the project to keep the source directory clean:
   ```bash
   mkdir build
   cd build
   ```

2. **Run CMake**  
   Configure the project using CMake:
   ```bash
   cmake ..
   ```
   Ensure that the correct Qt installation path is detected. If necessary, specify the path manually:
   ```bash
   cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt
   ```

3. **Build the Project**  
   Compile the application:
   ```bash
   cmake --build .
   ```

## Run the Application

After a successful build, the executable file will be available in the `build` directory. Run the application using the following command:

```bash
./Qt-LogViewer
```

## Notes

- Ensure all required dependencies are installed and accessible in your system's PATH.
- For troubleshooting build issues, refer to the CMake output for missing dependencies or configuration errors.
- The application is designed to work on platforms supported by the Qt framework. Compatibility details are not specified in the repository documentation.

