#pragma once

#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTimer>

/**
 * @class StylesheetLoader
 * @brief Loads, parses, and applies QSS stylesheets with variable support and runtime switching.
 *
 * Supports CSS-like variables (e.g. @ColorPrimary) inside @Variables blocks:
 *  - Default block: @Variables { @ColorPrimary: #123456; }
 *  - Named theme:   @Variables[Name="Dark"] { @ColorPrimary: #000000; }
 *
 * Precedence:
 *  - Default block is parsed first.
 *  - Theme block (if provided) overrides default variables.
 *  - Variables can reference others (e.g. @Accent: @ColorPrimary;), resolved recursively.
 *
 * Auto-reload:
 *  - When __Enable Auto Reload__ is active, `QFileSystemWatcher` observes the loaded file.
 *  - File change notifications start a single-shot debounce `QTimer` to avoid reentrancy and
 *    to coalesce multiple quick write events before calling `reload_stylesheet()`.
 *
 * Reload semantics:
 *  - `reload_stylesheet()` re-parses the last successfully loaded file using the current theme
 * name.
 *  - If the current theme is absent:
 *      - If a default `@Variables {}` block exists, variables are taken from the default block.
 *      - If no default block exists, variables stay unresolved and a warning is logged.
 *    This class does not auto-switch themes on reload; callers can use `set_theme()` if desired.
 *
 * Warnings:
 *  - If unresolved variables remain after substitution, a warning is logged.
 */
