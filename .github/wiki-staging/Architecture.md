<!-- page-ownership: automation-managed -->

# Architecture

## Overview
Qt-LogViewer is designed with a modular architecture, separating concerns into controllers, models, services, views, and forms. This structure ensures maintainability, scalability, and testability.

## Main Responsibilities
- **Controllers**: Manage application logic and coordinate interactions between models and views. Examples include `FileCatalogController`, `LogIngestController`, and `SessionController`.
- **Models**: Represent the application's data and provide mechanisms for data manipulation and querying. Examples include `LogModel`, `LogFileTreeModel`, and `PagingProxyModel`.
- **Services**: Handle specialized tasks such as log parsing, loading, and session management. Examples include `LogLoader`, `LogParser`, and `SessionManager`.
- **Views**: Define the user interface components, such as `LogFileExplorer`, `LogViewWidget`, and `SettingsDialog`.
- **Forms**: Contain UI definitions for views, including `LogFilterBarWidget.ui` and `MainWindow.ui`.

## Data and Interaction Flow
1. **File Selection and Cataloging**: The `FileCatalogController` manages the organization and display of log files in the `LogFileExplorer` view.
2. **Log Ingestion and Parsing**: The `LogIngestController` and `LogParser` handle the processing of log files, converting raw data into structured formats.
3. **Loading into Models**: Data is loaded into models such as `LogModel` and `PagingProxyModel` for efficient querying and display.
4. **Filtering and Paging**: The `LogSortFilterProxyModel` and `PagingProxyModel` enable advanced filtering and pagination of large datasets.
5. **UI Coordination and Session Persistence**: Controllers like `SessionController` and `LogViewerController` manage user interactions and save/restore workspace sessions.

## Notes
The architecture leverages Qt's model-view-controller (MVC) paradigm to ensure a clean separation of concerns and a responsive user interface.

