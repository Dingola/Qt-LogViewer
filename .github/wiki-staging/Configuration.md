# Configuration

Qt-LogViewer provides configuration options to customize its behavior and appearance. Configuration settings are managed through the `config.json` file located in the root directory of the repository.

## Configuration File Location

- **Path**: `config.json`

## Supported Configuration Options

The following options can be defined in the `config.json` file:

### 1. **Log File Path**
   - **Key**: `logFilePath`
   - **Description**: Specifies the absolute or relative path to the log file that Qt-LogViewer will display.
   - **Example**:
     ```json
     {
       "logFilePath": "/path/to/log/file.log"
     }
     ```

### 2. **Theme**
   - **Key**: `theme`
   - **Description**: Defines the visual theme of the application. Supported values include `light` and `dark`.
   - **Example**:
     ```json
     {
       "theme": "dark"
     }
     ```

### 3. **Auto-Refresh Interval**
   - **Key**: `autoRefreshInterval`
   - **Description**: Sets the interval (in milliseconds) for automatic log file refresh. A value of `0` disables auto-refresh.
   - **Example**:
     ```json
     {
       "autoRefreshInterval": 5000
     }
     ```

### 4. **Max Log Entries**
   - **Key**: `maxLogEntries`
   - **Description**: Limits the maximum number of log entries displayed in the viewer. Older entries are truncated when the limit is reached.
   - **Example**:
     ```json
     {
       "maxLogEntries": 1000
     }
     ```

## Editing Configuration

To modify the configuration:
1. Open the `config.json` file in a text editor.
2. Update the desired keys with appropriate values.
3. Save the file.
4. Restart Qt-LogViewer to apply changes.

## Default Configuration

If `config.json` is missing or incomplete, Qt-LogViewer uses the following default settings:
- **Log File Path**: `logs/application.log`
- **Theme**: `light`
- **Auto-Refresh Interval**: `0` (disabled)
- **Max Log Entries**: `500`

## Notes

- Ensure the `config.json` file is valid JSON format to avoid runtime errors.
- Changes to configuration settings take effect only after restarting the application.

