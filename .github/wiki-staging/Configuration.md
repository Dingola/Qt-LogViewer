# Configuration

Qt-LogViewer provides a configuration system to customize its behavior and adapt it to specific use cases. Below are the details on how to configure the application.

## Configuration File

The application uses a configuration file to store user preferences and settings. The configuration file is located in the user's home directory under the following path:

```
~/.config/Qt-LogViewer/config.ini
```

### File Structure

The configuration file is organized into sections, each containing key-value pairs. Below is an example structure:

```
[General]
theme=dark
language=en

[Logging]
logLevel=info
logFilePath=/path/to/log/file.log
```

### Key Configuration Options

#### General Section
- **theme**: Specifies the application theme. Possible values are `light` and `dark`.
- **language**: Sets the language for the user interface. Use standard language codes (e.g., `en` for English, `fr` for French).

#### Logging Section
- **logLevel**: Defines the logging verbosity. Supported values are `debug`, `info`, `warning`, and `error`.
- **logFilePath**: Specifies the path to the log file that the application will monitor.

## Editing the Configuration

To modify the configuration:
1. Open the `config.ini` file in a text editor.
2. Update the desired key-value pairs under the appropriate section.
3. Save the file and restart the application for changes to take effect.

## Default Configuration

If the configuration file is missing or incomplete, Qt-LogViewer will generate a default configuration file with the following settings:

```
[General]
theme=light
language=en

[Logging]
logLevel=info
logFilePath=
```

## Advanced Configuration

For advanced users, additional configuration options may be available. Refer to the source code or contact the maintainers for further details.

## Troubleshooting

- **Configuration Not Loading**: Ensure the `config.ini` file is located in the correct directory (`~/.config/Qt-LogViewer/`) and follows the correct structure.
- **Invalid Values**: If invalid values are provided, the application may revert to default settings for the affected keys.

For further assistance, consult the repository documentation or open an issue in the project repository.

