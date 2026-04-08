# Testing

## Overview

Testing in the `Qt-LogViewer` repository ensures the reliability and correctness of its features. The repository includes automated tests to validate functionality and detect regressions.

## Test Structure

The test files are located in the `tests/` directory. This directory contains unit tests and integration tests that cover core components of the application. Key files include:

- `tests/test_mainwindow.py`: Tests for the main application window.
- `tests/test_logparser.py`: Validates the log parsing functionality.
- `tests/test_utils.py`: Covers utility functions used across the application.

## Running Tests

To execute the tests, use the following command:

```bash
pytest
```

Ensure that `pytest` is installed in your environment before running the tests.

## Test Coverage

The tests focus on:

- **UI Components**: Verifying the behavior of widgets and dialogs.
- **Log Parsing**: Ensuring accurate parsing of log files.
- **Utilities**: Validating helper functions for consistency and correctness.

## Adding New Tests

When adding new features or fixing bugs, include corresponding tests in the `tests/` directory. Follow these guidelines:

1. **File Naming**: Name test files with the prefix `test_` followed by the feature or module name.
2. **Test Cases**: Write clear and concise test cases using `pytest` conventions.
3. **Assertions**: Use assertions to validate expected outcomes.

## Continuous Integration

The repository integrates automated testing into its CI pipeline. Tests are executed on every pull request to maintain code quality and prevent regressions. Ensure all tests pass before submitting changes.

