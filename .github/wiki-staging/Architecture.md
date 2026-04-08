<!-- page-ownership: automation-managed -->
# Architecture

## Overview
Qt-LogViewer employs a modular architecture with distinct responsibilities for controllers, models, services, and views. This design ensures scalability and maintainability while providing a responsive user experience.

## Main Responsibilities
- **Controllers**: Manage application logic and coordinate between models and views. Examples include `FileCatalogController`, `LogViewerController`, and `SessionController`.
- **Models**: Represent data structures such as logs (`LogModel`, `LogEntry`) and support filtering and pagination (`LogSortFilterProxyModel`, `PagingProxyModel`).
- **Services**: Handle background tasks like log loading (`LogLoader`, `LogLoadingService`), parsing (`LogParser`), and session management (`SessionManager`).
- **Views**: Provide the user interface, including widgets like `LogViewWidget`, `LogFilterBarWidget`, and `MainWindow`.

## Data and Interaction Flow
1. **File Selection and Cataloging**: `FileCatalogController` manages file selection and organizes logs into a catalog.
2. **Log Ingestion and Parsing**: `LogIngestController` and `LogParser` process log files and populate models.
3. **Loading into Models**: Logs are loaded into `LogModel` and filtered using `LogSortFilterProxyModel`.
4. **UI Coordination**: Views like `LogViewWidget` and `LogFilterWidget` display data and interact with controllers for updates.
5. **Session Persistence**: `SessionManager` saves and restores user sessions, including active views and filters.

## Notes
The architecture leverages asynchronous loading to maintain UI responsiveness and supports advanced features like regex-based search and per-view filtering.

