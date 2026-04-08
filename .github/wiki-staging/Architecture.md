# Architecture

## Overview

The `Qt-LogViewer` application is designed to provide an efficient and user-friendly interface for viewing and analyzing log files. It is built using the Qt framework and follows a modular architecture to ensure scalability and maintainability.

## Key Components

### Core Modules
- **MainWindow** (`src/gui/MainWindow.cpp`): The central interface for the application, responsible for managing user interactions and displaying log data.
- **LogController** (`src/controllers/LogController.cpp`): Handles the logic for loading, filtering, and managing log entries.
- **LogModel** (`src/models/LogModel.cpp`): Represents the underlying data structure for log entries, enabling efficient data manipulation and retrieval.
- **LogParser** (`src/parsers/LogParser.cpp`): Processes raw log files and converts them into structured data for the application.

### GUI Elements
- **LogTableView** (`src/gui/LogTableView.cpp`): Displays log entries in a tabular format, supporting sorting and filtering.
- **FilterDialog** (`src/gui/FilterDialog.cpp`): Provides a user interface for applying filters to log data.
- **SettingsDialog** (`src/gui/SettingsDialog.cpp`): Allows users to configure application preferences.

### Utilities
- **FileUtils** (`src/utils/FileUtils.cpp`): Contains helper functions for file operations, such as reading and writing log files.
- **DateTimeUtils** (`src/utils/DateTimeUtils.cpp`): Provides utilities for handling date and time formats in log entries.

## Interaction Flow

1. **Log File Loading**: The user selects a log file through the `MainWindow`. The `LogController` invokes the `LogParser` to process the file and populate the `LogModel`.
2. **Data Display**: The `LogModel` provides structured data to the `LogTableView`, which renders it in a sortable and filterable table format.
3. **Filtering**: Users can apply filters via the `FilterDialog`, which updates the `LogModel` to reflect the filtered data.
4. **Settings Management**: Application preferences are configured through the `SettingsDialog`, affecting behavior such as date/time formatting and default file paths.

## File Structure

The repository is organized as follows:
- **src/controllers**: Contains controllers managing application logic.
- **src/models**: Includes data models for log entries.
- **src/parsers**: Houses parsers for processing log files.
- **src/gui**: Implements graphical user interface components.
- **src/utils**: Provides utility functions for common operations.

## Design Principles

- **Modularity**: Each component is designed to perform a specific function, ensuring clear separation of concerns.
- **Scalability**: The architecture supports easy addition of new features, such as additional log formats or advanced filtering options.
- **Maintainability**: Code is organized into logical modules, making it easier to debug and extend.

## Limitations

The current architecture does not explicitly support distributed or real-time log analysis. Future enhancements may address these capabilities.

