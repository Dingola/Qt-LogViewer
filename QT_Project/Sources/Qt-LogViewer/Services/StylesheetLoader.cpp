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
 * @brief Loads a stylesheet file, parses variables, and applies it to the application.
 *        Logs success or failure.
 * @param file_path The path to the QSS file.
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

        // 1. Extract the correct @Variables block (by theme name or default)
        QString variables_block;
        QRegularExpression block_regex;

        if (!theme_name.isEmpty())
        {
            // Look for a named theme block (non-greedy, multiline)
            block_regex = QRegularExpression(QString(R"(@Variables\[Name="%1"\]\s*\{([\s\S]*?)\})")
                                                 .arg(QRegularExpression::escape(theme_name)),
                                             QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch match = block_regex.match(m_raw_stylesheet);

            if (match.hasMatch())
            {
                variables_block = match.captured(1);
            }
        }
        if (variables_block.isEmpty())
        {
            // Fallback: default block without a name
            block_regex = QRegularExpression(R"(@Variables\s*\{([\s\S]*?)\})",
                                             QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch match = block_regex.match(m_raw_stylesheet);
            if (match.hasMatch())
            {
                variables_block = match.captured(1);
            }
        }

        // 2. Parse variables from the selected block
        if (!variables_block.isEmpty())
        {
            QRegularExpression var_regex(R"(@([A-Za-z0-9_]+)\s*:\s*([^;]+);)");
            QRegularExpressionMatchIterator match_iterator = var_regex.globalMatch(variables_block);

            while (match_iterator.hasNext())
            {
                QRegularExpressionMatch match = match_iterator.next();
                QString name = match.captured(1);
                QString value = match.captured(2).trimmed();
                m_variables[name] = value;
            }
        }

        // 3. Remove all @Variables blocks from the stylesheet (non-greedy, multiline)
        QRegularExpression remove_blocks(R"(@Variables(\[Name="[^"]*"\])?\s*\{[\s\S]*?\})",
                                         QRegularExpression::DotMatchesEverythingOption);
        QString stylesheet = m_raw_stylesheet;
        stylesheet.replace(remove_blocks, "");

        // 4. Substitute variables and apply the stylesheet
        QString final_stylesheet = substitute_variables(stylesheet);

        if (final_stylesheet.contains(QRegularExpression(R"(@[A-Za-z0-9_]+)")))
        {
            qWarning()
                << "[StylesheetLoader] Warning: Unresolved variable(s) remain in stylesheet!";
        }

        apply_stylesheet(final_stylesheet);

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
    return substitute_variables(m_raw_stylesheet);
}

/**
 * @brief Sets or overrides a variable and reapplies the stylesheet.
 * @param name The variable name (without '@').
 * @param value The value to set.
 */
auto StylesheetLoader::set_variable(const QString& name, const QString& value) -> void
{
    m_variables[name] = value;
    QString stylesheet = m_raw_stylesheet;
    QString final_stylesheet = substitute_variables(stylesheet);
    apply_stylesheet(final_stylesheet);
}

/**
 * @brief Parses variables at the top of the stylesheet and removes their definitions.
 *        Variables must be defined as "@Name: value;" at the top of the file.
 * @param stylesheet The stylesheet string to parse and modify.
 */
auto StylesheetLoader::parse_variables(QString& stylesheet) -> void
{
    QRegularExpression var_regex(R"(^\s*@([A-Za-z0-9_]+)\s*:\s*([^;]+);)",
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
        QRegularExpression regex("@" + QRegularExpression::escape(key) + R"((?![A-Za-z0-9_]))");
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
