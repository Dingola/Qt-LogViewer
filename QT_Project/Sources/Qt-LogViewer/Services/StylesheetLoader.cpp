#include "Qt-LogViewer/Services/StylesheetLoader.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>

/**
 * @brief Constructs a StylesheetLoader.
 *
 * Sets up a `QFileSystemWatcher` and a single-shot debounce `QTimer` for safe, coalesced reloads.
 * @param parent The parent QObject, or nullptr.
 */
StylesheetLoader::StylesheetLoader(QObject* parent): QObject(parent)
{
    QObject::connect(&m_watcher, &QFileSystemWatcher::fileChanged, this,
                     &StylesheetLoader::on_stylesheet_file_changed);

    m_reload_timer.setSingleShot(true);
    m_reload_timer.setInterval(150);  // milliseconds
    QObject::connect(&m_reload_timer, &QTimer::timeout, this, [this]() {
        if (m_auto_reload_enabled && !m_current_stylesheet_path.isEmpty())
        {
            qDebug() << "[StylesheetLoader] Auto-reloading stylesheet from"
                     << m_current_stylesheet_path;
            reload_stylesheet();
        }
    });
}

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
auto StylesheetLoader::process_and_apply_stylesheet(const QString& raw_stylesheet,
                                                    const QString& theme_name,
                                                    const QString& source_path,
                                                    bool configure_watcher) -> bool
{
    bool success = false;

    if (!raw_stylesheet.isEmpty())
    {
        m_raw_stylesheet = raw_stylesheet;
        m_current_stylesheet_path = source_path;
        m_variables.clear();

        // Parse and store available themes
        m_available_themes = parse_available_themes(m_raw_stylesheet);

        // 1. Extract and parse the default @Variables block (if present)
        QString default_block = extract_variables_block(m_raw_stylesheet, QString());

        if (!default_block.isEmpty())
        {
            parse_variables_block(default_block, m_variables);
        }

        // 2. Extract and parse the theme @Variables block (overrides defaults if present)
        if (!theme_name.isEmpty())
        {
            QString theme_block = extract_variables_block(m_raw_stylesheet, theme_name);

            if (!theme_block.isEmpty())
            {
                parse_variables_block(theme_block, m_variables);
            }
        }

        // 3. Recursively resolve all variables (handles references to other variables)
        QMap<QString, QString> resolved_variables;

        for (auto it = m_variables.begin(); it != m_variables.end(); ++it)
        {
            QSet<QString> seen;
            resolved_variables[it.key()] = resolve_variable(it.key(), m_variables, seen);
        }

        m_variables = resolved_variables;

        // 4. Remove all @Variables blocks from the stylesheet (non-greedy, multiline)
        QString stylesheet = remove_variables_blocks(m_raw_stylesheet);

        // 5. Substitute variables and apply the stylesheet
        QString final_stylesheet = substitute_variables(stylesheet);

        if (final_stylesheet.contains(QRegularExpression(R"(@[A-Za-z0-9_\-]+)")))
        {
            qWarning()
                << "[StylesheetLoader] Warning: Unresolved variable(s) remain in stylesheet!";
        }

        apply_stylesheet(final_stylesheet);
        m_current_theme_name = theme_name;

        // Update watcher paths
        QStringList watched = m_watcher.files();
        if (!watched.isEmpty())
        {
            m_watcher.removePaths(watched);
        }

        if (configure_watcher && m_auto_reload_enabled && !m_current_stylesheet_path.isEmpty())
        {
            m_watcher.addPath(m_current_stylesheet_path);
        }

        // Log source
        if (!m_current_stylesheet_path.isEmpty())
        {
            qDebug() << "[StylesheetLoader] Loaded stylesheet from" << m_current_stylesheet_path
                     << "with theme:" << theme_name;
        }
        else
        {
            qDebug() << "[StylesheetLoader] Loaded stylesheet from data with theme:" << theme_name;
        }

        success = true;
    }
    else
    {
        qWarning() << "[StylesheetLoader] Provided stylesheet data is empty.";
    }

    return success;
}

