#pragma once

#include <QMap>
#include <QObject>
#include <QString>

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
         * @brief Loads a stylesheet file, parses variables, and applies it to the application.
         * @param file_path The path to the QSS file.
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

    private:
        QMap<QString, QString> m_variables;
        QString m_raw_stylesheet;
        QString m_current_stylesheet_path;
};
