<!-- page-ownership: automation-managed -->

# Architecture

## Overview
The architecture of Qt-LogViewer is modular, with distinct components for controllers, models, services, views, and forms. This design ensures separation of concerns and supports scalability and maintainability.

## Main Responsibilities
- **Controllers**: Manage application logic and coordinate interactions between models and views. Key controllers include `FileCatalogController`, `LogIngestController`, and `SessionController`.
- **Models**: Represent the data structure and provide data manipulation logic. Examples include `LogModel`, `LogFileTreeModel`, and `PagingProxyModel`.
- **Services**: Handle background tasks such as log loading, parsing, and session management. Key services include `LogLoader`, `LogParser`, and `SessionManager`.
- **Views**: Define the user interface components, such as `LogViewWidget`, `LogFilterBarWidget`, and `MainWindow`.
- **Forms**: Contain the UI definitions for widgets and dialogs, such as `SettingsDialog.ui` and `PaginationWidget.ui`.

## Data and Interaction Flow
1. **File Selection and Cataloging**: The `FileCatalogController` manages the file explorer, grouping logs by application and session.
2. **Log Ingestion and Parsing**: The `LogIngestController` and `LogParser` handle loading and parsing log files asynchronously.
3. **Data Management**: Parsed logs are stored in models like `LogModel` and `LogFileTreeModel`.
4. **Filtering and Paging**: Filters are applied using `LogSortFilterProxyModel`, and large datasets are paginated using `PagingProxyModel`.
5. **UI Coordination**: Views such as `LogViewWidget` and `MainWindow` display the data and interact with controllers for user actions.
6. **Session Persistence**: The `SessionManager` and `SessionRepository` save and restore user workspaces.

## Notes
The architecture leverages Qt's signal-slot mechanism for communication between components, ensuring a responsive and modular design.