/**
 * @brief Loads a stylesheet file, parses variables (default and theme), resolves them recursively,
 * and applies it to the application. Logs success or failure.
 *
 * Reload semantics:
 *  - Applies variables from the default block first (if present).
 *  - Overrides with requested theme (if provided and present).
 *
 * @param file_path The path to the QSS file.
 * @param theme_name The theme name to use, or empty for default.
 * @return true if loading and applying succeeded, false otherwise.
 */
auto StylesheetLoader::load_stylesheet(const QString& file_path, const QString& theme_name) -> bool
{
    bool success = false;
    QFile style_file(file_path);

    if (style_file.open(QFile::ReadOnly | QFile::Text))
    {
        const QString raw = QString::fromUtf8(style_file.readAll());
        success = process_and_apply_stylesheet(raw, theme_name, file_path, true);
    }
    else
    {
        qWarning() << "[StylesheetLoader] Failed to load stylesheet from" << file_path << ":"
                   << style_file.errorString();
    }

    return success;
}

/**
 * @brief Loads a stylesheet from an in-memory buffer/string (e.g., external sources).
 *
 * Applies default block first, then the requested theme when present.
 * In-memory stylesheets are not watched for changes.
 *
 * @param stylesheet The raw QSS stylesheet text.
 * @param theme_name The theme name to use, or empty for default.
 * @return true if parsing and applying succeeded, false otherwise.
 */
auto StylesheetLoader::load_stylesheet_from_data(const QString& stylesheet,
                                                 const QString& theme_name) -> bool
{
    return process_and_apply_stylesheet(stylesheet, theme_name, QString(), false);
}

/**
 * @brief Reloads the last successfully loaded stylesheet path with the current theme.
 * @return true if reloading and applying succeeded, false otherwise.
 */
auto StylesheetLoader::reload_stylesheet() -> bool
{
    bool success = false;

    if (!m_current_stylesheet_path.isEmpty())
    {
        success = load_stylesheet(m_current_stylesheet_path, m_current_theme_name);
    }
    else
    {
        qWarning() << "[StylesheetLoader] Reload failed: No previously loaded stylesheet path.";
    }

    return success;
}

/**
 * @brief Changes the current theme and reapplies the stylesheet.
 * @param theme_name The theme to set. Empty uses the default @Variables block.
 * @return true if the theme exists (or empty default) and was applied, false otherwise.
 */
auto StylesheetLoader::set_theme(const QString& theme_name) -> bool
{
    bool success = false;

    // Accept empty (default) theme, otherwise check availability
    if (theme_name.isEmpty() || m_available_themes.contains(theme_name))
    {
        if (!m_raw_stylesheet.isEmpty())
        {
            success = load_stylesheet_from_data(m_raw_stylesheet, theme_name);
        }
        else if (!m_current_stylesheet_path.isEmpty())
        {
            success = load_stylesheet(m_current_stylesheet_path, theme_name);
        }
        else
        {
            qWarning() << "[StylesheetLoader] set_theme failed: No stylesheet loaded yet.";
        }
    }
    else
    {
        qWarning() << "[StylesheetLoader] Theme not available:" << theme_name
                   << "Available:" << m_available_themes;
    }

    return success;
}

/**
 * @brief Returns the current stylesheet as a QString (with variables substituted).
 * @return The current stylesheet with variables replaced.
 */
auto StylesheetLoader::get_current_stylesheet() const -> QString
{
    QString stylesheet = remove_variables_blocks(m_raw_stylesheet);
    return substitute_variables(stylesheet);
}

/**
 * @brief Returns a list of available themes based on the loaded stylesheet.
 * @return A QStringList of theme names.
 */
auto StylesheetLoader::get_available_themes() const -> QStringList
{
    return m_available_themes;
}

/**
 * @brief Returns the current theme name.
 * @return The current theme name, or an empty string if not set.
 */
auto StylesheetLoader::get_current_theme_name() const -> QString
{
    return m_current_theme_name;
}