class StylesheetLoader: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a StylesheetLoader. Sets up file watcher and debounce timer.
         * @param parent The parent QObject, or nullptr.
         */
        explicit StylesheetLoader(QObject* parent = nullptr);

        /**
         * @brief Loads a stylesheet file, parses variables (default and theme), resolves them
         * recursively, and applies it to the application. Logs success or failure.
         * @param file_path The path to the QSS file.
         * @param theme_name The theme name to use, or empty for default.
         * @return true if loading and applying succeeded, false otherwise.
         */
        auto load_stylesheet(const QString& file_path,
                             const QString& theme_name = QString()) -> bool;

        /**
         * @brief Loads a stylesheet from an in-memory buffer/string (e.g., external sources).
         * @param stylesheet The raw QSS stylesheet text.
         * @param theme_name The theme name to use, or empty for default.
         * @return true if parsing and applying succeeded, false otherwise.
         */
        auto load_stylesheet_from_data(const QString& stylesheet,
                                       const QString& theme_name = QString()) -> bool;

        /**
         * @brief Reloads the last successfully loaded stylesheet path with the current theme.
         * @return true if reloading and applying succeeded, false otherwise.
         * @note If no path was previously loaded, returns false.
         */
        auto reload_stylesheet() -> bool;

        /**
         * @brief Changes the current theme and reapplies the stylesheet.
         * @param theme_name The theme to set. Empty uses the default @Variables block.
         * @return true if the theme exists (or empty default) and was applied, false otherwise.
         */
        auto set_theme(const QString& theme_name) -> bool;

        /**
         * @brief Returns the current stylesheet as a QString (with variables substituted).
         * @return The current stylesheet with variables replaced.
         */
        [[nodiscard]] auto get_current_stylesheet() const -> QString;

        /**
         * @brief Returns a list of available themes based on the loaded stylesheet.
         * @return A QStringList of theme names.
         */
        [[nodiscard]] auto get_available_themes() const -> QStringList;

        /**
         * @brief Returns the current theme name.
         * @return The current theme name, or an empty string if not set.
         */
        [[nodiscard]] auto get_current_theme_name() const -> QString;

        /**
         * @brief Returns a copy of the current variables map after resolution.
         * @return A QMap of variable name to resolved value.
         */
        [[nodiscard]] auto get_variables() const -> QMap<QString, QString>;

        /**
         * @brief Checks if a variable exists (after resolution).
         * @param name The variable name (without '@').
         * @return true if the variable exists, false otherwise.
         */
        [[nodiscard]] auto has_variable(const QString& name) const -> bool;

        /**
         * @brief Removes a variable and reapplies the stylesheet.
         * @param name The variable name (without '@').
         * @return true if the variable was removed, false otherwise.
         */
        auto remove_variable(const QString& name) -> bool;

        /**
         * @brief Sets or overrides a variable and reapplies the stylesheet.
         * @param name The variable name (without '@').
         * @param value The value to set.
         */
        auto set_variable(const QString& name, const QString& value) -> void;

        /**
         * @brief Enables automatic reloading when the loaded stylesheet file changes.
         * @param enabled True to enable, false to disable.
         * @return true if the watcher was configured, false otherwise (e.g., no file loaded).
         */
        auto enable_auto_reload(bool enabled) -> bool;

    private slots:
        /**
         * @brief Slot invoked when the watched stylesheet file changes.
         *
         * Starts a single-shot debounce timer; when it fires, the stylesheet is reloaded.
         * This prevents reentrancy and coalesces multiple quick change notifications.
         *
         * @param changed_path The file path reported by `QFileSystemWatcher`.
         */
        void on_stylesheet_file_changed(const QString& changed_path);

    private:
        /**
         * @brief Common parsing/apply routine used by both file and in-memory loading.
         *
         * Parses available themes and variables, resolves recursively, removes @Variables blocks,
         * substitutes variables, applies the stylesheet, and updates internal state.
         *
         * @param raw_stylesheet The raw QSS input.
         * @param theme_name The requested theme (empty means default block only).
         * @param source_path The file path if coming from a file, empty when from data.
         * @param configure_watcher If true, updates watcher paths for file-based loads. For data,
         *                          paths are cleared.
         * @return true if parsing + applying succeeded, false otherwise.
         */
        auto process_and_apply_stylesheet(const QString& raw_stylesheet, const QString& theme_name,
                                          const QString& source_path,
                                          bool configure_watcher) -> bool;

        /**
         * @brief Replaces all variable placeholders in the stylesheet with their values.
         * @param stylesheet The stylesheet string to process.
         * @return The stylesheet with variables substituted.
         */
        [[nodiscard]] auto substitute_variables(const QString& stylesheet) const -> QString;

        /**
         * @brief Applies the given stylesheet to the QApplication.
         * @param stylesheet The stylesheet string to apply.
         */
        auto apply_stylesheet(const QString& stylesheet) -> void;

        /**
         * @brief Extracts the @Variables block for a given theme name, or the default block if not
         * found.
         * @param stylesheet The full QSS stylesheet.
         * @param theme_name The theme name to look for.
         * @return The content of the variables block, or an empty string if not found.
         */
        static auto extract_variables_block(const QString& stylesheet,
                                            const QString& theme_name) -> QString;

        /**
         * @brief Parses variables from a variables block and fills the variables map.
         * @param variables_block The content of the variables block.
         * @param variables The map to fill with variable name/value pairs.
         */
        static void parse_variables_block(const QString& variables_block,
                                          QMap<QString, QString>& variables);

        /**
         * @brief Parses all available theme names from the raw stylesheet.
         * @param stylesheet The raw QSS stylesheet.
         * @return A QStringList of theme names.
         */
        [[nodiscard]] static auto parse_available_themes(const QString& stylesheet) -> QStringList;

        /**
         * @brief Removes all @Variables blocks from the given stylesheet string.
         * @param stylesheet The stylesheet string to process.
         * @return The stylesheet with all @Variables blocks removed.
         */
        [[nodiscard]] static auto remove_variables_blocks(const QString& stylesheet) -> QString;

        /**
         * @brief Recursively resolves a variable to its final value, following references to other
         * variables. Prevents infinite recursion by tracking visited names.
         * @param name The variable name to resolve (without '@').
         * @param variables The map of all available variables.
         * @param seen A set of variable names already visited in this resolution chain (to prevent
         * cycles).
         * @return The fully resolved value of the variable, or an empty string if not found or
         * cyclic.
         */
        [[nodiscard]] static auto resolve_variable(const QString& name,
                                                   const QMap<QString, QString>& variables,
                                                   QSet<QString>& seen) -> QString;

    private:
        QMap<QString, QString> m_variables;
        QString m_raw_stylesheet;
        QString m_current_stylesheet_path;
        QStringList m_available_themes;
        QString m_current_theme_name;
        QFileSystemWatcher m_watcher;
        bool m_auto_reload_enabled = false;
        QTimer m_reload_timer;
};
