/**
 * @file Translator.cpp
 * @brief This file contains the implementation of the Translator class.
 */

#include "Qt-LogViewer/Services/Translator.h"

#include <QCoreApplication>
#include <QDir>

/**
 * @brief Constructs a Translator object with the given parent.
 *
 * @param parent The parent object.
 */
Translator::Translator(QObject* parent)
    : QObject(parent),
      m_qt_translator(),
      m_app_translator(),
      m_translations_path(QCoreApplication::applicationDirPath() + "/translations"),
      m_current_locale()
{
    QDir translations_dir(m_translations_path);

    if (!translations_dir.exists())
    {
        qDebug() << "The translation folder does not exist: " << m_translations_path;
    }
}

// NOLINTBEGIN(modernize-use-trailing-return-type)

/**
 * @brief Loads the language translations for the specified language code.
 *
 * This method attempts to load the Qt provided translation file and the app-specific translation
 * file for the specified language code. If both files are loaded successfully, the translators are
 * installed and the languageChanged signal is emitted. If either of the files fails to load, the
 * method attempts to load the translations for the default language.
 *
 * @param language_code The language code to load (e.g. "en", "de").
 * @return True if the translations were loaded successfully, false otherwise.
 */
bool Translator::load_translation(const QString& language_code)
{
    return load_translation(QLocale(language_code));
}

/**
 * @brief Loads the language translations for the specified locale.
 *
 * This method attempts to load the Qt provided translation file and the app-specific translation
 * file for the specified locale. If both files are loaded successfully, the translators are
 * installed and the languageChanged signal is emitted. If either of the files fails to load, the
 * method attempts to load the translations for the default language.
 *
 * @param locale The locale to load translations for.
 * @return True if the translations were loaded successfully, false otherwise.
 */
bool Translator::load_translation(const QLocale& locale)
{
    remove_none_empty_translators();
    qDebug() << "Attempting to load translations for the language" << locale << "from"
             << m_translations_path;
    bool result = load(locale, QStringLiteral("qt"), m_qt_translator);

    if (result)
    {
        result = load(locale, QStringLiteral("app"), m_app_translator);

        if (result)
        {
            qDebug() << (result ? "Successfully loaded the translators for locale"
                                : "Failed to load the application translator for locale")
                     << locale;
            qApp->installTranslator(&m_qt_translator);
            qApp->installTranslator(&m_app_translator);
            m_current_locale = locale;
            emit languageChanged();
        }
    }
    else
    {
        qDebug() << "Failed to load the Qt translator for locale" << locale;
    }

    // Attempt to load the default translation if it hasn't been tried yet.
    // This is done if a translation (other than the default) has been chosen but failed to load.
    if (!result && (QLocale(QStringLiteral("en_EN")) != locale))
    {
        qDebug() << "Attempting to load the default translation";
        result = load_default_translation();
    }

    return result;
}

/**
 * @brief Loads the translations for the default language.
 *
 * @return True if the translations were loaded successfully, false otherwise.
 */
bool Translator::load_default_translation()
{
    return load_translation(QLocale(QStringLiteral("en_EN")));
}

/**
 * @brief Returns the current language code.
 *
 * This method returns the language code of the currently set locale.
 *
 * @return The current language code (e.g. "en", "de").
 */
QString Translator::get_current_language_code() const
{
    return m_current_locale.name();
}

/**
 * @brief Returns a list of available language codes found in the translations directory.
 * @return A QStringList of language codes (e.g. "en", "de").
 */
QStringList Translator::get_available_language_codes() const
{
    QStringList codes;
    QDir dir(m_translations_path);
    QStringList files = dir.entryList(QStringList() << "app_*.qm", QDir::Files);

    for (const QString& file: files)
    {
        // Example: "app_de.qm" -> "de"
        QString code = file.mid(4, file.length() - 7);

        if (!codes.contains(code))
        {
            codes.append(code);
        }
    }

    return codes;
}

/**
 * @brief Returns a list of available language names found in the translations directory.
 * @return A QStringList of language names (e.g. "English", "German").
 */
QStringList Translator::get_available_language_names() const
{
    QStringList names;
    QStringList codes = get_available_language_codes();

    for (const QString& code: codes)
    {
        QLocale locale(code);
        QString name = QLocale::languageToString(locale.language());
        names.append(name);
    }

    return names;
}

/**
 * @brief Returns a map of language codes to language names found in the translations directory.
 * @return A QMap mapping language code to language name.
 */
QMap<QString, QString> Translator::get_language_code_name_map() const
{
    QMap<QString, QString> map;
    QStringList codes = get_available_language_codes();

    for (const QString& code: codes)
    {
        QLocale locale(code);
        QString name = QLocale::languageToString(locale.language());
        map.insert(code, name);
    }

    return map;
}

// NOLINTEND(modernize-use-trailing-return-type)

/**
 * @brief Removes the installed translators if they are not empty.
 */
auto Translator::remove_none_empty_translators() -> void
{
    if (!m_qt_translator.isEmpty())
    {
        qApp->removeTranslator(&m_qt_translator);
    }

    if (!m_app_translator.isEmpty())
    {
        qApp->removeTranslator(&m_app_translator);
    }
}

/**
 * @brief Loads the translations for the specified locale and filename into the given translator.
 *
 * @param locale The locale to load translations for.
 * @param filename The filename of the translation file.
 * @param translator The translator object to load the translations into.
 * @return True if the translations were loaded successfully, false otherwise.
 */
auto Translator::load(const QLocale& locale, const QString& filename,
                      QTranslator& translator) -> bool
{
    return translator.load(locale, filename, QStringLiteral("_"), m_translations_path);
}