/**
 * @brief Returns a copy of the current variables map after resolution.
 * @return A QMap of variable name to resolved value.
 */
auto StylesheetLoader::get_variables() const -> QMap<QString, QString>
{
    return m_variables;
}

/**
 * @brief Checks if a variable exists (after resolution).
 * @param name The variable name (without '@').
 * @return true if the variable exists, false otherwise.
 */
auto StylesheetLoader::has_variable(const QString& name) const -> bool
{
    return m_variables.contains(name);
}

/**
 * @brief Removes a variable and reapplies the stylesheet.
 * @param name The variable name (without '@').
 * @return true if the variable was removed, false otherwise.
 */
auto StylesheetLoader::remove_variable(const QString& name) -> bool
{
    bool removed = false;

    if (m_variables.remove(name) > 0)
    {
        removed = true;
        QString final_stylesheet = get_current_stylesheet();
        apply_stylesheet(final_stylesheet);
    }

    return removed;
}

/**
 * @brief Sets or overrides a variable and reapplies the stylesheet.
 * @param name The variable name (without '@').
 * @param value The value to set.
 */
auto StylesheetLoader::set_variable(const QString& name, const QString& value) -> void
{
    m_variables[name] = value;
    QString final_stylesheet = get_current_stylesheet();
    apply_stylesheet(final_stylesheet);
}

/**
 * @brief Enables automatic reloading when the loaded stylesheet file changes.
 * @param enabled True to enable, false to disable.
 * @return true if the watcher was configured, false otherwise (e.g., no file loaded).
 */
auto StylesheetLoader::enable_auto_reload(bool enabled) -> bool
{
    bool configured = false;
    m_auto_reload_enabled = enabled;

    QStringList watched = m_watcher.files();
    if (!watched.isEmpty())
    {
        m_watcher.removePaths(watched);
    }

    if (m_auto_reload_enabled && !m_current_stylesheet_path.isEmpty())
    {
        configured = m_watcher.addPath(m_current_stylesheet_path);
    }

    return configured;
}

/**
 * @brief Replaces all variable placeholders in the stylesheet with their values.
 * @param stylesheet The stylesheet string to process.
 * @return The stylesheet with variables substituted.
 */
auto StylesheetLoader::substitute_variables(const QString& stylesheet) const -> QString
{
    QString result = stylesheet;

    // Sort variable names by length descending to avoid partial replacements
    QStringList keys = m_variables.keys();
    std::sort(keys.begin(), keys.end(),
              [](const QString& a, const QString& b) { return a.length() > b.length(); });

    for (const QString& key: keys)
    {
        // Replace only exact variable names (not as part of longer names)
        // Use negative lookahead: not followed by [A-Za-z0-9_]
        QRegularExpression regex("@" + QRegularExpression::escape(key) + R"((?![A-Za-z0-9_\-]))");
        result.replace(regex, m_variables.value(key));
    }

    return result;
}

/**
 * @brief Applies the given stylesheet to the QApplication.
 * @param stylesheet The stylesheet string to apply.
 */
auto StylesheetLoader::apply_stylesheet(const QString& stylesheet) -> void
{
    qApp->setStyleSheet(stylesheet);
}

/**
 * @brief Extracts the @Variables block for a given theme name, or the default block if not found.
 * @param stylesheet The full QSS stylesheet.
 * @param theme_name The theme name to look for.
 * @return The content of the variables block, or an empty string if not found.
 */
