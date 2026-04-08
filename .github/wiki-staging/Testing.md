# Testing

## Overview

Testing in the `Qt-LogViewer` repository ensures the reliability and correctness of its features. The repository includes automated tests to validate functionality and prevent regressions.

## Test Suite Location

The test suite is located in the `tests/` directory. This folder contains unit tests and integration tests for various components of the application.

## Running Tests

To execute the tests, use the following command:

```
ctest
```

This command runs all available tests in the repository and provides a summary of the results.

## Test Coverage

The tests cover the following key areas:

- **Core Functionality**: Ensures the log viewer processes and displays logs correctly.
- **UI Components**: Validates the behavior of user interface elements.
- **File Handling**: Tests the ability to load and parse log files accurately.
- **Error Handling**: Verifies the application's response to invalid inputs or unexpected conditions.

## Adding Tests

To add new tests:

1. Create a new test file in the `tests/` directory.
2. Write test cases using the existing framework conventions.
3. Ensure the test file is included in the `CMakeLists.txt` for proper integration.

## Test Dependencies

The testing framework relies on the following dependencies:
- **Qt Test**: Provides tools for unit testing Qt-based applications.

Ensure these dependencies are installed before running or adding tests.

## Reporting Issues

If a test fails or an issue is identified, report it using the repository's issue tracker. Include detailed information about the failure and steps to reproduce it.

