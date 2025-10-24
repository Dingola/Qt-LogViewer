#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @class StylesheetLoader
 * @brief Loads, parses, and applies QSS stylesheets with variable support and runtime switching.
 *
 * Supports CSS-like variables (e.g. @ColorPrimary) at the top of the QSS file.
 * Variables are replaced throughout the stylesheet. Allows switching between multiple stylesheets
 * and changing variables at runtime.
 */
class StylesheetLoader: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a StylesheetLoader.
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
         * @brief Sets or overrides a variable and reapplies the stylesheet.
         * @param name The variable name (without '@').
         * @param value The value to set.
         */
        auto set_variable(const QString& name, const QString& value) -> void;

    private:
        /**
         * @brief Parses variables at the top of the stylesheet and removes their definitions.
         *        Variables must be defined as "@Name: value;" at the top of the file.
         * @param stylesheet The stylesheet string to parse and modify.
         */
        auto parse_variables(QString& stylesheet) -> void;

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
         * variables.
         *
         * Prevents infinite recursion by tracking already visited variable names.
         *
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
};