auto StylesheetLoader::extract_variables_block(const QString& stylesheet,
                                               const QString& theme_name) -> QString
{
    QString result;
    QRegularExpression block_regex;

    if (!theme_name.isEmpty())
    {
        block_regex = QRegularExpression(QString(R"(@Variables\[Name="%1"\]\s*\{([\s\S]*?)\})")
                                             .arg(QRegularExpression::escape(theme_name)),
                                         QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match = block_regex.match(stylesheet);

        if (match.hasMatch())
        {
            result = match.captured(1);
        }
    }

    if (result.isEmpty())
    {
        block_regex = QRegularExpression(R"(@Variables\s*\{([\s\S]*?)\})",
                                         QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch match = block_regex.match(stylesheet);

        if (match.hasMatch())
        {
            result = match.captured(1);
        }
    }

    return result;
}

/**
 * @brief Parses variables from a variables block and fills the variables map.
 * @param variables_block The content of the variables block.
 * @param variables The map to fill with variable name/value pairs.
 */
void StylesheetLoader::parse_variables_block(const QString& variables_block,
                                             QMap<QString, QString>& variables)
{
    QRegularExpression var_regex(R"(@([A-Za-z0-9_\-]+)\s*:\s*([^;]+);)");
    QRegularExpressionMatchIterator match_iterator = var_regex.globalMatch(variables_block);

    while (match_iterator.hasNext())
    {
        QRegularExpressionMatch match = match_iterator.next();
        QString name = match.captured(1);
        QString value = match.captured(2).trimmed();
        variables[name] = value;
    }
}

/**
 * @brief Parses all available theme names from the raw stylesheet.
 * @param stylesheet The raw QSS stylesheet.
 * @return A QStringList of theme names.
 */
auto StylesheetLoader::parse_available_themes(const QString& stylesheet) -> QStringList
{
    QStringList themes;
    QRegularExpression theme_regex("@Variables\\[Name=\"([^\"]+)\"\\]");
    QRegularExpressionMatchIterator it = theme_regex.globalMatch(stylesheet);

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QString theme = match.captured(1);

        if (!theme.isEmpty())
        {
            themes << theme;
        }
    }

    // Fallback: Add "Default" if there is an ungrouped @Variables block
    QRegularExpression default_block(R"(@Variables\s*\{)");

    if (default_block.match(stylesheet).hasMatch())
    {
        themes << "Default";
    }

    themes.removeDuplicates();
    return themes;
}

/**
 * @brief Removes all @Variables blocks from the given stylesheet string.
 * @param stylesheet The stylesheet string to process.
 * @return The stylesheet with all @Variables blocks removed.
 */
auto StylesheetLoader::remove_variables_blocks(const QString& stylesheet) -> QString
{
    QString result = stylesheet;
    QRegularExpression remove_blocks(R"(@Variables(\[Name="[^"]*"\])?\s*\{[\s\S]*?\})",
                                     QRegularExpression::DotMatchesEverythingOption);
    result.replace(remove_blocks, "");
    return result;
}

/**
 * @brief Recursively resolves a variable to its final value, following references to other
 * variables. Prevents cycles via a seen-set.
 * @param name The variable name to resolve (without '@').
 * @param variables The map of all available variables.
 * @param seen A set of variable names already visited in this resolution chain.
 * @return The fully resolved value of the variable, or an empty string if not found or cyclic.
 */
auto StylesheetLoader::resolve_variable(const QString& name,
                                        const QMap<QString, QString>& variables,
                                        QSet<QString>& seen) -> QString
{
    QString result;

    if (!seen.contains(name))
    {
        seen.insert(name);
        auto it = variables.find(name);

        if (it != variables.end())
        {
            QString value = it.value();
            static QRegularExpression var_regex(R"(@([A-Za-z0-9\-_]+))");
            QRegularExpressionMatch match = var_regex.match(value);

            while (match.hasMatch())
            {
                QString inner_var = match.captured(1);
                QString resolved = resolve_variable(inner_var, variables, seen);
                value.replace("@" + inner_var, resolved);
                match = var_regex.match(value);
            }

            result = value;
        }
    }

    return result;
}

/**
 * @brief Slot called by QFileSystemWatcher when the stylesheet file changes.
 *
 * Starts the debounce timer to coalesce multiple change notifications; the actual reload
 * happens when the timer fires, outside of the signal emission stack.
 *
 * @param changed_path Path that changed.
 */
void StylesheetLoader::on_stylesheet_file_changed(const QString& changed_path)
{
    if (m_auto_reload_enabled && changed_path == m_current_stylesheet_path)
    {
        m_reload_timer.start();
    }
}
