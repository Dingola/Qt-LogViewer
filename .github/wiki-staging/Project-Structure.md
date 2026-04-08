<!-- page-ownership: automation-managed -->

# Project Structure

## Main Directories

- `QT_Project/` - main application
- `QT_Project/Headers/Qt-LogViewer/Views/` - UI-related classes
- `QT_Project/Headers/Qt-LogViewer/Services/` - service layer
- `QT_Project/Headers/Qt-LogViewer/Models/` - data models
- `QT_Project/Headers/Qt-LogViewer/Controllers/` - application logic
- `QT_Project/Sources/` - implementation files
- `QT_Project/Forms/` - Qt Designer forms
- `QT_Project/Resources/` - application resources
- `QT_Project_Tests/` - tests mirroring the production structure
- `Configs/` - configuration files such as tooling and documentation settings

## Structure Guidance

The repository keeps UI, services, models, and controllers separated so that the application structure remains predictable and easier to review.
