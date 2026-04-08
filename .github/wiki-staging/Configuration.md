# Configuration

Qt-LogViewer provides configuration options to customize its behavior and appearance. Configuration settings are managed through specific files and can be adjusted to suit user preferences or project requirements.

## Configuration Files

The repository includes the following configuration files:

- **`config/logviewer.ini`**: This file contains settings related to the application's functionality, such as log display preferences, filtering options, and default paths. Users can edit this file to modify the application's behavior.

## Key Configuration Options

The `logviewer.ini` file supports the following key options:

- **`logDirectory`**: Specifies the default directory for loading log files. Example:
  ```
  logDirectory=/path/to/logs
  ```

- **`filterKeywords`**: Defines keywords for filtering log entries. Multiple keywords can be separated by commas. Example:
  ```
  filterKeywords=ERROR,WARNING
  ```

- **`theme`**: Sets the application's theme. Supported values include `light` and `dark`. Example:
  ```
  theme=dark
  ```

- **`autoRefreshInterval`**: Configures the interval (in seconds) for automatic log refresh. Example:
  ```
  autoRefreshInterval=5
  ```

## Editing Configuration

To modify the configuration:

1. Locate the `config/logviewer.ini` file in the repository.
2. Open the file in a text editor.
3. Update the desired settings using the format provided above.
4. Save the changes and restart Qt-LogViewer to apply them.

## Default Behavior

If `logviewer.ini` is missing or contains invalid entries, Qt-LogViewer will revert to its default settings. Ensure the file is properly formatted to avoid issues.

For advanced configuration or troubleshooting, refer to the repository's README or contact the maintainers.

