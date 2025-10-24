#include "Qt-LogViewer/Services/StylesheetLoader.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>

/**
 * @brief Constructs a StylesheetLoader.
 * @param parent The parent QObject, or nullptr.
 */
StylesheetLoader::StylesheetLoader(QObject* parent): QObject(parent) {}

/**
 * @brief Loads a stylesheet file, parses variables (default and theme), resolves them recursively,
 * and applies it to the application. Logs success or failure.
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
        m_raw_stylesheet = QString::fromUtf8(style_file.readAll());
        m_current_stylesheet_path = file_path;
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

        qDebug() << "[StylesheetLoader] Loaded stylesheet from" << file_path
                 << "with theme:" << theme_name;
        success = true;
    }
    else
    {
        qWarning() << "[StylesheetLoader] Failed to load stylesheet from" << file_path << ":"
                   << style_file.errorString();
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
 * @brief Parses variables at the top of the stylesheet and removes their definitions.
 *        Variables must be defined as "@Name: value;" at the top of the file.
 * @param stylesheet The stylesheet string to parse and modify.
 */
auto StylesheetLoader::parse_variables(QString& stylesheet) -> void
{
    QRegularExpression var_regex(R"(^\s*@([A-Za-z0-9_\-]+)\s*:\s*([^;]+);)",
                                 QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator match_iterator = var_regex.globalMatch(stylesheet);

    while (match_iterator.hasNext())
    {
        QRegularExpressionMatch match = match_iterator.next();
        QString name = match.captured(1);
        QString value = match.captured(2).trimmed();
        m_variables[name] = value;
    }

    stylesheet.remove(var_regex);
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
 * variables.
 *
 * Prevents infinite recursion by tracking already visited variable names.
 *
 * @param name The variable name to resolve (without '@').
 * @param variables The map of all available variables.
 * @param seen A set of variable names already visited in this resolution chain (to prevent cycles).
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
